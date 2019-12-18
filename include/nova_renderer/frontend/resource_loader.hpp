#pragma once

#include <cstddef>
#include <unordered_map>

#include "nova_renderer/rhi/forward_decls.hpp"

namespace nova::renderer {
    namespace rhi {
        enum class PixelFormat;
    }

    namespace shaderpack {
        // yolo
        enum class PixelFormatEnum;
    } // namespace shaderpack

    /*!
     * \brief Provides a means to access Nova's resources, and also helps in creating resources? IDK yet but that's fine
     *
     * Basically I need both a high-level API to make resources with, and I want to make those resource easy to access.
     */
    class ResourceFactory {
    public:
        explicit ResourceFactory(rhi::RenderEngine& device);

        /*!
         * \brief Creates a new dynamic texture with the provided initial texture data
         *
         * \param name The name of the texture. After the texture can been created, you can use this to refer to it
         * \param width The width of the texture
         * \param height The height of the texture
         * \param pixel_format The format of the pixels in this texture
         * \param data The initial data for this texture. Must be large enough to have all the pixels in the texture
         * \return The newly-created image, or nullptr if the image could not be created. Check the Nova logs to find out why
         */
        [[nodiscard]] rhi::Image* add_texture(
            const std::string& name, std::size_t width, std::size_t height, rhi::PixelFormat pixel_format, void* data);

        /*!
         * \brief Retrieves the texture with the specified name
         */
        [[nodiscard]] rhi::Image* get_texture(const std::string& name) const;

    private:
        rhi::RenderEngine& device;

        std::unordered_map<std::string, rhi::Image*> textures;
    };
} // namespace nova::renderer
