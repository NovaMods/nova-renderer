/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include <spirv_cross/spirv_glsl.hpp>

#include "gl3_render_engine.hpp"

#include "../../util/logger.hpp"
#include "gl3_structs.hpp"

namespace nova::renderer::rhi {
    Gl3RenderEngine::Gl3RenderEngine(NovaSettings& settings) : RenderEngine(settings) {
        const bool loaded_opengl = gladLoadGL() != 0;
        if(!loaded_opengl) {
            NOVA_LOG(FATAL) << "Could not load OpenGL 3.1 functions, sorry bro";
            return;
        }

        open_window_and_create_surface(settings.window);

        set_initial_state();
    }

    void Gl3RenderEngine::set_initial_state() {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void Gl3RenderEngine::set_num_renderpasses([[maybe_unused]] uint32_t num_renderpasses) {
        // Gl3 doesn't need to do anything either
    }

    Result<Renderpass*> Gl3RenderEngine::create_renderpass([[maybe_unused]] const shaderpack::RenderPassCreateInfo& data) {
        return Result<Renderpass*>(new Renderpass);
    }

    Framebuffer* Gl3RenderEngine::create_framebuffer([[maybe_unused]] const Renderpass* renderpass,
                                                     const std::vector<Image*>& attachments,
                                                     [[maybe_unused]] const glm::uvec2& framebuffer_size) {
        Gl3Framebuffer* framebuffer = new Gl3Framebuffer;

        glGenFramebuffers(1, &framebuffer->id);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);

        uint32_t attachment_idx = 0;
        for(const Image* attachment : attachments) {
            const Gl3Image* gl_attachment = static_cast<const Gl3Image*>(attachment);

            GLenum attachment_slot = GL_COLOR_ATTACHMENT0 + attachment_idx;
            if(gl_attachment->is_depth_tex) {
                attachment_slot = GL_DEPTH_ATTACHMENT;

            } else {
                attachment_idx++;
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_slot, GL_TEXTURE_2D, gl_attachment->id, 0);
        }

        return framebuffer;
    }

    DescriptorPool* Gl3RenderEngine::create_descriptor_pool(const uint32_t num_sampled_images,
                                                            const uint32_t num_samplers,
                                                            const uint32_t num_uniform_buffers) {
        Gl3DescriptorPool* pool = new Gl3DescriptorPool;
        pool->sets.resize(num_sampled_images + num_uniform_buffers);
        pool->sampler_sets.resize(num_samplers);

        return pool;
    }

