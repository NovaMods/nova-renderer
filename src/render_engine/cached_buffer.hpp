#pragma once

#include "nova_renderer/device_memory_resource.hpp"
#include "nova_renderer/render_engine.hpp"

namespace nova::renderer {
    namespace rhi {
        class RenderDevice;
    }

    /*!
     * \brief A buffer which is cached in host memory
     */
    template <typename BufferData>
    class CachedBuffer {
    public:
        explicit CachedBuffer(DeviceMemoryResource& buffer_memory, rhi::RenderDevice& render_engine) : render_engine(&render_engine) {
            rhi::BufferCreateInfo create_info = {};
            create_info.size = sizeof(BufferData);
            create_info.buffer_usage = rhi::BufferUsage::UniformBuffer;

            gpu_buffer = render_engine.create_buffer(create_info, buffer_memory);
        }

        void set_data(const BufferData& new_data) {
            if(render_engine) {
            }
        }

    private:
        rhi::RenderDevice* render_engine;

        rhi::Buffer* gpu_buffer;
    };
} // namespace nova::renderer