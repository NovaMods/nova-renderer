#include "nova_renderer/resource_loader.hpp"

#include "nova_renderer/nova_renderer.hpp"

using namespace nova::mem;

namespace nova::renderer {
    RX_LOG("DeviceResources", logger);

    using namespace rhi;
    using namespace shaderpack;

    constexpr size_t STAGING_BUFFER_ALIGNMENT = 2048;
    constexpr size_t STAGING_BUFFER_TOTAL_MEMORY_SIZE = 8388608;

    constexpr size_t UNIFORM_BUFFER_ALIGNMENT = 64;           // TODO: Get a real value
    constexpr size_t UNIFORM_BUFFER_TOTAL_MEMORY_SIZE = 8096; // TODO: Get a real value

    size_t size_in_bytes(PixelFormat pixel_format);

    DeviceResources::DeviceResources(NovaRenderer& renderer)
        : renderer(renderer), device(renderer.get_engine()), internal_allocator(renderer.get_global_allocator()) {
        allocate_staging_buffer_memory();

        allocate_uniform_buffer_memory();
    }

    rx::optional<BufferResourceAccessor> DeviceResources::create_uniform_buffer(const rx::string& name, const Bytes size) {
        BufferResource resource = {};
        resource.name = name;
        resource.size = size;

        const BufferCreateInfo create_info = {rx::string::format("UniformBuffer%s", name), size.b_count(), BufferUsage::UniformBuffer};
        resource.buffer = device.create_buffer(create_info, *uniform_buffer_memory, internal_allocator);
        if(resource.buffer == nullptr) {
            logger(rx::log::level::k_error, "Could not create uniform buffer %s", name);
            return rx::nullopt;
        }

        uniform_buffers.insert(name, resource);

        return BufferResourceAccessor{&uniform_buffers, name};
    }

    rx::optional<BufferResourceAccessor> DeviceResources::get_uniform_buffer(const rx::string& name) {
        if(uniform_buffers.find(name) != nullptr) {
            return BufferResourceAccessor{&uniform_buffers, name};
        }

        return rx::nullopt;
    }

    void DeviceResources::destroy_uniform_buffer(const rx::string& name) {
        if(const BufferResource* res = uniform_buffers.find(name)) {
            // TODO device.destroy_buffer(res->buffer);
        }
        uniform_buffers.erase(name);
    }

    rx::optional<TextureResourceAccessor> DeviceResources::create_texture(const rx::string& name,
                                                                          const std::size_t width,
                                                                          const std::size_t height,
                                                                          const PixelFormat pixel_format,
                                                                          const void* data,
                                                                          rx::memory::allocator* allocator) {

        TextureResource resource = {};

        resource.name = name;
        resource.width = width;
        resource.height = height;
        resource.format = pixel_format;

        const size_t pixel_size = size_in_bytes(pixel_format);

        shaderpack::TextureCreateInfo info = {};
        info.name = name;
        info.usage = ImageUsage::SampledImage;
        info.format.pixel_format = to_pixel_format_enum(pixel_format);
        info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        info.format.width = static_cast<float>(width);
        info.format.height = static_cast<float>(height);

        resource.image = device.create_image(info, allocator);
        resource.image->is_dynamic = false;

        if(data != nullptr) {
            Buffer* staging_buffer = get_staging_buffer_with_size(width * height * pixel_size);

            CommandList* cmds = device.create_command_list(0, QueueType::Transfer, CommandList::Level::Primary, allocator);
            cmds->set_debug_name(rx::string::format("UploadTo%s", name));

            ResourceBarrier initial_texture_barrier = {};
            initial_texture_barrier.resource_to_barrier = resource.image;
            initial_texture_barrier.access_before_barrier = ResourceAccess::CopyRead;
            initial_texture_barrier.access_after_barrier = ResourceAccess::CopyWrite;
            initial_texture_barrier.old_state = ResourceState::Undefined;
            initial_texture_barrier.new_state = ResourceState::CopyDestination;
            initial_texture_barrier.source_queue = QueueType::Transfer;
            initial_texture_barrier.destination_queue = QueueType::Transfer;
            initial_texture_barrier.image_memory_barrier.aspect = ImageAspect::Color;

            rx::vector<ResourceBarrier> initial_barriers{allocator};
            initial_barriers.push_back(initial_texture_barrier);
            cmds->resource_barriers(PipelineStage::Transfer, PipelineStage::Transfer, initial_barriers);
            cmds->upload_data_to_image(resource.image, width, height, pixel_size, staging_buffer, data);

            ResourceBarrier final_texture_barrier = {};
            final_texture_barrier.resource_to_barrier = resource.image;
            final_texture_barrier.access_before_barrier = ResourceAccess::CopyWrite;
            final_texture_barrier.access_after_barrier = ResourceAccess::ShaderRead;
            final_texture_barrier.old_state = ResourceState::CopyDestination;
            final_texture_barrier.new_state = ResourceState::ShaderRead;
            final_texture_barrier.source_queue = QueueType::Transfer;
            final_texture_barrier.destination_queue = QueueType::Graphics;
            final_texture_barrier.image_memory_barrier.aspect = ImageAspect::Color;

            rx::vector<ResourceBarrier> final_barriers{allocator};
            final_barriers.push_back(final_texture_barrier);
            cmds->resource_barriers(PipelineStage::Transfer, PipelineStage::VertexShader, final_barriers);

            Fence* upload_done_fence = device.create_fence(false, allocator);
            device.submit_command_list(cmds, QueueType::Transfer, upload_done_fence);

            // Be sure that the data copy is complete, so that this method doesn't return before the GPU is done with the staging buffer
            rx::vector<Fence*> upload_done_fences{allocator};
            upload_done_fences.push_back(upload_done_fence);
            device.wait_for_fences(upload_done_fences);
            device.destroy_fences(upload_done_fences, allocator);

            return_staging_buffer(staging_buffer);

            logger(rx::log::level::k_verbose, "Uploaded texture data to texture %s", name);
        }

        textures.insert(name, resource);

        return TextureResourceAccessor{&textures, name};
    }

