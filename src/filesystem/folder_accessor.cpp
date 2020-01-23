#include <utility>
#include "nova_renderer/filesystem/folder_accessor.hpp"

#include "nova_renderer/util/logger.hpp"

#include "loading_utils.hpp"
#include "regular_folder_accessor.hpp"
#include "zip_folder_accessor.hpp"

namespace nova::filesystem {
    FolderAccessorBase* create(const rx::string& path) {
        rx::memory::allocator* allocator = &rx::memory::g_system_allocator;

        // Where is the shaderpack, and what kind of folder is it in ?
        if(renderer::is_zip_folder(path)) {
            // zip folder in shaderpacks folder
            return allocator->create<ZipFolderAccessor>(path);

        } else if(rx::filesystem::directory directory(path); directory) {
            // regular folder in shaderpacks folder
            return allocator->create<RegularFolderAccessor>(directory);
        }

        NOVA_LOG(FATAL) << "Could not create folder accessor for path " << path.data();

        return nullptr;
    }

    FolderAccessorBase::FolderAccessorBase(rx::string folder)
        : root_folder(std::move(folder)), resource_existence_mutex(new rx::concurrency::mutex) {}

    bool FolderAccessorBase::does_resource_exist(const rx::string& resource_path) {
        std::lock_guard l(*resource_existence_mutex);

        const auto full_path = rx::string::format("%s/%s", root_folder, resource_path);
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
