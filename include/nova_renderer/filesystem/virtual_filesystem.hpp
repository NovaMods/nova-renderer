#pragma once
#include <memory>
#include <vector>

#include "nova_renderer/util/filesystem.hpp"
#include "folder_accessor.hpp"

namespace nova::filesystem {
    /*!
     * Nova's virtual filesystem
     *
     * The virtual filesystem may have one or more filesystem roots. When you request access to a file at a specific path, the virtual
     * filesystem looks for it in all the filesystem roots, in their priority order
     *
     * Resource roots may be either the path to a zip file or the path to a filesystem directory
     */
    class VirtualFilesystem {
        friend class std::shared_ptr<VirtualFilesystem>;
    public:
        [[nodiscard]] static std::shared_ptr<VirtualFilesystem> get_instance();

        [[nodiscard]] std::shared_ptr<FolderAccessorBase> get_folder_accessor(const fs::path& path) const;

    private:
        static std::shared_ptr<VirtualFilesystem> instance;

        std::vector<std::shared_ptr<FolderAccessorBase>> resource_roots;

        VirtualFilesystem();
    };
} // namespace nova::fs