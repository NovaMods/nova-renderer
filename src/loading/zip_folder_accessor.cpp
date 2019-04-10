/*!
 * \author ddubois
 * \date 15-Aug-18.
 */

#include "zip_folder_accessor.hpp"

#include <array>
#include <memory>

#include "nova_renderer/util/utils.hpp"
#include "../util/logger.hpp"

namespace nova::renderer {
    zip_folder_accessor::zip_folder_accessor(const fs::path& folder) : folder_accessor_base(folder), files(new file_tree_node) {
        const auto folder_string = folder.string();

        if(mz_zip_reader_init_file(&zip_archive, folder_string.c_str(), 0) == 0) {
            NOVA_LOG(DEBUG) << "Could not open zip archive " << folder_string;
            throw resource_not_found_exception(folder_string);
        }

        build_file_tree();
    }

    zip_folder_accessor::~zip_folder_accessor() {
        mz_zip_reader_end(&zip_archive);
        delete_file_tree(files);
    }

    void zip_folder_accessor::delete_file_tree(std::unique_ptr<file_tree_node>& node) { node = nullptr; }

    std::string zip_folder_accessor::read_text_file(const fs::path& resource_path) {
        const fs::path full_path = *root_folder / resource_path;

        const std::string resource_string = full_path.string();
        if(!does_resource_exist_on_filesystem(full_path)) {
            NOVA_LOG(DEBUG) << "Resource at path " << resource_string << " does not exist";
            throw resource_not_found_exception(resource_string);
        }

        const uint32_t file_idx = resource_indexes.at(resource_string);

        mz_zip_archive_file_stat file_stat = {};
        const mz_bool has_file_stat = mz_zip_reader_file_stat(&zip_archive, file_idx, &file_stat);
        if(has_file_stat == 0) {
            const mz_zip_error err_code = mz_zip_get_last_error(&zip_archive);
            const std::string err = mz_zip_get_error_string(err_code);

            NOVA_LOG(DEBUG) << "Could not get information for file " << resource_string << ": " << err;
            throw resource_not_found_exception(resource_string);
        }

        std::vector<char> resource_buffer;
        resource_buffer.reserve(static_cast<uint64_t>(file_stat.m_uncomp_size));

        const mz_bool file_extracted = mz_zip_reader_extract_to_mem(&zip_archive,
                                                                    file_idx,
                                                                    resource_buffer.data(),
                                                                    resource_buffer.size(),
                                                                    0);
        if(file_extracted == 0) {
            const mz_zip_error err_code = mz_zip_get_last_error(&zip_archive);
            const std::string err = mz_zip_get_error_string(err_code);

            NOVA_LOG(DEBUG) << "Could not extract file " << resource_string << ": " << err;
            throw resource_not_found_exception(resource_string);
        }

        return std::string{resource_buffer.data()};
    }

    std::vector<fs::path> zip_folder_accessor::get_all_items_in_folder(const fs::path& folder) {
        std::string folder_stringname = folder.string();
        std::vector<std::string> folder_path_parts = split(folder.string(), '/');

        file_tree_node* cur_node = files.get();
        // Get the node at this path
        for(const std::string& part : folder_path_parts) {
            bool found_node = false;
            for(std::unique_ptr<file_tree_node>& child : cur_node->children) {
                if(child->name == part) {
                    cur_node = child.get();
                    found_node = true;
                    break;
                }
            }

            if(!found_node) {
                throw resource_not_found_exception(folder.string());
            }
        }

        std::vector<fs::path> children_paths;
        children_paths.reserve(cur_node->children.size());
        for(const std::unique_ptr<file_tree_node>& child : cur_node->children) {
            std::string s = child->get_full_path();
            children_paths.emplace_back(s);
        }

        return children_paths;
    }

    void zip_folder_accessor::build_file_tree() {
        uint32_t num_files = mz_zip_reader_get_num_files(&zip_archive);

        std::vector<std::string> all_filenames;
        all_filenames.resize(num_files);
        std::array<char, 1024> filename_buffer{0};

        for(uint32_t i = 0; i < num_files; i++) {
            uint32_t num_bytes_in_filename = mz_zip_reader_get_filename(&zip_archive, i, filename_buffer.data(), 1024);
            filename_buffer[num_bytes_in_filename] = '\0';
            all_filenames.emplace_back(filename_buffer.data());
        }

        // Build a tree from all the files
        for(const std::string& filename : all_filenames) {
            auto filename_parts = split(filename, '/');
            auto cur_node = files.get();
            for(const auto& part : filename_parts) {
                bool node_found = false;
                for(const auto& child : cur_node->children) {
                    if(child->name == part) {
                        // We already have a node for the current folder. Set this node as the current one and go to the
                        // next iteration of the loop
                        cur_node = child.get();

                        node_found = true;
                        break;
                    }
                }
                if(node_found) {
                    continue;
                }

                // We didn't find a node for the current part of the path, so let's add one
                auto new_node = std::make_unique<file_tree_node>();
                new_node->name = part;
                new_node->parent = cur_node;

                auto* new_node_raw = new_node.get();
                cur_node->children.push_back(std::move(new_node));

                cur_node = new_node_raw;
            }
        }
    }

    bool zip_folder_accessor::does_resource_exist_on_filesystem(const fs::path& resource_path) {
        const auto resource_string = resource_path.string();
        const auto existence_maybe = does_resource_exist_in_map(resource_string);
        if(existence_maybe) {
            return existence_maybe.value();
        }

        int32_t ret_val = mz_zip_reader_locate_file(&zip_archive, resource_string.c_str(), "", 0);
        if(ret_val != -1) {
            // resource found!
            resource_indexes.emplace(resource_string, ret_val);
            resource_existence.emplace(resource_string, true);
            return true;
        }
        // resource not found
        resource_existence.emplace(resource_string, false);
        return false;
    }

    void print_file_tree(const std::unique_ptr<file_tree_node>& folder, uint32_t depth) {
        if(folder == nullptr) {
            return;
        }

        std::stringstream ss;
        for(uint32_t i = 0; i < depth; i++) {
            ss << "    ";
        }

        ss << folder->name;
        NOVA_LOG(INFO) << ss.str();

        for(const auto& child : folder->children) {
            print_file_tree(child, depth + 1);
        }
    }

    std::string file_tree_node::get_full_path() const {
        std::vector<std::string> names;
        const file_tree_node* cur_node = this;
        while(cur_node != nullptr) {
            names.push_back(cur_node->name);
            cur_node = cur_node->parent;
        }

        // Skip the last string in the vector, since it's the resourcepack root node
        return join({++names.rbegin(), names.rend()}, "/");
    }
} // namespace nova::renderer