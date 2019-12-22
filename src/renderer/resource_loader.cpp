#include "nova_renderer/frontend/resource_loader.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/util/logger.hpp"

using namespace bvestl::polyalloc;

namespace nova::renderer {
    using namespace rhi;
    using namespace shaderpack;

    constexpr size_t STAGING_BUFFER_ALIGNMENT = 2048;
    constexpr size_t STAGING_BUFFER_TOTAL_MEMORY_SIZE = 8388608;

    size_t size_in_bytes(PixelFormat pixel_format);

    ResourceStorage::ResourceStorage(NovaRenderer& renderer) : renderer(renderer), device(renderer.get_engine()) {
        allocate_staging_buffer_memory();
    }

    Image* ResourceStorage::create_texture(
        const std::string& name, const std::size_t width, const std::size_t height, const PixelFormat pixel_format, void* data) {

        const size_t pixel_size = size_in_bytes(pixel_format);

        shaderpack::TextureCreateInfo info = {};
        info.name = name;
        info.usage = ImageUsage::SampledImage;
        info.format.pixel_format = to_pixel_format_enum(pixel_format);
        info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        info.format.width = static_cast<float>(width);
        info.format.height = static_cast<float>(height);

        const std::shared_ptr<Buffer> staging_buffer = get_staging_buffer_with_size(width * height * pixel_size);

        Image* texture = device->create_image(info);
        texture->is_dynamic = false;

        CommandList* cmds = device->get_command_list(0, QueueType::Transfer);

        ResourceBarrier initial_texture_barrier = {};
        initial_texture_barrier.resource_to_barrier = texture;
        initial_texture_barrier.access_before_barrier = AccessFlags::CopyRead;
        initial_texture_barrier.access_after_barrier = AccessFlags::CopyWrite;
        initial_texture_barrier.old_state = ResourceState::Undefined;
        initial_texture_barrier.new_state = ResourceState::CopyDestination;
        initial_texture_barrier.image_memory_barrier.aspect = ImageAspectFlags::Color;

        cmds->resource_barriers(PipelineStageFlags::TopOfPipe, PipelineStageFlags::Transfer, {initial_texture_barrier});
        cmds->upload_data_to_image(texture, width, height, pixel_size, staging_buffer.get(), data);

        ResourceBarrier final_texture_barrier = {};
        final_texture_barrier.resource_to_barrier = texture;
        final_texture_barrier.access_before_barrier = AccessFlags::CopyWrite;
        final_texture_barrier.access_after_barrier = AccessFlags::ShaderRead;
        final_texture_barrier.old_state = ResourceState::CopyDestination;
        final_texture_barrier.new_state  = ResourceState::ShaderRead;
        final_texture_barrier.image_memory_barrier.aspect = ImageAspectFlags::Color;

        cmds->resource_barriers(PipelineStageFlags::Transfer, PipelineStageFlags::AllGraphics, {final_texture_barrier});

        Fence* upload_done_fence = device->create_fence();
        device->submit_command_list(cmds, QueueType::Transfer, upload_done_fence);

        // Be sure that the data copy is complete, so that this method doesn't return before the GPU is done with the staging buffer
        device->wait_for_fences({upload_done_fence});
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

    void ResourceStorage::allocate_staging_buffer_memory() {
        DeviceMemory*
            memory = device->allocate_device_memory(STAGING_BUFFER_TOTAL_MEMORY_SIZE, MemoryUsage::StagingBuffer, ObjectType::Buffer).value;

        staging_buffer_memory = new DeviceMemoryResource(memory,
                                                         new BlockAllocationStrategy(*renderer.get_global_allocator(),
                                                                                     Bytes(STAGING_BUFFER_TOTAL_MEMORY_SIZE),
                                                                                     STAGING_BUFFER_ALIGNMENT));
    }

    std::shared_ptr<Buffer> ResourceStorage::get_staging_buffer_with_size(const size_t size) {
        const auto return_staging_buffer = [&](Buffer* buf) { staging_buffers[size].push_back(buf); };

        // Align the size so we can bin the staging buffers
        // TODO: Experiment and find a good alignment
        const auto a = size % STAGING_BUFFER_ALIGNMENT;
        const auto needed_size = STAGING_BUFFER_ALIGNMENT + a;
        const auto actual_size = size + needed_size;

        if(auto itr = staging_buffers.find(actual_size); itr != staging_buffers.end()) {
            auto& buffer_list = itr->second;
            if(!buffer_list.empty()) {
                auto* buffer = buffer_list.back();
                buffer_list.pop_back();

                // Specify a custom deleter like a boss
                return std::shared_ptr<Buffer>(buffer, return_staging_buffer);
            }
        }

        const BufferCreateInfo info = {actual_size, BufferUsage::StagingBuffer};

        Buffer* buffer = device->create_buffer(info, *staging_buffer_memory);
        return std::shared_ptr<Buffer>(buffer, return_staging_buffer);
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
