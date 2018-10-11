/*!
 * \author ddubois
 * \date 14-Aug-18.
 */

#include "folder_accessor.hpp"
#include "../util/logger.hpp"

namespace nova {
    folder_accessor_base::folder_accessor_base(const fs::path &folder) : root_folder(folder) {}

    bool folder_accessor_base::does_resource_exist(const fs::path &resource_path) {
        auto full_path = root_folder / resource_path;
        return does_resource_exist_internal(full_path);
    }

    std::vector<uint32_t> folder_accessor_base::read_spirv_file(fs::path& resource_path) {
        std::string buf = read_text_file(resource_path);

        const uint32_t* buf_data = reinterpret_cast<uint32_t*>(buf.data());
        std::vector<uint32_t> ret_val;
        ret_val.reserve(buf.size() / 4);
        ret_val.insert(ret_val.begin(), buf_data, buf_data + (buf.size() / 4));

        return ret_val;
    }

    std::optional<bool> folder_accessor_base::does_resource_exist_in_map(const std::string &resource_string) const {
        if(resource_existence.find(resource_string) != resource_existence.end()) {
            return std::make_optional<bool>(resource_existence.at(resource_string));
        }

        return {};
    }

    shader_includer::shader_includer(folder_accessor_base& folder_access) : folder_access(folder_access) {}

    shaderc_include_result* shader_includer::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) {
        const fs::path source_path = requested_source;
        auto* result = new shaderc_include_result;
        result->source_name = requested_source;
        result->source_name_length = std::strlen(requested_source);

        if(loaded_files.find(source_path) != loaded_files.end()) {
            result->content = loaded_files.at(source_path).data();
            result->content_length = std::strlen(result->content);

            return result;
        }
        
        try {
            const std::string full_text = folder_access.read_text_file(source_path);
            loaded_files[source_path] = full_text;

            result->content = loaded_files.at(source_path).data();
            result->content_length = std::strlen(result->content);

            return result;

        } catch(resource_not_found_exception& e) {
            NOVA_LOG(ERROR) << "Could not include file " << source_path.string() << " requested by " << requesting_source << ": " << e.what();

            result->content = e.what();
            result->content_length = std::strlen(result->content);

            return result;
        }
    }

    void shader_includer::ReleaseInclude(shaderc_include_result* data) {
        delete data;
    }

    const fs::path &folder_accessor_base::get_root() const {
        return root_folder;
    }

    bool has_root(const fs::path &path, const fs::path &root) {
        if(std::distance(path.begin(), path.end()) < std::distance(root.begin(), root.end())) {
            // The path is shorter than the root path - the root can't possible be contained in the path
            return false;
        }

        auto path_itr = path.begin();
        auto root_itr = root.begin();
        while(root_itr != root.end()) {
            // Don't need to check path_itr - The if statement at the beginning ensures that the path has more members
            // than the root
            if(*root_itr != *path_itr) {
                return false;
            }

            ++root_itr;
            ++path_itr;
        }

        return true;
    }
}  // namespace nova
