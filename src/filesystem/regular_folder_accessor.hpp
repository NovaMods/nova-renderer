#pragma once

#include "nova_renderer/filesystem/folder_accessor.hpp"

namespace nova::filesystem {
    /*!
     * \brief Allows access to resources in a regular folder
     */
    class RegularFolderAccessor final : public FolderAccessorBase {
    public:
        explicit RegularFolderAccessor(const rx::filesystem::directory& folder);
    
        ~RegularFolderAccessor() override = default;

        rx::vector<uint8_t> read_file(const rx::string& path) override;

        rx::vector<rx::string> get_all_items_in_folder(const rx::string& folder) override;

        FolderAccessorBase* create_subfolder_accessor(const rx::string& path) const override;

    protected:
        bool does_resource_exist_on_filesystem(const rx::string& resource_path) override;
    
    };
} // namespace nova::renderer
