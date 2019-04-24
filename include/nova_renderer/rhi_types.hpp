#pragma once

#include <glm/glm.hpp>

namespace nova::renderer::rhi {
#pragma region Enums
    enum class DescriptorType { CombinedImageSampler, UniformBuffer, StorageBuffer };

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

    enum ShaderStageFlags {
        Vertex = 0x0001,
        TessellationControl = 0x0002,
        TessellationEvaluation = 0x0004,
        Geometry = 0x0008,
        Fragment = 0x0010,
        Compute = 0x0020,
        Raygen = 0x0100,
        AnyHit = 0x0200,
        ClosestHit = 0x0400,
        Miss = 0x0800,
        Intersection = 0x1000,
        Task = 0x0040,
        Mesh = 0x0080,
    };
#pragma endregion

#pragma region Structs
    enum class QueueType {
        Graphics,
        Transfer,
        AsyncCompute,
    };

    /*!
     * \brief A resource
     *
     * Resources may by dynamic of static. Dynamic resources are updated after they are created, possibly by a shader,
     * while static resources are loaded once and that's that
     */
    struct Resource {
        enum Type {
            Buffer,
            Image,
        };

        Type type;
        bool is_dynamic = false;
    };

    struct Image : Resource {
        bool is_depth_tex = false;
    };

    struct Buffer : Resource {};

    struct Framebuffer {
        glm::uvec2 size;
    };

    struct Renderpass {
        bool writes_to_backbuffer = false;
    };

    /*!
     * \brief The interface for a pipeline. Includes both inputs (descriptors) and outputs (framebuffers)
     */
    struct PipelineInterface {};

    struct Pipeline {};

    struct Semaphore {};

    struct Fence {};

    struct Descriptor {};

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

    struct ResourceBindingDescription {
        /*!
         * \brief Descriptor set that his binding belongs to
         */
        uint32_t set;

        /*!
         * \brief Binding of this resource binding
         */
        uint32_t binding;

        /*!
         * \brief Number of bindings. Useful if you have an array of descriptors
         */
        uint32_t count;

        /*!
         * \brief The type of object that will be bound
         */
        DescriptorType type;

        /*!
         * \brief The shader stages that need access to this binding
         */
        ShaderStageFlags stages;

        bool operator==(const ResourceBindingDescription& other);

        bool operator!=(const ResourceBindingDescription& other);
    };
#pragma endregion

    ShaderStageFlags operator|=(const ShaderStageFlags lhs, const ShaderStageFlags rhs);
} // namespace nova::renderer::rhi
