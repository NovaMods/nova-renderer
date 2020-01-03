#include "nova_renderer/frontend/resource_loader.hpp"

#include <utility>

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/util/logger.hpp"

using namespace nova::mem;

namespace nova::renderer {
    using namespace rhi;
    using namespace shaderpack;

    constexpr size_t STAGING_BUFFER_ALIGNMENT = 2048;
    constexpr size_t STAGING_BUFFER_TOTAL_MEMORY_SIZE = 8388608;

    size_t size_in_bytes(PixelFormat pixel_format);

    ResourceStorage::ResourceStorage(NovaRenderer& renderer) : renderer(renderer), device(renderer.get_engine()) {
        allocate_staging_buffer_memory();
    }

    std::optional<TextureResourceAccessor> ResourceStorage::create_texture(const std::string& name,
                                                                           const std::size_t width,
                                                                           const std::size_t height,
                                                                           const PixelFormat pixel_format,
                                                                           void* data,
                                                                           AllocatorHandle<>& allocator) {

        const auto resource = allocator.allocate_shared<TextureResource>(
            [&](const auto* tex) { device->destroy_texture(tex->image, allocator); });

        resource->name = name;
        resource->width = width;
        resource->height = height;
        resource->format = pixel_format;

        const size_t pixel_size = size_in_bytes(pixel_format);

        shaderpack::TextureCreateInfo info = {};
        info.name = name;
        info.usage = ImageUsage::SampledImage;
        info.format.pixel_format = to_pixel_format_enum(pixel_format);
        info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        info.format.width = static_cast<float>(width);
        info.format.height = static_cast<float>(height);

        const std::shared_ptr<Buffer> staging_buffer = get_staging_buffer_with_size(width * height * pixel_size);

        resource->image = device->create_image(info, allocator);
        resource->image->is_dynamic = false;

        {
            CommandList* cmds = device->create_command_list(allocator, 0, QueueType::Transfer);

            ResourceBarrier initial_texture_barrier = {};
            initial_texture_barrier.resource_to_barrier = resource->image;
            initial_texture_barrier.access_before_barrier = AccessFlags::CopyRead;
            initial_texture_barrier.access_after_barrier = AccessFlags::CopyWrite;
            initial_texture_barrier.old_state = ResourceState::Undefined;
            initial_texture_barrier.new_state = ResourceState::CopyDestination;
            initial_texture_barrier.image_memory_barrier.aspect = ImageAspectFlags::Color;

            cmds->resource_barriers(PipelineStageFlags::TopOfPipe, PipelineStageFlags::Transfer, {initial_texture_barrier});
            cmds->upload_data_to_image(resource->image, width, height, pixel_size, staging_buffer.get(), data);

            ResourceBarrier final_texture_barrier = {};
            final_texture_barrier.resource_to_barrier = resource->image;
            final_texture_barrier.access_before_barrier = AccessFlags::CopyWrite;
            final_texture_barrier.access_after_barrier = AccessFlags::ShaderRead;
            final_texture_barrier.old_state = ResourceState::CopyDestination;
            final_texture_barrier.new_state = ResourceState::ShaderRead;
            final_texture_barrier.image_memory_barrier.aspect = ImageAspectFlags::Color;

            cmds->resource_barriers(PipelineStageFlags::Transfer, PipelineStageFlags::AllGraphics, {final_texture_barrier});

            Fence* upload_done_fence = device->create_fence(allocator);
            device->submit_command_list(cmds, QueueType::Transfer, upload_done_fence);

            // Be sure that the data copy is complete, so that this method doesn't return before the GPU is done with the staging buffer
            device->wait_for_fences({upload_done_fence});
            device->destroy_fences({upload_done_fence}, allocator);
        }

        textures.emplace(name, resource);

        return std::make_optional<TextureResourceAccessor>(&textures, name);
    }

    std::optional<TextureResourceAccessor> ResourceStorage::get_texture(const std::string& name) const {
#if NOVA_DEBUG
        if(textures.find(name) != textures.end()) {
            return std::make_optional<TextureResourceAccessor>(&textures, name);

        } else {
            NOVA_LOG(ERROR) << "Could not find image \"" << name << "\"";
            return {};
        }
#else
        return std::make_optional(TextureResourceAccessor(&textures, name));
#endif
    }

