#include "nova_renderer/render_engine.hpp"

namespace nova::renderer::rhi {
    void RenderEngine::set_shaderpack_data_allocator(bvestl::polyalloc::allocator_handle allocator_handle) {
        shaderpack_allocator = allocator_handle;
    }
}