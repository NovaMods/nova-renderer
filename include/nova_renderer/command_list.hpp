/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#pragma once
#include <vector>

#include "nova_renderer/rhi_types.hpp"

namespace nova::renderer::rhi {
#pragma region Enums
    enum class ResourceState {
        Undefined,
        General,

        ColorAttachment,
        DepthStencilAttachment,
        DepthReadOnlyStencilAttachment,
        DepthAttachmentStencilReadOnly,
        DepthStencilReadOnlyAttachment,

        PresentSource,

        NonFragmentShaderReadOnly,
        FragmentShaderReadOnly,

        TransferSource,
        TransferDestination,
    };

    enum ResourceAccessFlags {
        IndexReadBit = 0x00000002,
        VertexAttributeReadBit = 0x00000004,
        UniformReadBit = 0x00000008,
        InputAttachmentReadBit = 0x00000010,
        ShaderReadBit = 0x00000020,
        ShaderWriteBit = 0x00000040,
        ColorAttachmentReadBit = 0x00000080,
        ColorAttachmentWriteBit = 0x00000100,
        DepthStencilAttachmentReadBit = 0x00000200,
        DepthStencilAttachmentWriteBit = 0x00000400,
        TransferReadBit = 0x00000800,
        TransferWriteBit = 0x00001000,
        HostReadBit = 0x00002000,
        HostWriteBit = 0x00004000,
        MemoryReadBit = 0x00008000,
        MemoryWriteBit = 0x00010000,
    };

    enum ImageAspectFlags {
        Color = 0x00000001,
        Depth = 0x00000002,
        Stencil = 0x00000004,
    };

    enum PipelineStageFlags {
        TopOfPipe = 0x00000001,
        DrawIndirect = 0x00000002,
        VertexInput = 0x00000004,
        VertexShader = 0x00000008,
        TessellationControlShader = 0x00000010,
        TessellationEvaluationShader = 0x00000020,
        GeometryShader = 0x00000040,
        FragmentShader = 0x00000080,
        EarlyFragmentTests = 0x00000100,
        LateFragmentTests = 0x00000200,
        ColorAttachmentOutput = 0x00000400,
        ComputeShader = 0x00000800,
        Transfer = 0x00001000,
        BottomOfPipe = 0x00002000,
        Host = 0x00004000,
        AllGraphics = 0x00008000,
        AllCommands = 0x00010000,
        ShadingRateImage = 0x00400000,
        RayTracingShader = 0x00200000,
        AccelerationStructureBuild = 0x02000000,
        TaskShader = 0x00080000,
        MeshShader = 0x00100000,
        FragmentDensityProcess = 0x00800000,
    };
#pragma endregion

#pragma region Structs
    struct ResourceBarrier {
        Resource* resource_to_barrier;

        ResourceState initial_state;
        ResourceState final_state;

        ResourceAccessFlags access_before_barrier;
        ResourceAccessFlags access_after_barrier;

        QueueType source_queue;
        QueueType destination_queue;

        union {
            struct {
                ImageAspectFlags aspect;
            } image_memory_barrier;

            struct {
                uint64_t offset;
                uint64_t size;
            } buffer_memory_barrier;
        };
    };
#pragma endregion

#pragma region Command list API
    /*!
     * \brief An API-agnostic command list
     *
     * Command lists are allocated from the render engine. Once allocated, ownership is passed to the callee. You can
     * then record whatever commands you want and submit the command list back to the render engine for execution on
     * the GPU. Once submitted, you may not record any more commands into the command list
     *
     * There is one command list pool per swapchain image per thread. All the pools for one swapchain image are
     * reset at the beginning of a frame that renders to that swapchain image. This means that any command list
     * allocated in one frame will not be valid in the next frame. DO NOT hold on to command lists
     *
     * A command list may only be recorded to from one thread at a time
     *
     * Command lists are fully bound to ChaiScript
     */
    class CommandList {
    public:
        enum class Level {
            Primary,
            Secondary,
        };

        CommandList() = default;

        CommandList(CommandList&& old) noexcept = default;
        CommandList& operator=(CommandList&& old) noexcept = default;

        CommandList(const CommandList& other) = delete;
        CommandList& operator=(const CommandList& other) = delete;

        /*!
         * \brief Inserts a barrier so that all access to a resource before the barrier is resolved before any access
         * to the resource after the barrier
         *
         * \param stages_before_barrier The pipeline stages that should be completed before the barriers take effect
         * \param stages_after_barrier The pipeline stages that must wait for the barrier
         * \param barriers All the resource barriers to use
         */
        virtual void resource_barriers(PipelineStageFlags stages_before_barrier,
                                       PipelineStageFlags stages_after_barrier,
                                       const std::vector<ResourceBarrier>& barriers) = 0;

        /*!
         * \brief Records a command to copy one region of a buffer to another buffer
         *
         * \param destination_buffer The buffer to write data to
         * \param destination_offset The offset in the destination buffer to start writing to. Measured in bytes
         * \param source_buffer The buffer to read data from
         * \param source_offset The offset in the source buffer to start reading from. Measures in bytes
         * \param num_bytes The number of bytes to copy
         *
         * \pre destination_buffer != nullptr
         * \pre destination_buffer is a buffer
         * \pre destination_offset is less than the size of destination_buffer
         * \pre source_buffer != nullptr
         * \pre source_buffer is a buffer
         * \pre source_offset is less than the size of source_buffer
         * \pre destination_offset plus num_bytes is less than the size of destination_buffer
         * \pre destination_offset plus num_bytes is less than the size of source_buffer
         */
        virtual void copy_buffer(
            Buffer* destination_buffer, uint64_t destination_offset, Buffer* source_buffer, uint64_t source_offset, uint64_t num_bytes) = 0;

        /*!
         * \brief Executed a number of command lists
         *
         * These command lists should be secondary command lists. Nova doesn't validate this because yolo but you need
         * to be nice - the API-specific validation layers _will_ yell at you
         */
        virtual void execute_command_lists(const std::vector<CommandList*>& lists) = 0;

        virtual void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) = 0;
        virtual void end_renderpass() = 0;
        virtual void bind_pipeline() = 0;
        virtual void bind_material() = 0;

        virtual void bind_vertex_buffers() = 0;
        virtual void bind_index_buffer() = 0;
        virtual void draw_indexed_mesh() = 0;

        virtual ~CommandList() = default;
    };
#pragma endregion
} // namespace nova::renderer::rhi
