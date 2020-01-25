#include "zip_folder_accessor.hpp"

#include <array>
#include <memory>

#include <rx/core/array.h>

#include "nova_renderer/util/logger.hpp"
#include "nova_renderer/util/utils.hpp"

namespace nova::filesystem {
    ZipFolderAccessor::ZipFolderAccessor(const rx::string& folder) : FolderAccessorBase(folder) {
        if(mz_zip_reader_init_file(&zip_archive, folder.data(), 0) == 0) {
            NOVA_LOG(ERROR) << "Could not open zip archive " << folder.data();
        }

        build_file_tree();
    }

    ZipFolderAccessor::~ZipFolderAccessor() { mz_zip_reader_end(&zip_archive); }

    rx::vector<uint8_t> ZipFolderAccessor::read_file(const rx::string& path) {
        const auto full_path = rx::string::format("%s/%s", root_folder, path);

        if(!does_resource_exist_on_filesystem(full_path)) {
            NOVA_LOG(ERROR) << "Resource at path " << full_path.data() << " does not exist";
            return {};
        }

        const auto file_idx = resource_indexes.find(full_path);

        mz_zip_archive_file_stat file_stat = {};
        const mz_bool has_file_stat = mz_zip_reader_file_stat(&zip_archive, *file_idx, &file_stat);
        if(has_file_stat == 0) {
            const mz_zip_error err_code = mz_zip_get_last_error(&zip_archive);
            const rx::string err = mz_zip_get_error_string(err_code);

            NOVA_LOG(ERROR) << "Could not get information for file " << full_path.data() << ": " << err.data();
        }

        rx::vector<uint8_t> resource_buffer;
        resource_buffer.reserve(static_cast<uint64_t>(file_stat.m_uncomp_size));

        const mz_bool file_extracted = mz_zip_reader_extract_to_mem(&zip_archive,
                                                                    *file_idx,
                                                                    resource_buffer.data(),
                                                                    resource_buffer.size(),
                                                                    0);
        if(file_extracted == 0) {
            const mz_zip_error err_code = mz_zip_get_last_error(&zip_archive);
            const rx::string err = mz_zip_get_error_string(err_code);

            NOVA_LOG(ERROR) << "Could not extract file " << full_path.data() << ": " << err.data();
        }

        return resource_buffer;
    }

    rx::vector<rx::string> ZipFolderAccessor::get_all_items_in_folder(const rx::string& folder) {
        const rx::vector<rx::string> folder_path_parts = folder.split('/');

        FileTreeNode* cur_node = &files;
        // Get the node at this path
        folder_path_parts.each_fwd([&](const rx::string& part) {
            bool found_node = false;
            cur_node->children.each_fwd([&](FileTreeNode& child) {
                if(child.name == part) {
                    cur_node = &child;
                    found_node = true;

                    return false;
                }

                return true;
            });

            if(!found_node) {
                NOVA_LOG(ERROR) << "Couldn't find node " << folder.data();
            }
        });

        rx::vector<rx::string> children_paths;
        children_paths.reserve(cur_node->children.size());
        cur_node->children.each_fwd([&](const FileTreeNode& child) {
            rx::string s = child.get_full_path();
            children_paths.emplace_back(s);
        });

        return children_paths;
    }

    FolderAccessorBase* ZipFolderAccessor::create_subfolder_accessor(const rx::string& path) const {
        rx::memory::allocator* allocator = &rx::memory::g_system_allocator;
        return allocator->create<ZipFolderAccessor>(rx::string::format("%s/%s", root_folder, path), zip_archive);
    }

    ZipFolderAccessor::ZipFolderAccessor(const rx::string& folder, const mz_zip_archive archive)
        : FolderAccessorBase(folder), zip_archive(archive) {
        build_file_tree();
    }

    void ZipFolderAccessor::build_file_tree() {
        const uint32_t num_files = mz_zip_reader_get_num_files(&zip_archive);

        rx::vector<rx::string> all_file_names;
        all_file_names.resize(num_files);
        char filename_buffer[1024];

        for(uint32_t i = 0; i < num_files; i++) {
            const uint32_t num_bytes_in_filename = mz_zip_reader_get_filename(&zip_archive, i, filename_buffer, 1024);
            filename_buffer[num_bytes_in_filename] = '\0';
            all_file_names.emplace_back(filename_buffer);
        }

        // Build a tree from all the files
        all_file_names.each_fwd([&](const rx::string& filename) {
            const rx::vector<rx::string> filename_parts = filename.split('/');
            FileTreeNode* cur_node = &files;

            filename_parts.each_fwd([&](const rx::string& part) {
                bool node_found = false;
                cur_node->children.each_fwd([&](FileTreeNode& child) {
                    if(child.name == part) {
                        // We already have a node for the current folder. Set this node as the current one and go to the
                        // next iteration of the loop
                        cur_node = &child;

                        node_found = true;

                        return false;
                    }

                    return true;
                });

                if(!node_found) {
                    // We didn't find a node for the current part of the path, so let's add one
                    FileTreeNode new_node;
                    new_node.name = part;
                    new_node.parent = cur_node;

                    cur_node->children.push_back(new_node);

                    cur_node = &cur_node->children.last();
                }
            });
        });
    }

    bool ZipFolderAccessor::does_resource_exist_on_filesystem(const rx::string& resource_path) {
        const auto existence_maybe = does_resource_exist_in_map(resource_path);
        if(existence_maybe) {
            return *existence_maybe;
        }

        const int32_t ret_val = mz_zip_reader_locate_file(&zip_archive, resource_path.data(), "", 0);
        if(ret_val != -1) {
            // resource found!
            resource_indexes.insert(resource_path, ret_val);
            resource_existence.insert(resource_path, true);
            return true;
        }

        // resource not found
        resource_existence.insert(resource_path, false);
        return false;
    }

    void print_file_tree(const FileTreeNode& folder, const uint32_t depth) {
        std::stringstream ss;
        for(uint32_t i = 0; i < depth; i++) {
            ss << "    ";
        }

        ss << folder.name.data();
        NOVA_LOG(INFO) << ss.str();

        folder.children.each_fwd([&](const FileTreeNode& child) { print_file_tree(child, depth + 1); });
    }

    rx::string FileTreeNode::get_full_path() const {
        rx::vector<rx::string> names;
        const FileTreeNode* cur_node = this;
        while(cur_node != nullptr) {
            names.push_back(cur_node->name);
            cur_node = cur_node->parent;
        }

        // Skip the last string in the vector, since it's the resourcepack root node
        bool is_first = false;
        const uint32_t num_path_parts = names.size() - 1;
        uint32_t cur_path_part = 0;
        rx::string joined;
        names.each_rev([&](rx::string& str) {
            if(!is_first && cur_path_part > 0 && cur_path_part < num_path_parts - 1) {
                joined = rx::string::format("%s/%s", joined, str);
            }
            if(!is_first) {
                cur_path_part++;
            }
            is_first = false;

            return true;
        });

        return joined;
    }
} // namespace nova::filesystem