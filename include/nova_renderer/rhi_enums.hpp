#pragma once

namespace nova::renderer::rhi {
    enum class MemoryUsage {
        DeviceOnly,
        LowFrequencyUpload,
        StagingBuffer,
    };

    enum class ObjectType {
        Buffer,
        Texture,
        RenderTexture,
        SwapchainSurface,
        Any,
    };

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

    enum class ResourceAccessFlags {
        NoFlags = 0x00000000,
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

    enum class ImageAspectFlags {
        Color = 0x00000001,
        Depth = 0x00000002,
        Stencil = 0x00000004,
    };

    enum class PipelineStageFlags {
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

    enum class ShaderStageFlags {
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

    enum class QueueType {
        Graphics,
        Transfer,
        AsyncCompute,
    };
    enum class BufferUsage {
        UniformBuffer,
        IndexBuffer,
        VertexBuffer,
        StagingBuffer,
    };

    enum class ResourceType {
        Buffer,
        Image,
    };
} // namespace nova::renderer::rhi