    std::optional<DescriptorSetWrite> ResourceStorage::get_descriptor_info_for_resource(const std::string& resource_name) {
        if(const auto& itr = textures.find(resource_name); itr != textures.end()) {
            DescriptorSetWrite write = {};
            write.type = DescriptorType::CombinedImageSampler;
            auto& resource_write = write.resources.emplace_back();

            resource_write.image_info.image = itr->second.image;
            resource_write.image_info.sampler = renderer.get_point_sampler();
            resource_write.image_info.format.pixel_format = to_pixel_format_enum(itr->second.format);
            resource_write.image_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
            resource_write.image_info.format.width = static_cast<float>(itr->second.width);
            resource_write.image_info.format.height = static_cast<float>(itr->second.height);

            return write;

        } else {
            NOVA_LOG(ERROR) << "Could not find resource named \"" << resource_name << "\"";
            return {};
        }
    }

    std::optional<TextureResourceAccessor> ResourceStorage::create_render_target(const std::string& name,
                                                                                 const size_t width,
                                                                                 const size_t height,
                                                                                 const PixelFormat pixel_format,
                                                                                 AllocatorHandle<>& allocator,
                                                                                 const bool /* can_be_sampled // Not yet supported */) {
        shaderpack::TextureCreateInfo create_info;
        create_info.name = name;
        create_info.usage = ImageUsage::RenderTarget;
        create_info.format.pixel_format = to_pixel_format_enum(pixel_format);
        create_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        create_info.format.width = static_cast<float>(width);
        create_info.format.height = static_cast<float>(height);

        auto* image = device->create_image(create_info, allocator);
        if(image) {
            // Barrier it into the correct format and return it

            image->is_dynamic = false;

            auto resource = allocator.allocate_shared<TextureResource>(
                [&](const auto* tex) { device->destroy_texture(tex->image, allocator); });

            {
                CommandList* cmds = device->create_command_list(allocator, 0, QueueType::Transfer);

                ResourceBarrier initial_texture_barrier = {};
                initial_texture_barrier.resource_to_barrier = resource->image;
                initial_texture_barrier.access_before_barrier = AccessFlags::ColorAttachmentRead;
                initial_texture_barrier.access_after_barrier = AccessFlags::ColorAttachmentWrite;
                initial_texture_barrier.old_state = ResourceState::Undefined;
                initial_texture_barrier.new_state = ResourceState::RenderTarget;
                initial_texture_barrier.image_memory_barrier.aspect = ImageAspectFlags::Color;

                cmds->resource_barriers(PipelineStageFlags::TopOfPipe,
                                        PipelineStageFlags::ColorAttachmentOutput,
                                        {initial_texture_barrier});

                Fence* upload_done_fence = device->create_fence(allocator);
                device->submit_command_list(cmds, QueueType::Transfer, upload_done_fence);

                // Be sure that the barrier is complete, so that this method doesn't return before the render target is ready to use
                device->wait_for_fences({upload_done_fence});
                device->destroy_fences({upload_done_fence}, allocator);
            }

            render_targets.emplace(name, resource);

            return std::make_optional<TextureResourceAccessor>(&render_targets, name);

        } else {
            NOVA_LOG(ERROR) << "Could not create render target " << name;
            return {};
        }
    }

    std::optional<TextureResourceAccessor> ResourceStorage::get_render_target(const std::string& name) const {
        if(render_targets.find(name) != render_targets.end()) {
            return std::make_optional<TextureResourceAccessor>(&render_targets, name);

        } else {
            return {};
        }
    }

    void ResourceStorage::allocate_staging_buffer_memory() {
        DeviceMemory* memory = device
                                   ->allocate_device_memory(STAGING_BUFFER_TOTAL_MEMORY_SIZE,
                                                            MemoryUsage::StagingBuffer,
                                                            ObjectType::Buffer,
                                                            *staging_buffer_allocator)
                                   .value;

        auto* strat = staging_buffer_allocator->new_other_object<BlockAllocationStrategy>(renderer.get_global_allocator().get(),
                                                                                          Bytes(STAGING_BUFFER_TOTAL_MEMORY_SIZE),
                                                                                          STAGING_BUFFER_ALIGNMENT);

        staging_buffer_memory = staging_buffer_allocator->new_other_object<DeviceMemoryResource>(memory, strat);
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

        Buffer* buffer = device->create_buffer(info, *staging_buffer_memory, *staging_buffer_allocator);
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
