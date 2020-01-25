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

    void VirtualFilesystem::add_resource_root(FolderAccessorBase* root_accessor) { resource_roots.push_back(root_accessor); }

    FolderAccessorBase* VirtualFilesystem::get_folder_accessor(const rx::string& path) const {
        FolderAccessorBase* ret_val = nullptr;

        resource_roots.each_fwd([&](FolderAccessorBase* root) {
            if(root && root->does_resource_exist(path)) {
                ret_val = root->create_subfolder_accessor(path);
            }
        });

        if(ret_val == nullptr) {
            NOVA_LOG(ERROR) << "Could not file folder " << path.data();
        }

        return ret_val;
    }
} // namespace nova::filesystem
