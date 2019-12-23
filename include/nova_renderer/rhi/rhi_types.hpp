#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/shaderpack_data.hpp"
#include "nova_renderer/memory/bytes.hpp"

namespace nova::renderer::rhi {

#pragma region Structs
    struct BufferCreateInfo {
        memory::Bytes size = 0;

        BufferUsage buffer_usage{};
    };

    struct DeviceMemory {};

    /*!
     * \brief A resource
     *
     * Resources may by dynamic of static. Dynamic resources are updated after they are created, possibly by a shader,
     * while static resources are loaded once and that's that
     */
    struct Resource {
        ResourceType type = {};
        bool is_dynamic = true;
    };

    struct Sampler {};

    struct Image : Resource {
        bool is_depth_tex = false;
    };

    struct Buffer : Resource {
        memory::Bytes size = 0;
    };

    struct Framebuffer {
        glm::uvec2 size;

        uint32_t num_attachments;
    };

    struct Renderpass {};

    struct ResourceBindingDescription {
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
        ShaderStageFlags stages;

        bool operator==(const ResourceBindingDescription& other);

        bool operator!=(const ResourceBindingDescription& other);
    };

    /*!
     * \brief The interface for a pipeline. Includes both inputs (descriptors) and outputs (framebuffers)
     */
    struct PipelineInterface {
        std::unordered_map<std::string, ResourceBindingDescription> bindings;
    };

    struct Pipeline {};

    struct Semaphore {};

    struct PresentSemaphore {};

    struct Fence {};

    struct DescriptorPool {};

    struct DescriptorSet {};

    // TODO: This struct actually maps pretty directly to a Vulkan barrier, so it doesn't map well to a D3D12 barrier. Figure out how to
    // make it D3D12-friendly
    struct ResourceBarrier {
        Resource* resource_to_barrier;

        /*!
         * \brief The resource access that much finish before this barrier executed
         */
        AccessFlags access_before_barrier;

        /*!
         * \brief The resource access that must wait for this battier to finish executing
         */
        AccessFlags access_after_barrier;

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

        union {
            struct {
                ImageAspectFlags aspect;
            } image_memory_barrier;

            struct {
                memory::Bytes offset;
                memory::Bytes size;
            } buffer_memory_barrier;
        };
    };

    struct DescriptorImageInfo {
        Image* image;
        shaderpack::TextureFormat format;
        Sampler* sampler;
    };

    struct DescriptorBufferInfo {
        Buffer* buffer;
    };

    union DescriptorResourceInfo {
        /*!
         * \brief Information to update an image descriptor
         */
        DescriptorImageInfo image_info;

        /*!
         * \brief Information to update a buffer descriptor
         */
        DescriptorBufferInfo buffer_info;
    };

    struct DescriptorSetWrite {
        /*!
         * \brief Pointer to the descriptor set to write to
         */
        DescriptorSet* set;

        /*!
         * \brief The specific binding in the set that you want to write to
         */
        uint32_t first_binding;

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
        std::pmr::vector<DescriptorResourceInfo> resources;
    };
#pragma endregion

    ShaderStageFlags operator|=(ShaderStageFlags lhs, ShaderStageFlags rhs);
} // namespace nova::renderer::rhi