    std::vector<DescriptorSet*> Gl3RenderEngine::create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                        const DescriptorPool* pool) {}

    Result<PipelineInterface*> Gl3RenderEngine::create_pipeline_interface(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
        [[maybe_unused]] const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        [[maybe_unused]] const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) {
        Gl3PipelineInterface* pipeline_interface = new Gl3PipelineInterface;
        pipeline_interface->bindings = bindings;

        return Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    Result<Pipeline*> Gl3RenderEngine::create_pipeline(const PipelineInterface* pipeline_interface,
                                                       const shaderpack::PipelineCreateInfo& data) {
        Gl3Pipeline* pipeline = new Gl3Pipeline;

        pipeline->id = glCreateProgram();

        std::vector<std::string> pipeline_creation_errors;
        pipeline_creation_errors.reserve(4);

        const Result<GLuint> vertex_shader = compile_shader(data.vertex_shader.source, GL_VERTEX_SHADER);
        if(vertex_shader) {
            glAttachShader(pipeline->id, vertex_shader.value);

        } else {
            pipeline_creation_errors.push_back(vertex_shader.error.to_string());
        }

        if(supports_geometry_shaders && data.geometry_shader) {
            const Result<GLuint> geometry_shader = compile_shader(data.geometry_shader->source, GL_GEOMETRY_SHADER_ARB);
            if(geometry_shader) {
                glAttachShader(pipeline->id, geometry_shader.value);

            } else {
                pipeline_creation_errors.push_back(geometry_shader.error.to_string());
            }
        }

        if(data.fragment_shader) {
            const Result<GLuint> fragment_shader = compile_shader(data.fragment_shader->source, GL_FRAGMENT_SHADER);
            if(fragment_shader) {
                glAttachShader(pipeline->id, fragment_shader.value);

            } else {
                pipeline_creation_errors.push_back(fragment_shader.error.to_string());
            }
        }

        glLinkProgram(pipeline->id);

        GLint link_log_length;
        glGetProgramiv(pipeline->id, GL_INFO_LOG_LENGTH, &link_log_length);
        if(link_log_length > 0) {
            std::string program_link_log;
            program_link_log.reserve(link_log_length);
            glGetProgramInfoLog(pipeline->id, link_log_length, nullptr, program_link_log.data());

            pipeline_creation_errors.push_back(program_link_log);

            return Result<Pipeline*>(NovaError(program_link_log));
        }

        const Gl3PipelineInterface* gl3_pipeline_interface = static_cast<const Gl3PipelineInterface*>(pipeline_interface);
        for(const auto& binding : gl3_pipeline_interface->bindings) {
            const GLuint uniform_location = glGetUniformLocation(pipeline->id, binding.first.c_str());
            pipeline->uniform_cache.emplace(binding.first, uniform_location);
        }

        return Result(static_cast<Pipeline*>(pipeline));
    }

    Buffer* Gl3RenderEngine::create_buffer(const BufferCreateInfo& info) { return nullptr; }

    Image* Gl3RenderEngine::create_texture(const shaderpack::TextureCreateInfo& info) {
        Gl3Image* image = new Gl3Image;

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
                NOVA_LOG(WARN) << "[Gl3] 32-bits per channel texture requested, but Gl3 only supports 16 bits per channel";
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

    Semaphore* Gl3RenderEngine::create_semaphore() { return nullptr; }
    std::vector<Semaphore*> Gl3RenderEngine::create_semaphores(uint32_t num_semaphores) { return std::vector<Semaphore*>(); }
    Fence* Gl3RenderEngine::create_fence(bool signaled) { return nullptr; }
    std::vector<Fence*> Gl3RenderEngine::create_fences(uint32_t num_fences, bool signaled) { return std::vector<Fence*>(); }

    void Gl3RenderEngine::destroy_renderpass(Renderpass* pass) { delete pass; }

    void Gl3RenderEngine::destroy_framebuffer(const Framebuffer* framebuffer) {}

    void Gl3RenderEngine::destroy_pipeline(Pipeline* pipeline) {}

    void Gl3RenderEngine::destroy_texture(Image* resource) {
        Gl3Image* gl_image = static_cast<Gl3Image*>(resource);
        glDeleteTextures(1, &gl_image->id);

        delete gl_image;
    }

    void Gl3RenderEngine::destroy_semaphores(const std::vector<Semaphore*>& semaphores) {}
    void Gl3RenderEngine::destroy_fences(const std::vector<Fence*>& fences) {}
    void Gl3RenderEngine::submit_command_list(CommandList* cmds,
                                              QueueType queue,
                                              Fence* fence_to_signal,
                                              const std::vector<Semaphore*>& wait_semaphores,
                                              const std::vector<Semaphore*>& signal_semaphores) {}

    Result<GLuint> compile_shader(const std::vector<uint32_t>& spirv, const GLenum shader_type) {
        spirv_cross::CompilerGLSL compiler(spirv);
        const std::string glsl = compiler.compile();
        const char* glsl_c = glsl.c_str();
        const GLint len = glsl.size(); // SIGNED LENGTH WHOOOOOO

        const GLuint shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &glsl_c, &len);
        glCompileShader(shader);

        GLint compile_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &compile_log_length);
        if(compile_log_length > 0) {
            std::string compile_log;
            compile_log.reserve(compile_log_length);
            glGetShaderInfoLog(shader, compile_log_length, nullptr, compile_log.data());
            return Result<GLuint>(NovaError(compile_log));
        }

        return Result(shader);
    }
} // namespace nova::renderer::rhi
