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

        /*!
         * \brief Adds the provided path to the resource roots that the virtual filesystem will care about
         */
        void add_resource_root(const fs::path& root);

        /*!
         * \brief Adds the provided folder accessor as an accessor for one of our root directories
         *
         * This method lets you add a custom folder accessor as a resource root. This allows for e.g. the Minecraft adapter to register a
         * shaderpack accessor which transpiles the shaders from GLSL 120 to SPIR-V
         */
        void add_resource_root(std::shared_ptr<FolderAccessorBase> root_accessor);

        [[nodiscard]] std::shared_ptr<FolderAccessorBase> get_folder_accessor(const fs::path& path) const;

    private:
        static std::shared_ptr<VirtualFilesystem> instance;

        /*!
         * \brief Accessor for Nova's working directory
         */
        std::shared_ptr<FolderAccessorBase> filesystem_root;

        std::vector<std::shared_ptr<FolderAccessorBase>> resource_roots;

        VirtualFilesystem();
    };
} // namespace nova::filesystem