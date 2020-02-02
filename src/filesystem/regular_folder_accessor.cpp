#include "regular_folder_accessor.hpp"

#include <rx/core/concurrency/scope_lock.h>
#include <rx/core/filesystem/directory.h>
#include <rx/core/filesystem/file.h>
#include <rx/core/log.h>

namespace nova::filesystem {
    RX_LOG("RegularFilesystem", logger);

    RegularFolderAccessor::RegularFolderAccessor(const rx::string& folder) : FolderAccessorBase(folder) {}

    rx::vector<uint8_t> RegularFolderAccessor::read_file(const rx::string& path) {
        rx::concurrency::scope_lock l(*resource_existence_mutex);

        const auto full_path = [&] {
            if(has_root(path, root_folder)) {
                return path;
            } else {
                return rx::string::format("%s/%s", root_folder, path);
            }
        }();

        if(!does_resource_exist_on_filesystem(full_path)) {
            logger(rx::log::level::k_error, "Resource at path %s doesn't exist", full_path);
            return {};
        }

        if(const auto bytes = rx::filesystem::read_binary_file(full_path)) {
            return *bytes;
        }

        return {};
    }

    rx::vector<rx::string> RegularFolderAccessor::get_all_items_in_folder(const rx::string& folder) {
        const auto full_path = rx::string::format("%s/%s", root_folder, folder);
        rx::vector<rx::string> paths = {};

        if(rx::filesystem::directory dir{full_path}) {
            dir.each([&](const rx::filesystem::directory::item& item) { paths.push_back(item.name()); });
        }

        return paths;
    }

    bool RegularFolderAccessor::does_resource_exist_on_filesystem(const rx::string& resource_path) {
        const auto existence_maybe = does_resource_exist_in_map(resource_path);
        if(existence_maybe) {
            // NOVA_LOG(TRACE) << "Does " << resource_path << " exist? " << *existence_maybe;
            return *existence_maybe;
        }

        if(const rx::filesystem::file file{resource_path, "r"}) {
            // logger(rx::log::level::k_verbose, "%s exists", resource_path);
            resource_existence.insert(resource_path, true);
            return true;
        }
        // NOVA_LOG(TRACE) << resource_path << " does not exist";
        resource_existence.insert(resource_path, false);
        return false;
    }

    FolderAccessorBase* RegularFolderAccessor::create_subfolder_accessor(const rx::string& path) const {
        return create(rx::string::format("%s/%s", root_folder, path));
    }
} // namespace nova::filesystem
