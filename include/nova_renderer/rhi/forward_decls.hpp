#pragma once

//! Forward declarations for all the types in the RHI

namespace nova::renderer::rhi {
    struct DescriptorSet;
    struct Pipeline;
    struct Renderpass;
    struct Framebuffer;
    struct PipelineInterface;
    struct Fence;
    struct Semaphore;
    struct Resource;
    struct Buffer;
    struct Image;
    struct DeviceMemory;
    struct Sampler;
    struct PresentSemaphore;
    struct DescriptorPool;

    class Swapchain;
    class CommandList;
    class RenderDevice;
} // namespace nova::renderer::rhi
