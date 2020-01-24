#pragma once

#include "nova_renderer/filesystem/folder_accessor.hpp"

namespace nova::filesystem {
    /*!
     * \brief Allows access to resources in a regular folder
     */
    class RegularFolderAccessor final : public FolderAccessorBase {
    public:
        explicit RegularFolderAccessor(const rx::string& folder);
    
        ~RegularFolderAccessor() override = default;

        std::string read_text_file(const fs::path& resource_path) override;

        std::pmr::vector<fs::path> get_all_items_in_folder(const fs::path& folder) override;

        std::shared_ptr<FolderAccessorBase> create_subfolder_accessor(const fs::path& path) const override;

    protected:
        bool does_resource_exist_on_filesystem(const fs::path& resource_path) override;
    
    };
} // namespace nova::renderer