    rx::optional<TextureResourceAccessor> DeviceResources::get_texture(const rx::string& name) {
        if(textures.find(name) != nullptr) {
            return TextureResourceAccessor{&textures, name};

        }

#if NOVA_DEBUG
        else {
            logger(rx::log::level::k_error, "Could not find image \"%s\"", name);
        }
#endif

        return rx::nullopt;
    }

    rx::optional<TextureResourceAccessor> DeviceResources::create_render_target(const rx::string& name,
                                                                                const size_t width,
                                                                                const size_t height,
                                                                                const PixelFormat pixel_format,
                                                                                rx::memory::allocator* allocator,
                                                                                const bool /* can_be_sampled // Not yet supported */) {
        shaderpack::TextureCreateInfo create_info;
        create_info.name = name;
        create_info.usage = ImageUsage::RenderTarget;
        create_info.format.pixel_format = to_pixel_format_enum(pixel_format);
        create_info.format.dimension_type = TextureDimensionTypeEnum::Absolute;
        create_info.format.width = static_cast<float>(width);
        create_info.format.height = static_cast<float>(height);

        auto* image = device.create_image(create_info, allocator);
        if(image) {
            // Barrier it into the correct format and return it

            image->is_dynamic = true;

            TextureResource resource = {};
            resource.name = name;
            resource.format = pixel_format;
            resource.height = height;
            resource.width = width;
            resource.image = image;

            {
                CommandList* cmds = device.create_command_list(0, QueueType::Graphics, CommandList::Level::Primary, allocator);
                cmds->set_debug_name(rx::string::format("ChangeFormatOf%s", name));

                ResourceBarrier initial_texture_barrier = {};
                initial_texture_barrier.resource_to_barrier = resource.image;
                initial_texture_barrier.old_state = ResourceState::Undefined;
                initial_texture_barrier.source_queue = QueueType::Graphics;
                initial_texture_barrier.destination_queue = QueueType::Graphics;

                PipelineStage stage_after_barrier;

                if(is_depth_format(pixel_format)) {
                    initial_texture_barrier.image_memory_barrier.aspect = ImageAspect::Depth;
                    initial_texture_barrier.new_state = ResourceState::DepthWrite;
                    initial_texture_barrier.access_before_barrier = ResourceAccess::MemoryWrite;
                    initial_texture_barrier.access_after_barrier = ResourceAccess::DepthStencilAttachmentRead;

                    stage_after_barrier = PipelineStage::EarlyFragmentTests;

                } else {
                    initial_texture_barrier.image_memory_barrier.aspect = ImageAspect::Color;
                    initial_texture_barrier.new_state = ResourceState::RenderTarget;
                    initial_texture_barrier.access_before_barrier = ResourceAccess::MemoryWrite;
                    initial_texture_barrier.access_after_barrier = ResourceAccess::ColorAttachmentRead;

                    stage_after_barrier = PipelineStage::ColorAttachmentOutput;
                }

                rx::vector<ResourceBarrier> initial_barriers{allocator};
                initial_barriers.push_back(initial_texture_barrier);
                cmds->resource_barriers(PipelineStage::TopOfPipe, stage_after_barrier, initial_barriers);

                Fence* upload_done_fence = device.create_fence(false, allocator);
                device.submit_command_list(cmds, QueueType::Graphics, upload_done_fence);

                // Be sure that the data copy is complete, so that this method doesn't return before the GPU is done with the staging buffer
                rx::vector<Fence*> upload_done_fences{allocator};
                upload_done_fences.push_back(upload_done_fence);
                device.wait_for_fences(upload_done_fences);
                device.destroy_fences(upload_done_fences, allocator);
            }

            render_targets.insert(name, resource);

            return TextureResourceAccessor{&render_targets, name};

        } else {
            logger(rx::log::level::k_error, "Could not create render target %s", name);
            return rx::nullopt;
        }
    }

