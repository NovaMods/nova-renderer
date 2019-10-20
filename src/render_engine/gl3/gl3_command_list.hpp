/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#ifndef NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
#define NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
#include "nova_renderer/command_list.hpp"

#include "gl3_structs.hpp"
#include "glad/glad.h"

namespace nova::renderer::rhi {
    enum class Gl3CommandType {
        None,
        BufferCopy,
        ExecuteCommandLists,
        BeginRenderpass,
        EndRenderpass,
        BindPipeline,
        BindDescriptorSets,
        BindVertexBuffers,
        BindIndexBuffer,
        DrawIndexedMesh,
    };

    struct Gl3BufferCopyCommand {
        GLuint destination_buffer;
        uint64_t destination_offset;

        GLuint source_buffer;
        uint64_t source_offset;

        uint64_t num_bytes;
    };

    struct Gl3ExecuteCommandListsCommand {
        std::vector<CommandList*> lists_to_execute;
    };

    struct Gl3BeginRenderpassCommand {
        GLuint framebuffer;
    };

    struct Gl3BindPipelineCommand {
        GLuint program;
    };

    struct Gl3BindDescriptorSetsCommand {
        GLuint program;
        std::vector<Gl3DescriptorSet*> sets;
        std::unordered_map<std::string, GLuint> uniform_cache;
        std::unordered_map<std::string, ResourceBindingDescription> pipeline_bindings;
        std::unordered_map<std::string, GLuint> uniform_block_indices;
    };

    struct Gl3BindVertexBuffersCommand {
        std::vector<GLuint> buffers;
    };

    struct Gl3BindIndexBufferCommand {
        GLuint buffer;
    };

    struct Gl3DrawIndexedMeshCommand {
        uint32_t num_indices;
        uint32_t num_instances;
    };

    struct Gl3Command {
        Gl3CommandType type = Gl3CommandType::None;

        union {
            Gl3BufferCopyCommand buffer_copy;
            Gl3ExecuteCommandListsCommand execute_command_lists;
            Gl3BeginRenderpassCommand begin_renderpass;
            Gl3BindPipelineCommand bind_pipeline;
            Gl3BindDescriptorSetsCommand bind_descriptor_sets;
            Gl3BindVertexBuffersCommand bind_vertex_buffers;
            Gl3BindIndexBufferCommand bind_index_buffer;
            Gl3DrawIndexedMeshCommand draw_indexed_mesh;
        };

        Gl3Command();

        Gl3Command(Gl3Command&& old) noexcept;
        Gl3Command& operator=(Gl3Command&& old) noexcept;

        Gl3Command(const Gl3Command& other);
        Gl3Command& operator=(const Gl3Command& other);

        ~Gl3Command();
    };

    /*!
     * \brief Command list implementation for OpenGL 3.1
     *
     * This class is fun because OpenGL has no notion of a command list - it's synchronous af. Thus, this command list
     * is a custom thing that records commands into host memory. When this command list is submitted to a "queue", Nova
     * runs the OpenGL commands. This lets command lists be recorded from multiple threads, but submitting a command
     * list to OpenGL is _way_ more expensive than submitting a DirectX or Vulkan command list
     *
     * OpenGL also has a really fun property where it can't be multithreaded, especially since Nova has to use OpenGL
     * 3.1. The OpenGL render engine will chew through OpenGL commands in a separate thread, and in that way mimic the
     * async nature of modern APIs, but still... it'll be rough
     *
     * On the other hand, OpenGL has no concept of a resource barrier...
     */
    class [[nodiscard]] Gl3CommandList final : public CommandList {
    public:
        Gl3CommandList();

        Gl3CommandList(Gl3CommandList&& old) noexcept = default;
        Gl3CommandList& operator=(Gl3CommandList&& old) noexcept = default;

        Gl3CommandList(const Gl3CommandList& other) = delete;
        Gl3CommandList& operator=(const Gl3CommandList& other) = delete;

        ~Gl3CommandList() override = default;

        void resource_barriers(PipelineStageFlags stages_before_barrier,
                               PipelineStageFlags stages_after_barrier,
                               const std::vector<ResourceBarrier>& barriers) override;

        void copy_buffer(Buffer* destination_buffer,
                         uint64_t destination_offset,
                         Buffer* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override;

        void execute_command_lists(const std::vector<CommandList*>& lists) override;

        void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) override;

        void end_renderpass() override;

        void bind_pipeline(const Pipeline* pipeline) override;

        void bind_descriptor_sets(const std::vector<DescriptorSet*>& descriptor_sets, const PipelineInterface* pipeline_interface) override;

        void bind_vertex_buffers(const std::vector<Buffer*>& buffers) override;

        void bind_index_buffer(const Buffer* buffer) override;

        void draw_indexed_mesh(uint32_t num_indices, uint32_t num_instances) override;

        /*!
         * \brief Provides access to the actual command list, so that the GL3 render engine can process the commands
         */
        [[nodiscard]] std::vector<Gl3Command> get_commands() const;

    private:
        std::vector<Gl3Command> commands;
    };
} // namespace nova::renderer::rhi

#endif // NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
