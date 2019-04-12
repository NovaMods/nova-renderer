#pragma once

#include "folder_accessor.hpp"

namespace nova::renderer {
    /*!
     * \brief Allows access to resources in a regular folder
     */
    class regular_folder_accessor : public folder_accessor_base {
    public:
        explicit regular_folder_accessor(const fs::path& folder);

        std::string read_text_file(const fs::path& resource_path) override;

        std::vector<fs::path> get_all_items_in_folder(const fs::path& folder) override;

    protected:
        bool does_resource_exist_on_filesystem(const fs::path& resource_path) override;
    };
} // namespace nova::renderer
