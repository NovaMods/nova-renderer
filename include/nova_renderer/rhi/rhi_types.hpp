#pragma once

#include <glm/glm.hpp>

#include "nova_renderer/memory/bytes.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/renderpack_data.hpp"

namespace nova::renderer::rhi {

#pragma region Structs
    struct RhiBufferCreateInfo {
        rx::string name;

        mem::Bytes size = 0;

        BufferUsage buffer_usage{};
    };

    struct RhiDeviceMemory {};

    /*!
     * \brief A resource
     *
     * Resources may by dynamic of static. Dynamic resources are updated after they are created, possibly by a shader,
     * while static resources are loaded once and that's that
     */
    struct RhiResource {
        ResourceType type = {};
        bool is_dynamic = true;
    };

    struct RhiSamplerCreateInfo {
        TextureFilter min_filter = TextureFilter::Point;
        TextureFilter mag_filter = TextureFilter::Point;

        TextureCoordWrapMode x_wrap_mode = TextureCoordWrapMode::ClampToEdge;
        TextureCoordWrapMode y_wrap_mode = TextureCoordWrapMode::ClampToEdge;
        TextureCoordWrapMode z_wrap_mode = TextureCoordWrapMode::ClampToEdge;

        float mip_bias = 0;

        bool enable_anisotropy = false;
        float max_anisotropy = 1;

        float min_lod = 0;
        float max_lod = 0;
    };

    struct RhiSampler {};

    struct RhiTextureCreateInfo {
        TextureUsage usage;
    };

    struct RhiImage : RhiResource {
        bool is_depth_tex = false;
    };

    struct RhiBuffer : RhiResource {
        mem::Bytes size = 0;
    };

    struct RhiFramebuffer {
        glm::uvec2 size;

        uint32_t num_attachments;
    };

    struct RhiRenderpass {};

    struct RhiResourceBindingDescription {
        /*!
         * \brief Descriptor set that this binding belongs to
         */
        uint32_t set;

        /*!
         * \brief Binding of this resource binding
         */
        uint32_t binding;

        /*!
         * \brief Number of bindings. Useful if you have an array of descriptors
         *
         * If this is a unbounded array, this count is the upper limit on the size of the array
         */
        uint32_t count;

        /*!
         * \brief If true, this binding is an unbounded array
         *
         * Unbounded descriptors must be the final binding in their descriptor set
         */
        bool is_unbounded;

        /*!
         * \brief The type of object that will be bound
         */
        DescriptorType type;

        /*!
         * \brief The shader stages that need access to this binding
         */
        ShaderStage stages;

        bool operator==(const RhiResourceBindingDescription& other);

        bool operator!=(const RhiResourceBindingDescription& other);
    };

    struct RhiVertexField {
        rx::string name;

        VertexFieldFormat format;
    };

    /*!
     * \brief The interface for a pipeline. Includes both inputs (descriptors) and outputs (framebuffers)
     */
    struct RhiPipelineInterface {
        rx::map<rx::string, RhiResourceBindingDescription> bindings;

        [[nodiscard]] uint32_t get_num_descriptors_of_type(DescriptorType type) const;
    };

    struct RhiPipeline {};

    struct RhiSemaphore {};

    struct RhiPresentSemaphore {};

    struct RhiFence {};

    struct RhiDescriptorPool {};

    struct RhiDescriptorSet {};

    // TODO: Resource state tracking in the command list so we don't need all this bullshit
    struct RhiResourceBarrier {
        RhiResource* resource_to_barrier;

        /*!
         * \brief The resource access that much finish before this barrier executed
         */
        ResourceAccess access_before_barrier;

        /*!
         * \brief The resource access that must wait for this battier to finish executing
         */
        ResourceAccess access_after_barrier;

        /*!
         * \brief How you're going to access this resource just before this barrier
         *
         * Will a shader read from it before the barrier? Will the fragment depth by copied to a depth buffer before
         * this barrier? Will the resource be used as a indirect draw command buffer right before this barrier?
         */
        ResourceState old_state;

        /*!
         * \brief How you're going to access this resource after this barrier
         *
         * Will a shader read from it after the barrier? Will the fragment depth by copied to a depth buffer after
         * this barrier? Will the resource be used as a indirect draw command buffer right after this barrier?
         */
        ResourceState new_state;

        QueueType source_queue;
        QueueType destination_queue;

        struct ImageMemoryBarrier {
            ImageAspect aspect;
        };

        struct BufferMemoryBarrier {
            mem::Bytes offset;
            mem::Bytes size;
        };

        union {
            ImageMemoryBarrier image_memory_barrier;

            BufferMemoryBarrier buffer_memory_barrier;
        };

        RhiResourceBarrier();
    };

    struct RhiDescriptorImageInfo {
        RhiImage* image;
        renderpack::TextureFormat format;
    };

    struct RhiDescriptorBufferInfo {
        RhiBuffer* buffer;
    };

    struct RhiDescriptorSamplerInfo {
        RhiSampler* sampler;
    };

    union RhiDescriptorResourceInfo {
        RhiDescriptorResourceInfo();

        /*!
         * \brief Information to update an image descriptor
         */
        RhiDescriptorImageInfo image_info;

        /*!
         * \brief Information to update a buffer descriptor
         */
        RhiDescriptorBufferInfo buffer_info{};

        RhiDescriptorSamplerInfo sampler_info;
    };

    struct RhiDescriptorSetWrite {
        /*!
         * \brief Pointer to the descriptor set to write to
         */
        RhiDescriptorSet* set;

        /*!
         * \brief The specific binding in the set that you want to write to
         */
        uint32_t binding;

        /*!
         * \brief The type of descriptor you're writing to
         */
        DescriptorType type;

        /*!
         * \brief All the resources to bind to this descriptor
         *
         * You may only bind multiple resources if the descriptor is an array descriptor. Knowing whether you're binding to an array
         * descriptor or not is your responsibility
         */
        rx::vector<RhiDescriptorResourceInfo> resources;
    };
#pragma endregion

    ShaderStage operator|=(ShaderStage lhs, ShaderStage rhs);
} // namespace nova::renderer::rhi
