#pragma once

//! Forward declarations for all the types in the RHI

namespace nova::renderer::rhi {
    struct RhiDescriptorSet;
    struct RhiPipeline;
    struct RhiRenderpass;
    struct RhiFramebuffer;
    struct RhiPipelineInterface;
    struct RhiFence;
    struct RhiSemaphore;
    struct RhiResource;
    struct RhiBuffer;
    struct RhiImage;
    struct RhiDeviceMemory;
    struct RhiSampler;
    struct RhiPresentSemaphore;
    struct RhiDescriptorPool;

    class Swapchain;
    class RhiRenderCommandList;
    class RenderDevice;
} // namespace nova::renderer::rhi
