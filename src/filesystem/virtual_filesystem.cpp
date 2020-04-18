#include "nova_renderer/filesystem/virtual_filesystem.hpp"

#include <rx/core/log.h>

#include "nova_renderer/constants.hpp"

#include "regular_folder_accessor.hpp"

using rx::utility::move;

namespace nova::filesystem {
    RX_LOG("VirtualFilesystem", logger);

    rx::ptr<VirtualFilesystem> VirtualFilesystem::instance;

    VirtualFilesystem::VirtualFilesystem(rx::memory::allocator& allocator) : internal_allocator{&allocator}, resource_roots{allocator} {
        // Always access the working directory
        add_resource_root("./");
    }

    VirtualFilesystem* VirtualFilesystem::get_instance(rx::memory::allocator& allocator) {
        if(!instance) {
            instance = rx::make_ptr<VirtualFilesystem>(allocator, allocator);
        }

        return instance.get();
    }

    VirtualFilesystem* VirtualFilesystem::get_instance() {
        return instance.get();
    }

    void VirtualFilesystem::add_resource_root(const rx::string& root) {
        resource_roots.emplace_back(FolderAccessorBase::create(*internal_allocator, root));
    }

    void VirtualFilesystem::add_resource_root(rx::ptr<FolderAccessorBase> root_accessor) { resource_roots.push_back(move(root_accessor)); }

    rx::ptr<FolderAccessorBase> VirtualFilesystem::get_folder_accessor(const rx::string& path) const {
        if(resource_roots.is_empty()) {
            logger->error("No resource roots available in the virtual filesystem! You must register at least one resource root path");

            return {};
        }

        rx::ptr<FolderAccessorBase> ret_val;

        resource_roots.each_fwd([&](rx::ptr<FolderAccessorBase>& root) {
            if(root && root->does_resource_exist(path)) {
                ret_val = root->create_subfolder_accessor(path);
                return renderer::RX_ITERATION_STOP;
            }

            return renderer::RX_ITERATION_CONTINUE;
        });

        if(!ret_val) {
            logger->error("Could not find folder %s", path);
        }

        return ret_val;
    }
} // namespace nova::filesystem
