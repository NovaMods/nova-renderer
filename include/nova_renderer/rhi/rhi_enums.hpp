#pragma once

#include "nova_renderer/util/utils.hpp"

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

    enum class PixelFormat {
        Rgba8,
        Rgba16F,
        Rgba32F,
        Depth32,
        Depth24Stencil8,
    };

    enum class TextureUsage {
        RenderTarget,
        SampledRenderTarget,
        SampledTexture,
    };

    enum class DescriptorType { CombinedImageSampler, UniformBuffer, StorageBuffer, Texture, Sampler };

    enum class ResourceAccess {
        IndirectCommandRead,

        IndexRead,
        VertexAttributeRead,
        UniformRead,
        InputAttachmentRead,
        ShaderRead,
        ShaderWrite,
        ColorAttachmentRead,
        ColorAttachmentWrite,
        DepthStencilAttachmentRead,
        DepthStencilAttachmentWrite,

        CopyRead,
        CopyWrite,
        HostRead,
        HostWrite,
        MemoryRead,
        MemoryWrite,

        ShadingRateImageRead,

        AccelerationStructureRead,
        AccelerationStructureWrite,

        FragmentDensityMapRead,
    };

    enum class ResourceState {
        Undefined,

        Common,

        CopySource,
        CopyDestination,

        UniformBuffer,
        VertexBuffer,
        IndexBuffer,

        ShaderRead,
        ShaderWrite,

        RenderTarget,
        DepthWrite,
        DepthRead,

        PresentSource,
    };

    enum class ImageAspect {
        Color = 0x00000001,
        Depth = 0x00000002,
        Stencil = 0x00000004,
    };

    enum class PipelineStage {
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

    enum class ShaderStage {
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

    bool is_depth_format(PixelFormat format);
} // namespace nova::renderer::rhi
