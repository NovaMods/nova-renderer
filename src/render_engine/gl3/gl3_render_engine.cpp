/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include <spirv_cross/spirv_glsl.hpp>

#include "gl3_render_engine.hpp"

#include "../../../tests/src/general_test_setup.hpp"
#include "../../util/logger.hpp"
#include "gl3_command_list.hpp"
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

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
    }

    void Gl3RenderEngine::set_num_renderpasses([[maybe_unused]] uint32_t num_renderpasses) {
        // Gl3 doesn't need to do anything either
    }

    Result<DeviceMemory*> Gl3RenderEngine::allocate_device_memory(const uint64_t /* size */,
                                                                  const MemoryUsage /* type */,
                                                                  const ObjectType /* allowed_objects */) {
        return Result(new DeviceMemory);
    }

    Result<Renderpass*> Gl3RenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& /* data */) {
        return Result<Renderpass*>(new Renderpass);
    }

    Framebuffer* Gl3RenderEngine::create_framebuffer(const Renderpass* /* renderpass */,
                                                     const std::vector<Image*>& attachments,
                                                     const glm::uvec2& /* framebuffer_size */) {
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
        pool->descriptors.resize(num_sampled_images + num_uniform_buffers);
        pool->sampler_sets.resize(num_samplers);

        return pool;
    }

    std::vector<DescriptorSet*> Gl3RenderEngine::create_descriptor_sets(const PipelineInterface* pipeline_interface,
                                                                        const DescriptorPool* pool) {}

    void Gl3RenderEngine::update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) {
        for(DescriptorSetWrite& write : writes) {
            const Gl3DescriptorSet* cset = static_cast<const Gl3DescriptorSet*>(write.set);
            Gl3DescriptorSet* set = const_cast<Gl3DescriptorSet*>(cset);    // I have a few regrets
            Gl3Descriptor& descriptor = set->descriptors.at(write.binding);

            switch(write.type) {

                case DescriptorType::CombinedImageSampler: {
                    DescriptorImageUpdate* image_update = write.image_info;
                    const Gl3Image* cimage = static_cast<const Gl3Image*>(image_update->image);
                    Gl3Image* image = const_cast<Gl3Image*>(cimage);
                    descriptor.resource = image;
                } break;

                case DescriptorType::UniformBuffer: {
                    NOVA_LOG(WARN) << __FILE__ << "(" << __LINE__ << "): Unimplemented";
                } break;

                case DescriptorType::StorageBuffer: {
                    NOVA_LOG(WARN) << __FILE__ << "(" << __LINE__ << "): Unimplemented";
                } break;

                default:;
            }
        }
    }

    Result<PipelineInterface*> Gl3RenderEngine::create_pipeline_interface(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
        const std::vector<shaderpack::TextureAttachmentInfo>& /* color_attachments */,
        const std::optional<shaderpack::TextureAttachmentInfo>& /* depth_texture */) {
        Gl3PipelineInterface* pipeline_interface = new Gl3PipelineInterface;
        pipeline_interface->bindings = bindings;

        return Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    Result<Pipeline*> Gl3RenderEngine::create_pipeline(PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& data) {
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

        Gl3PipelineInterface* gl3_pipeline_interface = static_cast<Gl3PipelineInterface*>(pipeline_interface);
        for(const auto& binding : gl3_pipeline_interface->bindings) {
            const GLuint uniform_location = glGetUniformLocation(pipeline->id, binding.first.c_str());
            gl3_pipeline_interface->uniform_cache.emplace(binding.first, uniform_location);
        }

        return Result(static_cast<Pipeline*>(pipeline));
    }

    Buffer* Gl3RenderEngine::create_buffer(const BufferCreateInfo& info) {
        Gl3Buffer* buffer = new Gl3Buffer;

        glGenBuffers(1, &buffer->id);

        GLuint buffer_bind_target = 0;

        switch(info.buffer_usage) {
            case BufferUsage::UniformBuffer: {
                buffer_bind_target = GL_UNIFORM_BUFFER;
            } break;

            case BufferUsage::IndexBuffer: {
                buffer_bind_target = GL_ELEMENT_ARRAY_BUFFER;
            } break;

            case BufferUsage::VertexBuffer: {
                buffer_bind_target = GL_ARRAY_BUFFER;
            } break;

            default:;
        }

        glBindBuffer(buffer_bind_target, buffer->id);
        glBufferData(buffer_bind_target, info.size, nullptr, GL_STATIC_DRAW);

        return buffer;
    }

    void Gl3RenderEngine::write_data_to_buffer(const void* data, const uint64_t num_bytes, const uint64_t offset, const Buffer* buffer) {
        const Gl3Buffer* gl_buffer = static_cast<const Gl3Buffer*>(buffer);

        glBindBuffer(GL_COPY_READ_BUFFER, gl_buffer->id);
        glBufferSubData(GL_COPY_READ_BUFFER, offset, num_bytes, data);
    }

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

    Semaphore* Gl3RenderEngine::create_semaphore() { return new Gl3Semaphore; }

    std::vector<Semaphore*> Gl3RenderEngine::create_semaphores(const uint32_t num_semaphores) {
        std::vector<Semaphore*> semaphores(num_semaphores);

        for(uint32_t i = 0; i < num_semaphores; i++) {
            semaphores[i] = new Gl3Semaphore;
        }

        return semaphores;
    }

    Fence* Gl3RenderEngine::create_fence(const bool signaled) {
        Gl3Fence* fence = new Gl3Fence;
        fence->signaled = signaled;

        return fence;
    }

    std::vector<Fence*> Gl3RenderEngine::create_fences(const uint32_t num_fences, const bool signaled) {
        std::vector<Fence*> fences;
        fences.reserve(num_fences);

        for(uint32_t i = 0; i < num_fences; i++) {
            Gl3Fence* fence = new Gl3Fence;
            fence->signaled = signaled;

            fences.emplace_back(fence);
        }

        return fences;
    }

    void Gl3RenderEngine::wait_for_fences(const std::vector<Fence*> fences) {
        for(Fence* fence : fences) {
            Gl3Fence* gl_fence = static_cast<Gl3Fence*>(fence);
            std::unique_lock lck(gl_fence->mutex);
            gl_fence->cv.wait(lck, [&gl_fence] { return gl_fence->signaled; });
        }
    }

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

    CommandList* Gl3RenderEngine::get_command_list(uint32_t /* thread_idx */,
                                                   QueueType /* needed_queue_type */,
                                                   CommandList::Level /* command_list_type */) {
        // TODO: Something useful for custom memory allocation
        return new Gl3CommandList();
    }

    void Gl3RenderEngine::submit_command_list(CommandList* cmds,
                                              QueueType /* queue */,
                                              Fence* fence_to_signal,
                                              const std::vector<Semaphore*>& wait_semaphores,
                                              const std::vector<Semaphore*>& signal_semaphores) {
        // GL is F U N
        // No equivalent for queues. Hope yalls like waiting for things
        // No equivalent for fences or semaphores. Mutexes, anyone?

        for(Semaphore* semaphore : wait_semaphores) {
            Gl3Semaphore* gl_semaphore = static_cast<Gl3Semaphore*>(semaphore);
            std::unique_lock lck(gl_semaphore->mutex);
            gl_semaphore->cv.wait(lck, [&gl_semaphore] { return gl_semaphore->signaled; });
        }

        Gl3CommandList* gl_cmds = static_cast<Gl3CommandList*>(cmds);
        const std::vector<Gl3Command>& commands = gl_cmds->get_commands();
        for(const Gl3Command& command : commands) {
            switch(command.type) {
                case Gl3CommandType::BufferCopy:
                    copy_buffers_impl(command.buffer_copy);
                    break;

                case Gl3CommandType::ExecuteCommandLists:
                    execute_command_lists_impl(command.execute_command_lists);
                    break;

                case Gl3CommandType::BeginRenderpass:
                    begin_renderpass_impl(command.begin_renderpass);
                    break;

                case Gl3CommandType::EndRenderpass:
                    break;

                case Gl3CommandType::BindPipeline:
                    bind_pipeline_impl(command.bind_pipeline);
                    break;

                case Gl3CommandType::BindDescriptorSets:
                    bind_descriptor_sets_impl(command.bind_descriptor_sets);
                    break;

                case Gl3CommandType::BindVertexBuffers:
                    bind_vertex_buffers_impl(command.bind_vertex_buffers);
                    break;

                case Gl3CommandType::BindIndexBuffer:
                    bind_index_buffer_impl(command.bind_index_buffer);
                    break;

                case Gl3CommandType::DrawIndexedMesh:
                    draw_indexed_mesh_impl(command.draw_indexed_mesh);
                    break;
            }
        }

        for(Semaphore* semaphore : signal_semaphores) {
            Gl3Semaphore* gl_semaphore = static_cast<Gl3Semaphore*>(semaphore);
            std::unique_lock lck(gl_semaphore->mutex);
            gl_semaphore->signaled = true;
            gl_semaphore->cv.notify_all();
        }

        Gl3Fence* fence = static_cast<Gl3Fence*>(fence_to_signal);
        std::unique_lock lck(fence->mutex);
        fence->signaled = true;
        fence->cv.notify_all();
    }

    void Gl3RenderEngine::copy_buffers_impl(const Gl3BufferCopyCommand& buffer_copy) {
        glBindBuffer(GL_COPY_READ_BUFFER, buffer_copy.source_buffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, buffer_copy.destination_buffer);

        glCopyBufferSubData(GL_COPY_READ_BUFFER,
                            GL_COPY_WRITE_BUFFER,
                            buffer_copy.source_offset,
                            buffer_copy.destination_offset,
                            buffer_copy.num_bytes);
    }

    void Gl3RenderEngine::begin_renderpass_impl(const Gl3BeginRenderpassCommand& begin_renderpass) {
        glBindFramebuffer(GL_FRAMEBUFFER, begin_renderpass.framebuffer);
    }

    void Gl3RenderEngine::bind_pipeline_impl(const Gl3BindPipelineCommand& bind_pipeline) { glUseProgram(bind_pipeline.program); }

    void Gl3RenderEngine::bind_descriptor_sets_impl(const Gl3BindDescriptorSetsCommand& bind_descriptor_sets) {
        for(uint32_t set_idx = 0; set_idx < bind_descriptor_sets.sets.size(); set_idx++) {
            const Gl3DescriptorSet* set = bind_descriptor_sets.sets.at(set_idx);

            for(uint32_t binding = 0; binding < set->descriptors.size(); binding++) {
                const Gl3Descriptor& descriptor = set->descriptors.at(binding);

                for(const auto& [binding_name, binding_desc] : bind_descriptor_sets.pipeline_bindings) {
                    if(binding_desc.set == set_idx && binding_desc.binding == binding) {
                        switch(binding_desc.type) {
                            case DescriptorType::CombinedImageSampler: {
                                const GLuint uniform_id = bind_descriptor_sets.uniform_cache.at(binding_name);
                                glUniform1i(uniform_id, descriptor.resource->id);
                            } break;

                            case DescriptorType::UniformBuffer: {
                                const GLuint block_index = bind_descriptor_sets.uniform_block_indices.at(binding_name);
                                glBindBufferBase(GL_UNIFORM_BUFFER, block_index, descriptor.resource->id);
                            } break;

                            case DescriptorType::StorageBuffer: {
                                // I don't know how to emulate this in GL3
                            } break;
                        }
                    }
                }
            }
        }
    }

    void Gl3RenderEngine::bind_vertex_buffers_impl(const Gl3BindVertexBuffersCommand& bind_vertex_buffers) {
        // TODO: use std::array or something where this is implicitly true
        assert(bind_vertex_buffers.buffers.size() == 7);

        // All the bindings because there is no god

        // Positions
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(0));
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FullVertex), reinterpret_cast<void*>(offsetof(FullVertex, position)));

        // Normals
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(1));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FullVertex), reinterpret_cast<void*>(offsetof(FullVertex, normal)));

        // Tangents
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(2));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FullVertex), reinterpret_cast<void*>(offsetof(FullVertex, tangent)));

        // Main UVs
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(3));
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(FullVertex), reinterpret_cast<void*>(offsetof(FullVertex, main_uv)));

        // Secondary UVs
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(4));
        glVertexAttribPointer(4, 2, GL_SHORT, GL_FALSE, sizeof(FullVertex), reinterpret_cast<void*>(offsetof(FullVertex, secondary_uv)));

        // Virtual texture ID
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(5));
        glVertexAttribPointer(5,
                              1,
                              GL_INT,
                              GL_FALSE,
                              sizeof(FullVertex),
                              reinterpret_cast<void*>(offsetof(FullVertex, virtual_texture_id)));

        // Additional data
        glBindBuffer(GL_ARRAY_BUFFER, bind_vertex_buffers.buffers.at(6));
        glVertexAttribPointer(6,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(FullVertex),
                              reinterpret_cast<void*>(offsetof(FullVertex, additional_stuff)));
    }

    void Gl3RenderEngine::bind_index_buffer_impl(const Gl3BindIndexBufferCommand& bind_index_buffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bind_index_buffer.buffer);
    }

    void Gl3RenderEngine::draw_indexed_mesh_impl(const Gl3DrawIndexedMeshCommand& draw_indexed_mesh) {
        glDrawArraysInstanced(GL_TRIANGLES, 0, draw_indexed_mesh.num_instances, draw_indexed_mesh.num_indices);
    }

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
