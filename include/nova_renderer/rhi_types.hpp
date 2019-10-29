#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include "nova_renderer/rhi_enums.hpp"

#include "device_memory_resource.hpp"
#include "shaderpack_data.hpp"

namespace nova::renderer::rhi {

#pragma region Structs
    struct BufferCreateInfo {
        uint64_t size = 0;

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
        uint32_t size = 0;
    };

    struct Framebuffer {
        glm::uvec2 size;

        uint32_t num_attachments;
    };

    struct Renderpass {
        bool writes_to_backbuffer = false;
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
         * \brief How you're going to access this resource just before this barrier
         *
         * Will a shader read from it before the barrier? Will the fragment depth by copied to a depth buffer before
         * this barrier? Will the resource be used as a indirect draw command buffer right before this barrier?
         */
        ResourceAccess access_before_barrier;

        /*!
         * \brief How you're going to access this resource after this barrier
         *
         * Will a shader read from it after the barrier? Will the fragment depth by copied to a depth buffer after
         * this barrier? Will the resource be used as a indirect draw command buffer right after this barrier?
         */
        ResourceAccess access_after_barrier;

        QueueType source_queue;
        QueueType destination_queue;

        union {
            struct {
                ImageAspectFlags aspect;
                ImageLayout current_layout;
                ImageLayout new_layout;
            } image_memory_barrier;

            struct {
                uint64_t offset;
                uint64_t size;
            } buffer_memory_barrier;
        };
    };

    struct DescriptorImageUpdate {
        const Image* image;
        shaderpack::TextureFormat format;
        Sampler* sampler;
    };

    struct DescriptorSetWrite {
        const DescriptorSet* set;
        uint32_t binding;
        DescriptorImageUpdate* image_info;
        DescriptorType type;
    };
#pragma endregion

    ShaderStageFlags operator|=(ShaderStageFlags lhs, ShaderStageFlags rhs);
} // namespace nova::renderer::rhi
