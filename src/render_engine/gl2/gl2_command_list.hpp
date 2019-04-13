/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#ifndef NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
#define NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
#include <nova_renderer/command_list.hpp>

#include "glad.h"

namespace nova::renderer::rhi {
    enum class Gl2CommandType {
        BufferCopy,
        ExecuteCommandLists,
        BeginRenderpass,
        EndRenderpass,
        BindPipeline,
        BindMaterial,
        BindVertexBuffers,
        BindIndexBuffer,
        DrawIndexedMesh,
    };

    struct Gl2BufferCopyCommand {
        GLuint destination_buffer;
        uint64_t destination_offset;

        GLuint source_buffer;
        uint64_t source_offset;

        uint64_t num_bytes;
    };

    struct Gl2ExecuteCommandListsCommand {
        std::vector<CommandList*> lists_to_execute;
    };

    struct Gl2BeginRenderpassCommand {
        GLuint framebuffer;
    };

    struct Gl2BindPipelineCommand {};

    struct Gl2BindMaterialCommand {};

    struct Gl2BindVertexBuffersCommand {};

    struct Gl2BindIndexBufferCommand {};

    struct Gl2DrawIndexedMeshCommand {};

    struct Gl2Command {
        Gl2CommandType type;

        union {
            Gl2BufferCopyCommand buffer_copy;
            Gl2ExecuteCommandListsCommand execute_command_lists;
            Gl2BeginRenderpassCommand begin_renderpass;
            Gl2BindPipelineCommand bind_pipeline;
            Gl2BindMaterialCommand bind_material;
            Gl2BindVertexBuffersCommand bind_vertex_buffers;
            Gl2BindIndexBufferCommand bind_index_buffer;
            Gl2DrawIndexedMeshCommand draw_indexed_mesh;
        };

        ~Gl2Command();
    };

    /*!
     * \brief Command list implementation for OpenGL 2.1
     *
     * This class is fun because OpenGL has no notion of a command list - it's synchronous af. Thus, this command list
     * is a custom thing that records commands into host memory. When this command list is submitted to a "queue", Nova
     * runs the OpenGL commands. This lets command lists be recorded from multiple threads, but submitting a command
     * list to OpenGL is _way_ more expensive than submitting a DirectX or Vulkan command list
     *
     * OpenGL also has a really fun property where it can't be multithreaded, especially since Nova has to use OpenGL
     * 2.1. The OpenGL render engine will chew through OpenGL commands in a separate thread, and in that way mimic the
     * async nature of modern APIs, but still... it'll be rough
     *
     * On the other hand, OpenGL has no concept of a resource barrier...
     */
    class Gl2CommandList final : public CommandList {
    public:
        Gl2CommandList();

		Gl2CommandList(Gl2CommandList&& old) noexcept = default;
		Gl2CommandList& operator=(Gl2CommandList&& old) noexcept = default;

		Gl2CommandList(const Gl2CommandList& other) = delete;
		Gl2CommandList& operator=(const Gl2CommandList& other) = delete;

        void resource_barriers([[maybe_unused]] PipelineStageFlags stages_before_barrier,
                              [[maybe_unused]] PipelineStageFlags stages_after_barrier,
                              [[maybe_unused]] const std::vector<ResourceBarrier>& barriers) override final;

        void copy_buffer(Buffer* destination_buffer,
                         uint64_t destination_offset,
                         Buffer* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override final;

        void execute_command_lists(const std::vector<CommandList*>& lists) override final;

        void begin_renderpass([[maybe_unused]] Renderpass* renderpass, Framebuffer* framebuffer) override final;

        void end_renderpass() override final;

        void bind_pipeline() override final;

        void bind_material() override final;

        void bind_vertex_buffers() override final;

        void bind_index_buffer() override final;

        void draw_indexed_mesh() override final;

        ~Gl2CommandList() override final;

        /*!
         * \brief Provides access to the actual command list, so that the GL2 render engine can process the commands
         */
        std::vector<Gl2Command> get_commands() const;

    private:
        std::vector<Gl2Command> commands;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
