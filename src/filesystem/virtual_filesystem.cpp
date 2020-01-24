#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include "nova_renderer/util/logger.hpp"

#include "regular_folder_accessor.hpp"

namespace nova::filesystem {
    VirtualFilesystem* VirtualFilesystem::instance = nullptr;

    VirtualFilesystem* VirtualFilesystem::get_instance() {
        if(!instance) {
            rx::memory::allocator* allocator = &rx::memory::g_system_allocator;
            instance = allocator->create<VirtualFilesystem>();
        }

        return instance;
    }

    void VirtualFilesystem::add_resource_root(const rx::string& root) { resource_roots.emplace_back(FolderAccessorBase::create(root)); }

    void VirtualFilesystem::add_resource_root(const FolderAccessorBase* root_accessor) { resource_roots.emplace_back(root_accessor); }

    FolderAccessorBase* VirtualFilesystem::get_folder_accessor(const rx::string& path) const {
        for(const auto& root : resource_roots) {
            if(root && root->does_resource_exist(path)) {
                return root->create_subfolder_accessor(path);
            }
        }

        NOVA_LOG(ERROR) << "Could not file folder " << path.data();
        return nullptr;
    }
} // namespace nova::filesystem
