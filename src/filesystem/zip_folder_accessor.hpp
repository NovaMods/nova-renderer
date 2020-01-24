#pragma once

#include <memory>

#include <miniz.h>

#include "nova_renderer/filesystem/folder_accessor.hpp"

namespace nova::filesystem {
    struct FileTreeNode {
        rx::string name;
        rx::vector<std::unique_ptr<FileTreeNode>> children;
        FileTreeNode* parent = nullptr;

        [[nodiscard]] rx::string get_full_path() const;
    };

    /*!
     * \brief Allows access to a zip folder
     */
    class ZipFolderAccessor : public FolderAccessorBase {
    public:
        explicit ZipFolderAccessor(const rx::string& folder);

        ZipFolderAccessor(const rx::string& folder, mz_zip_archive archive);

        ZipFolderAccessor(ZipFolderAccessor&& other) noexcept = default;
        ZipFolderAccessor& operator=(ZipFolderAccessor&& other) noexcept = default;

        ZipFolderAccessor(const ZipFolderAccessor& other) = delete;
        ZipFolderAccessor& operator=(const ZipFolderAccessor& other) = delete;

        ~ZipFolderAccessor() override;

        rx::vector<uint8_t> read_file(const rx::string& path) override final;

        rx::vector<rx::string> get_all_items_in_folder(const rx::string& folder) override final;

        FolderAccessorBase* create_subfolder_accessor(const rx::string& path) const override;

    private:
        /*!
         * \brief Map from filename to its index in the zip folder. Miniz seems to like indexes
         */
        rx::map<rx::string, uint32_t> resource_indexes;

        mz_zip_archive zip_archive = {};

        std::unique_ptr<FileTreeNode> files = nullptr;

        void delete_file_tree(std::unique_ptr<FileTreeNode>& node);

        void build_file_tree();

        bool does_resource_exist_on_filesystem(const rx::string& resource_path) override final;
    };

    /*!
     * \brief Prints out the nodes in a depth-first fashion
     */
    void print_file_tree(const std::unique_ptr<FileTreeNode>& folder, uint32_t depth);
} // namespace nova::filesystem
