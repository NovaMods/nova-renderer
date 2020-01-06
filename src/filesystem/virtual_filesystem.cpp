#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include "nova_renderer/util/logger.hpp"

#include "regular_folder_accessor.hpp"

namespace nova::filesystem {
    std::shared_ptr<VirtualFilesystem> VirtualFilesystem::instance;

    std::shared_ptr<VirtualFilesystem> VirtualFilesystem::get_instance() {
        if(!instance) {
            instance = std::make_shared<VirtualFilesystem>();
        }

        return instance;
    }

    void VirtualFilesystem::add_resource_root(const fs::path& root) {
        resource_roots.emplace_back(filesystem_root->create_subfolder_accessor(root));
    }

    void VirtualFilesystem::add_resource_root(const std::shared_ptr<FolderAccessorBase>& root_accessor) {
        resource_roots.emplace_back(root_accessor);
    }

    std::shared_ptr<FolderAccessorBase> VirtualFilesystem::get_folder_accessor(const fs::path& path) const {
        for(const auto& root : resource_roots) {
            if(root->does_resource_exist(path)) {
                return root->create_subfolder_accessor(path);
            }
        }

        NOVA_LOG(ERROR) << "Could not file folder " << path.string();
        return {};
    }

    VirtualFilesystem::VirtualFilesystem() : filesystem_root(std::make_shared<RegularFolderAccessor>("./")) {}
} // namespace nova::filesystem
