#pragma once

#include "nova_renderer/filesystem/folder_accessor.hpp"

namespace nova::filesystem {
    /*!
     * \brief Allows access to resources in a regular folder
     */
    class RegularFolderAccessor : public FolderAccessorBase {
    public:
        explicit RegularFolderAccessor(const fs::path& folder);
    
        ~RegularFolderAccessor() override;

        std::string read_text_file(const fs::path& resource_path) override final;

        std::pmr::vector<fs::path> get_all_items_in_folder(const fs::path& folder) override final;

        std::shared_ptr<FolderAccessorBase> create_subfolder_accessor(const fs::path& path) const override;

    protected:
        bool does_resource_exist_on_filesystem(const fs::path& resource_path) override final;
    
    };
} // namespace nova::renderer
