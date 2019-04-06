/*!
 * \author ddubois 
 * \date 03-Apr-19.
 */

#include "d3d12_render_engine.hpp"

namespace nova::renderer {
    d3d12_render_engine::d3d12_render_engine(nova_settings& settings) : render_engine(settings) {}

    std::shared_ptr<window_t> d3d12_render_engine::get_window() const { return std::shared_ptr<window>(); }

    result<renderpass_t*> d3d12_render_engine::create_renderpass(const render_pass_create_info_t& data) {
        return result<renderpass_t*>(new d3d12_renderpass_t);
    }

    framebuffer_t* d3d12_render_engine::create_framebuffer(const std::vector<resource_t*>& attachments) { return nullptr; }

    pipeline_t* d3d12_render_engine::create_pipeline(const pipeline_create_info_t& data) { return nullptr; }

    resource_t* d3d12_render_engine::create_buffer(const buffer_create_info_t& info) { return nullptr; }

    resource_t* d3d12_render_engine::create_texture(const texture2d_create_info_t& info) { return nullptr; }

    semaphore_t* d3d12_render_engine::create_semaphore() { return nullptr; }

    std::vector<semaphore_t*> d3d12_render_engine::create_semaphores(uint32_t num_semaphores) {
        return std::vector<semaphore_t*>();
    }

    fence_t* d3d12_render_engine::create_fence(bool signaled = false) { return nullptr; }

    std::vector<fence_t*> d3d12_render_engine::create_fences(uint32_t num_fences, bool signaled = false) {
        return std::vector<fence_t*>();
    }

    void d3d12_render_engine::destroy_renderpass(renderpass_t* pass) {}

    void d3d12_render_engine::destroy_pipeline(pipeline_t* pipeline) {}

    void d3d12_render_engine::destroy_resource(resource_t* resource) {}

    void d3d12_render_engine::destroy_semaphores(const std::vector<semaphore_t*>& semaphores) {}

    void d3d12_render_engine::destroy_fences(const std::vector<fence_t*>& fences) {}

    command_list_t* d3d12_render_engine::allocate_command_list(uint32_t thread_idx,
                                                                               queue_type needed_queue_type,
                                                                               command_list_t::level level) {
        return nullptr;
    }

    void d3d12_render_engine::submit_command_list(command_list_t* cmds,
                                                                  queue_type queue,
                                                                  fence_t* fence_to_signal = nullptr,
                                                                  const std::vector<semaphore_t*>& wait_semaphores = {},
                                                                  const std::vector<semaphore_t*>& signal_semaphores = {}) {}

    void d3d12_render_engine::open_window_and_create_surface(const nova_settings::window_options& options) {}
} // namespace nova::renderer
