/*!
 * \author ddubois 
 * \date 15-Aug-18.
 */

#include <miniz_zip.h>

#include "zip_folder_accessor.hpp"
#include "../util/logger.hpp"

namespace nova {
    zip_folder_accessor::zip_folder_accessor(const std::experimental::filesystem::path &folder)
        : folder_accessor_base(folder) {
        const auto folder_string = folder.string();
        if(!mz_zip_reader_init_file(&zip_archive, folder_string.c_str(), 0)) {
            logger::instance.log(log_level::DEBUG, "Could not open zip archive " + folder_string);
            throw resource_not_found_error(folder_string);
        }
    }

    zip_folder_accessor::~zip_folder_accessor() {
        mz_zip_reader_end(&zip_archive);
    }

    bool zip_folder_accessor::does_resource_exist(const fs::path &resource_path) {
        const auto resource_string = resource_path.string();
        const auto existence_maybe = does_resource_exist_in_map(resource_string);
        if(existence_maybe) {
            return existence_maybe.value();
        }

        int32_t ret_val = mz_zip_reader_locate_file(&zip_archive, resource_string.c_str(), "", 0);
        if(ret_val != -1) {
            // resource found!
            resource_indexes.emplace(resource_string, ret_val);
            resource_existance.emplace(resource_string, true);
            return true;

        } else {
            // resource not found
            resource_existance.emplace(resource_string, false);
            return false;
        }
    }

    std::vector<uint8_t> zip_folder_accessor::read_resource(const fs::path &resource_path) {
        std::string resource_string = resource_path.string();
        if(!does_resource_exist(resource_path)) {
            logger::instance.log(log_level::DEBUG, "Resource at path " + resource_string + " does not exist");
            throw resource_not_found_error(resource_string);
        }

        uint32_t file_idx = resource_indexes.at(resource_string);

        mz_zip_archive_file_stat file_stat = {};
        mz_bool has_file_stat = mz_zip_reader_file_stat(&zip_archive, file_idx, &file_stat);
        if(!has_file_stat) {
            mz_zip_error err_code = mz_zip_get_last_error(&zip_archive);
            std::string err = mz_zip_get_error_string(err_code);

            logger::instance.log(log_level::DEBUG, "Could not get information for file " + resource_string + ": " + err);
            throw resource_not_found_error(resource_string);
        }

        std::vector<uint8_t> resource_buffer;
        resource_buffer.reserve(file_stat.m_uncomp_size);

        mz_bool file_extracted = mz_zip_reader_extract_to_mem(&zip_archive, file_idx, resource_buffer.data(), resource_buffer.size(), 0);
        if(!file_extracted) {
            mz_zip_error err_code = mz_zip_get_last_error(&zip_archive);
            std::string err = mz_zip_get_error_string(err_code);

            logger::instance.log(log_level::DEBUG, "Could not extract file " + resource_string + ": " + err);
            throw resource_not_found_error(resource_string);
        }

        return resource_buffer;
    }
}