    rx::optional<TextureResourceAccessor> DeviceResources::get_render_target(const rx::string& name) {
        if(render_targets.find(name) != nullptr) {
            return TextureResourceAccessor{&render_targets, name};

        } else {
            return rx::nullopt;
        }
    }

    void DeviceResources::destroy_render_target(const rx::string& texture_name, rx::memory::allocator* allocator) {
        if(const auto* texture = render_targets.find(texture_name); texture != nullptr) {
            device.destroy_texture(texture->image, allocator);
            render_targets.erase(texture_name);
        }
#if NOVA_DEBUG
        else {
            logger(rx::log::level::k_error, "Could not delete texture %s, are you sure you spelled it correctly?", texture_name);
        }
#endif
    }

    void DeviceResources::allocate_staging_buffer_memory() {
        DeviceMemory* memory = device
                                   .allocate_device_memory(STAGING_BUFFER_TOTAL_MEMORY_SIZE,
                                                           MemoryUsage::StagingBuffer,
                                                           ObjectType::Buffer,
                                                           internal_allocator)
                                   .value;

        auto* strat = internal_allocator->create<BlockAllocationStrategy>(renderer.get_global_allocator(),
                                                                          Bytes(STAGING_BUFFER_TOTAL_MEMORY_SIZE),
                                                                          STAGING_BUFFER_ALIGNMENT);

        staging_buffer_memory = internal_allocator->create<DeviceMemoryResource>(memory, strat);
    }

    void DeviceResources::allocate_uniform_buffer_memory() {
        DeviceMemory* memory = device
                                   .allocate_device_memory(STAGING_BUFFER_TOTAL_MEMORY_SIZE,
                                                           MemoryUsage::LowFrequencyUpload,
                                                           ObjectType::Buffer,
                                                           internal_allocator)
                                   .value;

        auto* strat = internal_allocator->create<BlockAllocationStrategy>(renderer.get_global_allocator(),
                                                                          Bytes(UNIFORM_BUFFER_TOTAL_MEMORY_SIZE),
                                                                          UNIFORM_BUFFER_ALIGNMENT);

        uniform_buffer_memory = internal_allocator->create<DeviceMemoryResource>(memory, strat);
    }

    Buffer* DeviceResources::get_staging_buffer_with_size(const Bytes size) {
        // Align the size so we can bin the staging buffers
        // TODO: Experiment and find a good alignment
        const auto a = size.b_count() % STAGING_BUFFER_ALIGNMENT;
        const auto needed_size = STAGING_BUFFER_ALIGNMENT + a;
        const auto actual_size = size.b_count() + needed_size;

        if(auto* staging_buffer = staging_buffers.find(actual_size); staging_buffer != nullptr) {
            auto& buffer_list = *staging_buffer;
            if(buffer_list.size() > 0) {
                auto* buffer = buffer_list.last();
                buffer_list.erase(buffer_list.size() - 1, buffer_list.size() - 1);

                return buffer;
            }
        }

        const BufferCreateInfo info = {"GenericStagingBuffer", actual_size, BufferUsage::StagingBuffer};

        Buffer* buffer = device.create_buffer(info, *staging_buffer_memory, internal_allocator);
        return buffer;
    }

    void DeviceResources::return_staging_buffer(Buffer* buffer) {
        const auto size = buffer->size.b_count();
        auto* buffers = staging_buffers.find(size);
        if(!buffers) {
            buffers = staging_buffers.insert(size, {});
        }

        buffers->push_back(buffer);
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
