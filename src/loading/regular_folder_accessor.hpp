#pragma once

#include "folder_accessor.hpp"

namespace nova::renderer {
    /*!
     * \brief Allows access to resources in a regular folder
     */
    class RegularFolderAccessor : public FolderAccessorBase {
    public:
        explicit RegularFolderAccessor(const fs::path& folder);

        std::string read_text_file(const fs::path& resource_path) override final;

        std::vector<fs::path> get_all_items_in_folder(const fs::path& folder) override final;

    protected:
        bool does_resource_exist_on_filesystem(const fs::path& resource_path) override final;
    };
} // namespace nova::renderer
