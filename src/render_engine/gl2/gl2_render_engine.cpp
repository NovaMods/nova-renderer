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

        open_window_and_create_surface(settings.window);

        set_initial_state();

    }

    void gl2_render_engine::set_initial_state() {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    result<renderpass_t*> gl2_render_engine::create_renderpass(const shaderpack::render_pass_create_info_t& data) {
        return result<renderpass_t*>(new renderpass_t);
    }

    framebuffer_t* gl2_render_engine::create_framebuffer(const std::vector<resource_t*>& attachments) { return nullptr; }
    pipeline_t* gl2_render_engine::create_pipeline(const shaderpack::pipeline_create_info_t& data) { return nullptr; }
    buffer_t* gl2_render_engine::create_buffer(const buffer_create_info_t& info) { return nullptr; }

    image_t* gl2_render_engine::create_texture(const shaderpack::texture_create_info_t& info) {
        gl_image_t* image = new gl_image_t;

        glGenTextures(1, &image->id);
        glBindTexture(GL_TEXTURE_2D, image->id);

        GLint format = GL_RGBA;
        GLint internal_format = GL_RGBA8;
        GLint type = GL_UNSIGNED_BYTE;
        switch(info.format.pixel_format) {
            case shaderpack::pixel_format_enum::RGBA8:
                format = GL_RGBA;
                internal_format = GL_RGBA8;
                type = GL_UNSIGNED_BYTE;
                break;

            case shaderpack::pixel_format_enum::RGBA32F:
                NOVA_LOG(WARN) << "[GL2] 32-bits per channel texture requested, but GL2 only supports 16 bits per channel";
                [[fallthrough]];
            case shaderpack::pixel_format_enum::RGBA16F:
                format = GL_RGBA;
                internal_format = GL_RGBA16;
                type = GL_FLOAT;
                break;

            case shaderpack::pixel_format_enum::Depth:
                format = GL_RED;
                internal_format = GL_DEPTH_COMPONENT32;
                type = GL_INT;
                break;

            case shaderpack::pixel_format_enum::DepthStencil:
                format = GL_RED;
                internal_format = GL_DEPTH_COMPONENT24;
                type = GL_INT;
                break;
        }

        const glm::uvec2 texture_size = info.format.get_size_in_pixels(swapchain_size);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture_size.x, texture_size.y, 0, format, type, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        return image;
    }

    semaphore_t* gl2_render_engine::create_semaphore() { return nullptr; }
    std::vector<semaphore_t*> gl2_render_engine::create_semaphores(uint32_t num_semaphores) { return std::vector<semaphore_t*>(); }
    fence_t* gl2_render_engine::create_fence(bool signaled) { return nullptr; }
    std::vector<fence_t*> gl2_render_engine::create_fences(uint32_t num_fences, bool signaled) { return std::vector<fence_t*>(); }

    void gl2_render_engine::destroy_renderpass(renderpass_t* pass) { delete pass; }

    void gl2_render_engine::destroy_pipeline(pipeline_t* pipeline) {}

    void gl2_render_engine::destroy_texture(image_t* resource) {
        gl_image_t* gl_image = static_cast<gl_image_t*>(resource);
        glDeleteTextures(1, &gl_image->id);

        delete gl_image;
    }

    void gl2_render_engine::destroy_semaphores(const std::vector<semaphore_t*>& semaphores) {}
    void gl2_render_engine::destroy_fences(const std::vector<fence_t*>& fences) {}
    void gl2_render_engine::submit_command_list(command_list_t* cmds,
                                                queue_type queue,
                                                fence_t* fence_to_signal,
                                                const std::vector<semaphore_t*>& wait_semaphores,
                                                const std::vector<semaphore_t*>& signal_semaphores) {}
} // namespace nova::renderer::rhi
