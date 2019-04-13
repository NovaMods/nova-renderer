/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_render_engine.hpp"

#include "../../util/logger.hpp"
#include "gl2_structs.hpp"

namespace nova::renderer::rhi {
    Gl2RenderEngine::Gl2RenderEngine(NovaSettings& settings) : RenderEngine(settings) {
        const bool loaded_opengl = gladLoadGL() != 0;
        if(!loaded_opengl) {
            NOVA_LOG(FATAL) << "Could not load OpenGL 2.1 functions, sorry bro";
            return;
        }

        open_window_and_create_surface(settings.window);

        set_initial_state();
    }

    void Gl2RenderEngine::set_initial_state() {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void Gl2RenderEngine::set_num_renderpasses([[maybe_unused]] uint32_t num_renderpasses) {
        // GL2 doesn't need to do anything either
    }

    Result<Renderpass*> Gl2RenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& data) {
        return Result<Renderpass*>(new Renderpass);
    }

    Framebuffer* Gl2RenderEngine::create_framebuffer(const Renderpass* renderpass,
                                                     const std::vector<Image*>& attachments,
                                                     const glm::uvec2& framebuffer_size) {
        GL2Framebuffer* framebuffer = new GL2Framebuffer;

        return framebuffer;
    }

    Pipeline* Gl2RenderEngine::create_pipeline(const Renderpass* renderpass, const shaderpack::PipelineCreateInfo& data) { return nullptr; }

    Buffer* Gl2RenderEngine::create_buffer(const BufferCreateInfo& info) { return nullptr; }

    Image* Gl2RenderEngine::create_texture(const shaderpack::TextureCreateInfo& info) {
        GL2Image* image = new GL2Image;

        glGenTextures(1, &image->id);
        glBindTexture(GL_TEXTURE_2D, image->id);

        GLint format = GL_RGBA;
        GLint internal_format = GL_RGBA8;
        GLint type = GL_UNSIGNED_BYTE;
        switch(info.format.pixel_format) {
            case shaderpack::PixelFormatEnum::RGBA8:
                format = GL_RGBA;
                internal_format = GL_RGBA8;
                type = GL_UNSIGNED_BYTE;
                break;

            case shaderpack::PixelFormatEnum::RGBA32F:
                NOVA_LOG(WARN) << "[GL2] 32-bits per channel texture requested, but GL2 only supports 16 bits per channel";
                [[fallthrough]];
            case shaderpack::PixelFormatEnum::RGBA16F:
                format = GL_RGBA;
                internal_format = GL_RGBA16;
                type = GL_FLOAT;
                break;

            case shaderpack::PixelFormatEnum::Depth:
                format = GL_RED;
                internal_format = GL_DEPTH_COMPONENT32;
                type = GL_INT;
                break;

            case shaderpack::PixelFormatEnum::DepthStencil:
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

    Semaphore* Gl2RenderEngine::create_semaphore() { return nullptr; }
    std::vector<Semaphore*> Gl2RenderEngine::create_semaphores(uint32_t num_semaphores) { return std::vector<Semaphore*>(); }
    Fence* Gl2RenderEngine::create_fence(bool signaled) { return nullptr; }
    std::vector<Fence*> Gl2RenderEngine::create_fences(uint32_t num_fences, bool signaled) { return std::vector<Fence*>(); }

    void Gl2RenderEngine::destroy_renderpass(Renderpass* pass) { delete pass; }

    void Gl2RenderEngine::destroy_pipeline(Pipeline* pipeline) {}

    void Gl2RenderEngine::destroy_texture(Image* resource) {
        GL2Image* gl_image = static_cast<GL2Image*>(resource);
        glDeleteTextures(1, &gl_image->id);

        delete gl_image;
    }

    void Gl2RenderEngine::destroy_semaphores(const std::vector<Semaphore*>& semaphores) {}
    void Gl2RenderEngine::destroy_fences(const std::vector<Fence*>& fences) {}
    void Gl2RenderEngine::submit_command_list(CommandList* cmds,
                                              QueueType queue,
                                              Fence* fence_to_signal,
                                              const std::vector<Semaphore*>& wait_semaphores,
                                              const std::vector<Semaphore*>& signal_semaphores) {}
} // namespace nova::renderer::rhi
