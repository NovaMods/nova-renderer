/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_render_engine.hpp"
#include "gl2_command_list.hpp"

#include "../../util/logger.hpp"

namespace nova::renderer::rhi {
    gl2_render_engine::gl2_render_engine(nova_settings& settings) : render_engine_t(settings) {
        const bool loaded_opengl = gladLoadGL() != 0;
        if(!loaded_opengl) {
            NOVA_LOG(FATAL) << "Could not load OpenGL 2.1 functions, sorry bro";
            return;
        }

        set_initial_state();
    }

    void gl2_render_engine::set_initial_state() {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    result<renderpass_t*> gl2_render_engine::create_renderpass(const render_pass_create_info_t& data) {
        return result<renderpass_t*>(new renderpass_t);
    }

    framebuffer_t* gl2_render_engine::create_framebuffer(const std::vector<resource_t*>& attachments) { return nullptr; }
    pipeline_t* gl2_render_engine::create_pipeline(const pipeline_create_info_t& data) { return nullptr; }
    resource_t* gl2_render_engine::create_buffer(const buffer_create_info_t& info) { return nullptr; }
    resource_t* gl2_render_engine::create_texture(const texture2d_create_info_t& info) { return nullptr; }
    semaphore_t* gl2_render_engine::create_semaphore() { return nullptr; }
    std::vector<semaphore_t*> gl2_render_engine::create_semaphores(uint32_t num_semaphores) { return std::vector<semaphore_t*>(); }
    fence_t* gl2_render_engine::create_fence(bool signaled) { return nullptr; }
    std::vector<fence_t*> gl2_render_engine::create_fences(uint32_t num_fences, bool signaled) { return std::vector<fence_t*>(); }
    void gl2_render_engine::destroy_renderpass(renderpass_t* pass) {}
    void gl2_render_engine::destroy_pipeline(pipeline_t* pipeline) {}
    void gl2_render_engine::destroy_resource(resource_t* resource) {}
    void gl2_render_engine::destroy_semaphores(const std::vector<semaphore_t*>& semaphores) {}
    void gl2_render_engine::destroy_fences(const std::vector<fence_t*>& fences) {}
    void gl2_render_engine::submit_command_list(command_list_t* cmds,
                                                queue_type queue,
                                                fence_t* fence_to_signal,
                                                const std::vector<semaphore_t*>& wait_semaphores,
                                                const std::vector<semaphore_t*>& signal_semaphores) {}
} // namespace nova::renderer
