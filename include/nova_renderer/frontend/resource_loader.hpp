#pragma once

#include <cstddef>
#include <unordered_map>

#include "nova_renderer/rhi/device_memory_resource.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"
#include <nova_renderer/memory/allocators.hpp>

namespace nova::renderer {
    class NovaRenderer;

    namespace rhi {
        enum class PixelFormat;
    }

    namespace shaderpack {
        // yolo
        enum class PixelFormatEnum;
    } // namespace shaderpack

    struct TextureResource {
        std::string name;

        rhi::Image* image;

        size_t width;

        size_t height;

        rhi::PixelFormat format;
    };

    /*!
     * \brief Provides a means to access Nova's resources, and also helps in creating resources? IDK yet but that's fine
     *
     * Basically I need both a high-level API to make resources with, and I want to make those resource easy to access.
     */
    class ResourceStorage {
    public:
        explicit ResourceStorage(NovaRenderer& renderer);

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
        [[nodiscard]] TextureResource create_texture(const std::string& name,
                                                     std::size_t width,
                                                     std::size_t height,
                                                     rhi::PixelFormat pixel_format,
                                                     void* data,
                                                     mem::AllocatorHandle<>& allocator);

        /*!
         * \brief Retrieves the texture with the specified name
         */
        [[nodiscard]] std::optional<TextureResource> get_texture(const std::string& name) const;

        [[nodiscard]] std::optional<rhi::DescriptorSetWrite> get_descriptor_info_for_resource(const std::string& resource_name);

        [[nodiscard]] TextureResource create_render_target(const std::string& name,
                                                           size_t width,
                                                           size_t height,
                                                           rhi::PixelFormat pixel_format);

    private:
        NovaRenderer& renderer;

        rhi::RenderEngine* device;

        std::unordered_map<std::string, TextureResource> textures;

        std::unordered_map<std::string, TextureResource> render_targets;

        DeviceMemoryResource* staging_buffer_memory;

        std::unordered_map<size_t, std::vector<rhi::Buffer*>> staging_buffers;

        void allocate_staging_buffer_memory();

        /*!
         * \brief Retrieves a staging buffer at least the specified size
         *
         * The actual buffer returned may be larger than what you need
         */
        std::shared_ptr<rhi::Buffer> get_staging_buffer_with_size(size_t size);
    };
} // namespace nova::renderer
