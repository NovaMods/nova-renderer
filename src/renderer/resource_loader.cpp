#include "nova_renderer/frontend/resource_loader.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/util/logger.hpp"

namespace nova::renderer {
    using namespace rhi;
    using namespace shaderpack;

    size_t size_in_bytes(PixelFormat pixel_format);

    ResourceStorage::ResourceStorage(NovaRenderer& renderer) : renderer(renderer), device(renderer.get_engine()) {}

    Image* ResourceStorage::create_texture(
        const std::string& name, const std::size_t width, const std::size_t height, const PixelFormat pixel_format, void* data) {
        shaderpack::TextureCreateInfo info = {};
        info.name = name;
        info.usage = ImageUsage::SampledImage;
        info.format.pixel_format = to_pixel_format_enum(pixel_format);
        info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        info.format.width = width;
        info.format.height = height;

        Image* texture = device->create_image(info);

        const size_t buffer_size = width * height * size_in_bytes(pixel_format);
        BufferCreateInfo buffer_info = {};
        buffer_info.size = buffer_size;
        buffer_info.buffer_usage = BufferUsage::StagingBuffer;

        DeviceMemory* mem = device->allocate_device_memory(buffer_size, MemoryUsage::StagingBuffer, ObjectType::Buffer).value;

        // TODO: NovaRenderer needs a cache of staging buffers we can use
        Buffer* image_upload_buffer = renderer.get_staging_buffer(buffer_size);
        device->write_data_to_buffer(data, buffer_size, 0, image_upload_buffer);

        CommandList* cmds = device->get_command_list(0, QueueType::Transfer);

        ResourceBarrier barrier = {};
        barrier.resource_to_barrier = texture;
        barrier.access_before_barrier = AccessFlags::CopyRead;
        barrier.access_after_barrier = AccessFlags::CopyWrite;
        barrier.old_state = ResourceState::Undefined;
        barrier.new_state = ResourceState::CopyDestination;
        barrier.image_memory_barrier.aspect = ImageAspectFlags::Color;

        cmds->resource_barriers(PipelineStageFlags::TopOfPipe, PipelineStageFlags::Transfer, {barrier});
        cmds->copy_buffer_to_image(image_upload_buffer, texture, );
    }

    Image* ResourceStorage::get_texture(const std::string& name) const {
#if NOVA_DEBUG
        if(const auto& itr = textures.find(name); itr != textures.end()) {
            return itr->second.texture;
        } else {
            NOVA_LOG(ERROR) << "Could not find image \"" << name << "\"";
            return nullptr;
        }
#else
        return textures.at(name);
#endif
    }

    std::optional<DescriptorSetWrite> ResourceStorage::get_descriptor_info_for_resource(const std::string& resource_name) {
        if(const auto& itr = textures.find(resource_name); itr != textures.end()) {
            DescriptorSetWrite write = {};
            write.type = DescriptorType::CombinedImageSampler;
            auto& resource_write = write.resources.emplace_back();

            resource_write.image_info.image = itr->second.texture;
            resource_write.image_info.sampler = renderer.get_point_sampler();
            resource_write.image_info.format = itr->second.format;

            return write;

        } else {
            NOVA_LOG(ERROR) << "Could not find resource named \"" << resource_name << "\"";
            return {};
        }
    }

    size_t size_in_bytes(const PixelFormat pixel_format) {
        switch(pixel_format) {
            case PixelFormat::Rgba8:
                return 4;

            case PixelFormat::Rgba16F:
                return 8;

            case PixelFormat::Rgba32F:
                return 16;

            case PixelFormat::Depth32:
                return 4;

            case PixelFormat::Depth24Stencil8:
                return 4;

            default:
                return 4;
        }
    }
} // namespace nova::renderer
