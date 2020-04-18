#include "nova_renderer/filesystem/folder_accessor.hpp"

#include <rx/core/concurrency/scope_lock.h>
#include <rx/core/log.h>

#include "regular_folder_accessor.hpp"
#include "zip_folder_accessor.hpp"

namespace nova::filesystem {
    RX_LOG("filesystem", logger);

    bool is_zip_folder(const rx::string& path_to_folder) { return path_to_folder.ends_with(".zip"); }

    rx::ptr<FolderAccessorBase> FolderAccessorBase::create(rx::memory::allocator& allocator, const rx::string& path) {

        // Where is the renderpack, and what kind of folder is it in ?
        if(is_zip_folder(path)) {
            // zip folder in renderpacks folder
            return rx::make_ptr<ZipFolderAccessor>(allocator, allocator, path);

        } else if(const rx::filesystem::directory directory(path); directory) {
            // regular folder in renderpacks folder
            return rx::make_ptr<RegularFolderAccessor>(allocator, allocator, path);
        }

        logger->error("Could not create folder accessor for path %s", path);

        return {};
    }

    FolderAccessorBase::FolderAccessorBase(rx::memory::allocator& allocator, rx::string folder)
        : internal_allocator{&allocator},
          root_folder{rx::utility::move(folder)},
          resource_existence{allocator},
          resource_existence_mutex{rx::make_ptr<rx::concurrency::mutex>(allocator)} {}

    bool FolderAccessorBase::does_resource_exist(const rx::string& resource_path) {
        rx::concurrency::scope_lock l(*resource_existence_mutex);

        const auto full_path = rx::string::format(*internal_allocator, "%s/%s", root_folder, resource_path);
        return does_resource_exist_on_filesystem(full_path);
    }

    rx::string FolderAccessorBase::read_text_file(const rx::string& resource_path) {
        auto buf = read_file(resource_path);
        return buf.disown();
    }

    rx::optional<bool> FolderAccessorBase::does_resource_exist_in_map(const rx::string& resource_string) const {
        if(const auto* val = resource_existence.find(resource_string); val != nullptr) {
            return rx::optional<bool>(*val);
        }

        return rx::nullopt;
    }

    const rx::string& FolderAccessorBase::get_root() const { return root_folder; }

    bool has_root(const rx::string& path, const rx::string& root) { return path.begins_with(root); }
} // namespace nova::filesystem