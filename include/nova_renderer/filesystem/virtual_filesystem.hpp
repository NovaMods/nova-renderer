#pragma once

#include "folder_accessor.hpp"
#include <rx/core/ptr.h>

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
    public:
        VirtualFilesystem();

        [[nodiscard]] static VirtualFilesystem* get_instance();

        /*!
         * \brief Adds the provided path to the resource roots that the virtual filesystem will care about
         */
        void add_resource_root(const rx::string& root);

        /*!
         * \brief Adds the provided folder accessor as an accessor for one of our root directories
         *
         * This method lets you add a custom folder accessor as a resource root. This allows for e.g. the Minecraft adapter to register a
         * shaderpack accessor which transpiles the shaders from GLSL 120 to SPIR-V
         */
        void add_resource_root(FolderAccessorBase* root_accessor);

        [[nodiscard]] FolderAccessorBase* get_folder_accessor(const rx::string& path) const;

    private:
        static rx::ptr<VirtualFilesystem> instance;

        rx::vector<FolderAccessorBase*> resource_roots;
    };
} // namespace nova::filesystem
