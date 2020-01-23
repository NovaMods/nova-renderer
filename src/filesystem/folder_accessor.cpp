#include "nova_renderer/filesystem/folder_accessor.hpp"


#include "nova_renderer/util/logger.hpp"
#include "loading_utils.hpp"
#include "regular_folder_accessor.hpp"
#include "zip_folder_accessor.hpp"

namespace nova::filesystem {
    FolderAccessorBase* create(const rx::filesystem::directory& path) {
        auto mut_path = path;
        // Where is the shaderpack, and what kind of folder is it in ?
        if(renderer::is_zip_folder(mut_path)) {
            // zip folder in shaderpacks folder
            mut_path.replace_extension(".zip");
            return std::make_shared<ZipFolderAccessor>(mut_path);
        }
        if(exists(mut_path)) {
            // regular folder in shaderpacks folder
            return std::make_shared<RegularFolderAccessor>(mut_path);
        }

        NOVA_LOG(FATAL) << "Could not create folder accessor for path " << mut_path;

        return {};
    }

    FolderAccessorBase::FolderAccessorBase(fs::path folder) : root_folder(std::move(folder)), resource_existence_mutex(new std::mutex) {}

    bool FolderAccessorBase::does_resource_exist(const fs::path& resource_path) {
        std::lock_guard l(*resource_existence_mutex);

        const auto full_path = root_folder / resource_path;
        return does_resource_exist_on_filesystem(full_path);
    }

    std::pmr::vector<uint32_t> FolderAccessorBase::read_spirv_file(const fs::path& resource_path) {
        const std::string buf = read_text_file(resource_path);
        const auto* buf_data = reinterpret_cast<const uint32_t*>(buf.data());
        std::pmr::vector<uint32_t> ret_val;
        ret_val.reserve(buf.size() / 4);
        ret_val.insert(ret_val.begin(), buf_data, buf_data + (buf.size() / 4));

        return ret_val;
    }

    std::optional<bool> FolderAccessorBase::does_resource_exist_in_map(const std::string& resource_string) const {
        if(resource_existence.find(resource_string) != resource_existence.end()) {
            return std::make_optional<bool>(resource_existence.at(resource_string));
        }

        return {};
    }

    const fs::path& FolderAccessorBase::get_root() const { return root_folder; }

    bool has_root(const fs::path& path, const fs::path& root) {
        if(std::distance(path.begin(), path.end()) < std::distance(root.begin(), root.end())) {
            // The path is shorter than the root path - the root can't possible be contained in the path
            return false;
        }

        auto path_itr = path.begin();
        auto root_itr = root.begin();
        while(root_itr != root.end()) {
            // Don't need to check path_itr - The if statement at the beginning ensures that the path has more members
            // than the root
            if(*root_itr != *path_itr) {
                return false;
            }

            ++root_itr;
            ++path_itr;
        }

        return true;
    }
} // namespace nova::filesystem
