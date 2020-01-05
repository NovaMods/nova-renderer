#include "nova_renderer/filesystem/virtual_filesystem.hpp"

namespace nova::filesystem {
    std::shared_ptr<VirtualFilesystem> VirtualFilesystem::get_instance() {
        if(!instance) {
            instance = std::make_shared<VirtualFilesystem>();
        }

        return instance;
    }

    std::shared_ptr<FolderAccessorBase> VirtualFilesystem::get_folder_accessor(const fs::path& path) const {
        for(const auto& root : resource_roots) {
            if(root->does_resource_exist(path)) {
                return root->create_subfolder_accessor(path);
            }
        }
    }
} // namespace nova::fs
