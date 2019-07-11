#pragma once

#include <miniz/miniz_zip.h>

#include "folder_accessor.hpp"
#include <EASTL/unique_ptr.h>

namespace nova::renderer {
    struct FileTreeNode {
        eastl::string name;
        eastl::vector<eastl::unique_ptr<FileTreeNode>> children;
        FileTreeNode* parent = nullptr;

        [[nodiscard]] eastl::string get_full_path() const;
    };

    /*!
     * \brief Allows access to a zip folder
     */
    class ZipFolderAccessor : public FolderAccessorBase {
    public:
        explicit ZipFolderAccessor(const fs::path& folder);

        ZipFolderAccessor(ZipFolderAccessor&& other) noexcept = default;
        ZipFolderAccessor& operator=(ZipFolderAccessor&& other) noexcept = default;

        ZipFolderAccessor(const ZipFolderAccessor& other) = delete;
        ZipFolderAccessor& operator=(const ZipFolderAccessor& other) = delete;

        ~ZipFolderAccessor() override final;

        eastl::string read_text_file(const fs::path& resource_path) override final;

        eastl::vector<fs::path> get_all_items_in_folder(const fs::path& folder) override final;

    private:
        /*!
         * \brief Map from filename to its index in the zip folder. Miniz seems to like indexes
         */
        eastl::unordered_map<eastl::string, uint32_t> resource_indexes;

        mz_zip_archive zip_archive = {};

        eastl::unique_ptr<FileTreeNode> files = nullptr;

        void delete_file_tree(eastl::unique_ptr<FileTreeNode>& node);

        void build_file_tree();

        bool does_resource_exist_on_filesystem(const fs::path& resource_path) override final;
    };

    /*!
     * \brief Prints out the nodes in a depth-first fashion
     */
    void print_file_tree(const eastl::unique_ptr<FileTreeNode>& folder, uint32_t depth);
} // namespace nova::renderer
