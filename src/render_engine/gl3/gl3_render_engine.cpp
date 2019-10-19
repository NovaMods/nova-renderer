#include <spirv_glsl.hpp>

#include "gl3_render_engine.hpp"

#include "../../../tests/src/general_test_setup.hpp"
#include "../../util/logger.hpp"
#include "gl3_command_list.hpp"
#include "gl3_structs.hpp"

namespace nova::renderer::rhi {
    Gl4NvRenderEngine::Gl4NvRenderEngine(NovaSettingsAccessManager& settings) : RenderEngine(settings) {
        const bool loaded_opengl = true;    // TODO: Load OpenGL from GLFW
        if(!loaded_opengl) {
            NOVA_LOG(FATAL) << "Could not load OpenGL 4.6 functions, sorry bro";
            return;
        }

        open_window_and_create_surface(settings.settings.window);

        set_initial_state();
    }

    void Gl4NvRenderEngine::set_initial_state() {
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

    void Gl4NvRenderEngine::set_num_renderpasses([[maybe_unused]] uint32_t num_renderpasses) {
        // Gl3 doesn't need to do anything either
    }

    ntl::Result<DeviceMemory*> Gl4NvRenderEngine::allocate_device_memory(const uint64_t /* size */,
                                                                  const MemoryUsage /* type */,
                                                                  const ObjectType /* allowed_objects */) {
        return ntl::Result(new DeviceMemory);
    }

    ntl::Result<Renderpass*> Gl4NvRenderEngine::create_renderpass(const shaderpack::RenderPassCreateInfo& /* data */) {
        return ntl::Result<Renderpass*>(new Renderpass);
    }

    Framebuffer* Gl4NvRenderEngine::create_framebuffer(const Renderpass* /* renderpass */,
                                                     const std::vector<Image*>& attachments,
                                                     const glm::uvec2& /* framebuffer_size */) {
        auto* framebuffer = new Gl3Framebuffer;

        glGenFramebuffers(1, &framebuffer->id);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);

        uint32_t attachment_idx = 0;
        for(const Image* attachment : attachments) {
            const auto* gl_attachment = static_cast<const Gl3Image*>(attachment);

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

    DescriptorPool* Gl4NvRenderEngine::create_descriptor_pool(const uint32_t num_sampled_images,
                                                            const uint32_t num_samplers,
                                                            const uint32_t num_uniform_buffers) {
        auto* pool = new Gl3DescriptorPool;
        pool->descriptors.resize(static_cast<std::size_t>(num_sampled_images + num_uniform_buffers));
        pool->sampler_sets.resize(num_samplers);

        return pool;
    }

    std::vector<DescriptorSet*> Gl4NvRenderEngine::create_descriptor_sets(const PipelineInterface* pipeline_interface, DescriptorPool* pool) {
        auto* gl_descriptor_pool = static_cast<Gl3DescriptorPool*>(pool);
        std::vector<DescriptorSet*> sets;

        for(const auto& [name, desc] : pipeline_interface->bindings) {
            void* new_set_mem = gl_descriptor_pool->descriptor_allocator.allocate(sizeof(Gl3DescriptorSet));
            auto* new_set = new(new_set_mem) Gl3DescriptorSet;
            sets.push_back(new_set);

            new_set->descriptors.resize(desc.count);
        }

        return sets;
    }

    void Gl4NvRenderEngine::update_descriptor_sets(std::vector<DescriptorSetWrite>& writes) {
        for(DescriptorSetWrite& write : writes) {
            const auto* cset = static_cast<const Gl3DescriptorSet*>(write.set);
            auto* set = const_cast<Gl3DescriptorSet*>(cset); // I have a few regrets
            Gl3Descriptor& descriptor = set->descriptors.at(write.binding);

            switch(write.type) {

                case DescriptorType::CombinedImageSampler: {
                    DescriptorImageUpdate* image_update = write.image_info;
                    const auto* cimage = static_cast<const Gl3Image*>(image_update->image);
                    auto* image = const_cast<Gl3Image*>(cimage);
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

    ntl::Result<PipelineInterface*> Gl4NvRenderEngine::create_pipeline_interface(
        const std::unordered_map<std::string, ResourceBindingDescription>& bindings,
        const std::vector<shaderpack::TextureAttachmentInfo>& /* color_attachments */,
        const std::optional<shaderpack::TextureAttachmentInfo>& /* depth_texture */) {
        auto* pipeline_interface = new Gl3PipelineInterface;
        pipeline_interface->bindings = bindings;

        return ntl::Result(static_cast<PipelineInterface*>(pipeline_interface));
    }

    ntl::Result<Pipeline*> Gl4NvRenderEngine::create_pipeline(PipelineInterface* pipeline_interface,
                                                            const shaderpack::PipelineCreateInfo& data) {
        auto* pipeline = new Gl3Pipeline;

        pipeline->id = glCreateProgram();

        std::vector<std::string> pipeline_creation_errors;
        pipeline_creation_errors.reserve(4);

        const ntl::Result<GLuint>& vertex_shader = compile_shader(data.vertex_shader.source, GL_VERTEX_SHADER);
        if(vertex_shader) {
            glAttachShader(pipeline->id, vertex_shader.value);

        } else {
            pipeline_creation_errors.push_back(vertex_shader.error.to_string());
        }

        if(supports_geometry_shaders && data.geometry_shader) {
            // TODO: convert to the extension when we care about Mac support
            const ntl::Result<GLuint>& geometry_shader = compile_shader(data.geometry_shader->source, GL_GEOMETRY_SHADER);   
            if(geometry_shader) {
                glAttachShader(pipeline->id, geometry_shader.value);

            } else {
                pipeline_creation_errors.push_back(geometry_shader.error.to_string());
            }
        }

        if(data.fragment_shader) {
            const ntl::Result<GLuint>& fragment_shader = compile_shader(data.fragment_shader->source, GL_FRAGMENT_SHADER);
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
            std::string program_link_log_std;
            program_link_log_std.reserve(link_log_length);
            glGetProgramInfoLog(pipeline->id, link_log_length, nullptr, program_link_log_std.data());

            const std::string program_link_log = program_link_log_std;

            pipeline_creation_errors.push_back(program_link_log);

            return ntl::Result<Pipeline*>(ntl::NovaError(program_link_log));
        }
        auto* gl3_pipeline_interface = static_cast<Gl3PipelineInterface*>(pipeline_interface);
        for(const auto& binding : gl3_pipeline_interface->bindings) {
            const GLuint uniform_location = glGetUniformLocation(pipeline->id, binding.first.c_str());
            gl3_pipeline_interface->uniform_cache.emplace(binding.first, uniform_location);
        }

        return ntl::Result(static_cast<Pipeline*>(pipeline));
    }

    Buffer* Gl4NvRenderEngine::create_buffer(const BufferCreateInfo& info) {
        auto* buffer = new Gl3Buffer;

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

    void Gl4NvRenderEngine::write_data_to_buffer(const void* data, const uint64_t num_bytes, const uint64_t offset, const Buffer* buffer) {
        const auto* gl_buffer = static_cast<const Gl3Buffer*>(buffer);

        glBindBuffer(GL_COPY_READ_BUFFER, gl_buffer->id);
        glBufferSubData(GL_COPY_READ_BUFFER, offset, num_bytes, data);
    }

    Image* Gl4NvRenderEngine::create_texture(const shaderpack::TextureCreateInfo& info) {
        auto* image = new Gl3Image;

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

    Semaphore* Gl4NvRenderEngine::create_semaphore() { return new Gl3Semaphore; }

    std::vector<Semaphore*> Gl4NvRenderEngine::create_semaphores(const uint32_t num_semaphores) {
        std::vector<Semaphore*> semaphores(num_semaphores);

        for(uint32_t i = 0; i < num_semaphores; i++) {
            semaphores[i] = new Gl3Semaphore;
        }

        return semaphores;
    }

    Fence* Gl4NvRenderEngine::create_fence(const bool signaled) {
        auto* fence = new Gl3Fence;
        fence->signaled = signaled;

        return fence;
    }

    std::vector<Fence*> Gl4NvRenderEngine::create_fences(const uint32_t num_fences, const bool signaled) {
        std::vector<Fence*> fences;
        fences.reserve(num_fences);

        for(uint32_t i = 0; i < num_fences; i++) {
            auto* fence = new Gl3Fence;
            fence->signaled = signaled;

            fences.emplace_back(fence);
        }

        return fences;
    }

    void Gl4NvRenderEngine::wait_for_fences(const std::vector<Fence*> fences) {
        for(Fence* fence : fences) {
            auto* gl_fence = static_cast<Gl3Fence*>(fence);
            std::unique_lock lck(gl_fence->mutex);
            gl_fence->cv.wait(lck, [&gl_fence] { return gl_fence->signaled; });
        }
    }

    void Gl4NvRenderEngine::reset_fences(const std::vector<Fence*>& fences) {
        for(Fence* fence : fences) {
            auto* gl_fence = static_cast<Gl3Fence*>(fence);
            std::unique_lock guard(gl_fence->mutex);
            gl_fence->signaled = false;
        }
    }

    void Gl4NvRenderEngine::destroy_renderpass(Renderpass* pass) { delete pass; }

    void Gl4NvRenderEngine::destroy_framebuffer(Framebuffer* framebuffer) { delete framebuffer; }

    void Gl4NvRenderEngine::destroy_pipeline_interface(PipelineInterface* /* pipeline_interface */) {
        // No work needed, no GPU objects in Gl3PipelineInterface;
    }

    void Gl4NvRenderEngine::destroy_pipeline(Pipeline* pipeline) {}

    void Gl4NvRenderEngine::destroy_texture(Image* resource) {
        auto* gl_image = static_cast<Gl3Image*>(resource);
        glDeleteTextures(1, &gl_image->id);

        delete gl_image;
    }

    void Gl4NvRenderEngine::destroy_semaphores(
        std::vector<Semaphore*>& /* semaphores */) { // OpenGL semaphores have no GPU objects, so we don't need to do anything here
    }

    void Gl4NvRenderEngine::destroy_fences(
        std::vector<Fence*>& /* fences */) { // OpenGL fences have no GPU objects, so we don't need to do anything here
    }

    CommandList* Gl4NvRenderEngine::get_command_list(uint32_t /* thread_idx */,
                                                   QueueType /* needed_queue_type */,
                                                   CommandList::Level /* command_list_type */) {
        // TODO: Something useful for custom memory allocation
        return new Gl3CommandList();
    }

    void Gl4NvRenderEngine::submit_command_list(CommandList* cmds,
                                              QueueType /* queue */,
                                              Fence* fence_to_signal,
                                              const std::vector<Semaphore*>& wait_semaphores,
                                              const std::vector<Semaphore*>& signal_semaphores) {
        // GL is F U N
        // No equivalent for queues. Hope yalls like waiting for things
        // No equivalent for fences or semaphores. Mutexes, anyone?

        for(Semaphore* semaphore : wait_semaphores) {
            auto* gl_semaphore = static_cast<Gl3Semaphore*>(semaphore);
            std::unique_lock lck(gl_semaphore->mutex);
            gl_semaphore->cv.wait(lck, [&gl_semaphore] { return gl_semaphore->signaled; });
        }
        auto* gl_cmds = static_cast<Gl3CommandList*>(cmds);
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

                case Gl3CommandType::None:
                    NOVA_LOG(FATAL) << "Unimplemented: tried to submit none command buffer";
                    break;
            }
        }

        for(Semaphore* semaphore : signal_semaphores) {
            auto* gl_semaphore = static_cast<Gl3Semaphore*>(semaphore);
            std::unique_lock lck(gl_semaphore->mutex);
            gl_semaphore->signaled = true;
            gl_semaphore->cv.notify_all();
        }
        auto* fence = static_cast<Gl3Fence*>(fence_to_signal);
        std::unique_lock lck(fence->mutex);
        fence->signaled = true;
        fence->cv.notify_all();
    }

    void Gl4NvRenderEngine::open_window_and_create_surface(const NovaSettings::WindowOptions& options) {
        // Create GLFW window
        // Be sure to create an OpenGL 4.6 forward-compatible context!
    }

    void Gl4NvRenderEngine::copy_buffers_impl(const Gl3BufferCopyCommand& buffer_copy) {
        glBindBuffer(GL_COPY_READ_BUFFER, buffer_copy.source_buffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, buffer_copy.destination_buffer);

        glCopyBufferSubData(GL_COPY_READ_BUFFER,
                            GL_COPY_WRITE_BUFFER,
                            buffer_copy.source_offset,
                            buffer_copy.destination_offset,
                            buffer_copy.num_bytes);
    }

    void Gl4NvRenderEngine::begin_renderpass_impl(const Gl3BeginRenderpassCommand& begin_renderpass) {
        glBindFramebuffer(GL_FRAMEBUFFER, begin_renderpass.framebuffer);
    }

    void Gl4NvRenderEngine::bind_pipeline_impl(const Gl3BindPipelineCommand& bind_pipeline) { glUseProgram(bind_pipeline.program); }

    void Gl4NvRenderEngine::bind_descriptor_sets_impl(const Gl3BindDescriptorSetsCommand& bind_descriptor_sets) {
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

    void Gl4NvRenderEngine::bind_vertex_buffers_impl(const Gl3BindVertexBuffersCommand& bind_vertex_buffers) {
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

    void Gl4NvRenderEngine::bind_index_buffer_impl(const Gl3BindIndexBufferCommand& bind_index_buffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bind_index_buffer.buffer);
    }

    void Gl4NvRenderEngine::draw_indexed_mesh_impl(const Gl3DrawIndexedMeshCommand& draw_indexed_mesh) {
        glDrawArraysInstanced(GL_TRIANGLES, 0, draw_indexed_mesh.num_instances, draw_indexed_mesh.num_indices);
    }

    void Gl4NvRenderEngine::execute_command_lists_impl(const Gl3ExecuteCommandListsCommand& execute_command_lists) {
        
    }

    ntl::Result<GLuint> compile_shader(const std::vector<uint32_t>& spirv, const GLenum shader_type) {
        spirv_cross::CompilerGLSL compiler(spirv);
        const std::string glsl = compiler.compile().c_str();
        const char* glsl_c = glsl.c_str();
        const auto len = static_cast<GLint>(glsl.size()); // SIGNED LENGTH WHOOOOOO

        const GLuint shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &glsl_c, &len);
        glCompileShader(shader);

        GLint compile_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &compile_log_length);
        if(compile_log_length > 0) {
            std::string compile_log;
            compile_log.reserve(compile_log_length);
            glGetShaderInfoLog(shader, compile_log_length, nullptr, compile_log.data());
            return ntl::Result<GLuint>(ntl::NovaError(compile_log.c_str()));
        }

        return ntl::Result(shader);
    }
} // namespace nova::renderer::rhi
