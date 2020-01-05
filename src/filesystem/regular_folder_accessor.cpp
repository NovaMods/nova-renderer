#include "regular_folder_accessor.hpp"

#include "nova_renderer/util/logger.hpp"

namespace nova::filesystem {
    RegularFolderAccessor::RegularFolderAccessor(const fs::path& folder) : FolderAccessorBase(folder) {}

    std::string RegularFolderAccessor::read_text_file(const fs::path& resource_path) {
        std::lock_guard l(*resource_existence_mutex);
        fs::path full_resource_path;
        if(has_root(resource_path, root_folder)) {
            full_resource_path = resource_path;
        } else {
            full_resource_path = root_folder / resource_path;
        }

        if(!does_resource_exist_on_filesystem(full_resource_path)) {
            NOVA_LOG(ERROR) << "Resource at path " << full_resource_path.string() << " does not exist";
        }

        FILE* resource_file = std::fopen(full_resource_path.string().c_str(), "r");
        if(!resource_file) {
            // Error reading this file - it can't be read again in the future
            const auto& resource_string = full_resource_path.string();

            resource_existence.emplace(resource_string, false);
            NOVA_LOG(ERROR) << "Could not load resource at path " << resource_string;

            return {};
        }

        std::fseek(resource_file, 0, SEEK_END);
        const auto file_size = std::ftell(resource_file);

        std::string file_string(file_size, 0);

        std::fseek(resource_file, 0, SEEK_SET);

        std::fread(file_string.data(), sizeof(char), file_size, resource_file);

        std::fclose(resource_file);

        return file_string;
    }

    std::pmr::vector<fs::path> RegularFolderAccessor::get_all_items_in_folder(const fs::path& folder) {
        const fs::path full_path = root_folder / folder;
        std::pmr::vector<fs::path> paths = {};

        fs::directory_iterator folder_itr(full_path);
        for(const fs::directory_entry& entry : folder_itr) {
            paths.push_back(entry.path());
        }

        return paths;
    }

    bool RegularFolderAccessor::does_resource_exist_on_filesystem(const fs::path& resource_path) {
        // NOVA_LOG(TRACE) << "Checking resource existence for " << resource_path;
        const std::string& resource_string = resource_path.string();
        const auto existence_maybe = does_resource_exist_in_map(resource_string);
        if(existence_maybe) {
            // NOVA_LOG(TRACE) << "Does " << resource_path << " exist? " << *existence_maybe;
            return *existence_maybe;
        }

        if(exists(resource_path)) {
            // NOVA_LOG(TRACE) << resource_path << " exists";
            resource_existence.emplace(resource_string, true);
            return true;
        }
        // NOVA_LOG(TRACE) << resource_path << " does not exist";
        resource_existence.emplace(resource_string, false);
        return false;
    }

    std::shared_ptr<FolderAccessorBase> RegularFolderAccessor::create_subfolder_accessor(const fs::path& path) const {
        return create(root_folder / path);
    }
} // namespace nova::filesystem
