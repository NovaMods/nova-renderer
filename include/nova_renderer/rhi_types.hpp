#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "device_memory_resource.hpp"
#include "nova_renderer/polyalloc.hpp"
#include "nova_renderer/rhi_enums.hpp"
#include "shaderpack_data.hpp"

namespace nova::renderer::rhi {

#pragma region Structs
    struct NOVA_API BufferCreateInfo {

        uint64_t size = 0;

        BufferUsage buffer_usage{};

        DeviceMemoryAllocation allocation;
    };

    struct NOVA_API DeviceMemory {};

    /*!
     * \brief A resource
     *
     * Resources may by dynamic of static. Dynamic resources are updated after they are created, possibly by a shader,
     * while static resources are loaded once and that's that
     */
    struct NOVA_API Resource {
        ResourceType type = {};
        bool is_dynamic = false;
    };

    struct NOVA_API Sampler {};

    struct NOVA_API Image : Resource {
        bool is_depth_tex = false;
    };

    struct NOVA_API Buffer : Resource {
        uint32_t size = 0;
    };

    struct NOVA_API Framebuffer {
        glm::uvec2 size;
    };

    struct NOVA_API Renderpass {
        bool writes_to_backbuffer = false;
    };

    struct NOVA_API ResourceBindingDescription {
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
    struct NOVA_API PipelineInterface {
        std::unordered_map<std::string, ResourceBindingDescription> bindings;
    };

    struct NOVA_API Pipeline {};

    struct NOVA_API Semaphore {};

    struct NOVA_API PresentSemaphore {};

    struct NOVA_API Fence {};

    struct NOVA_API DescriptorPool {};

    struct NOVA_API DescriptorSet {};

    struct NOVA_API ResourceBarrier {
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

    struct NOVA_API DescriptorImageUpdate {
        const Image* image;
        shaderpack::TextureFormat format;
        Sampler* sampler;
    };

    struct NOVA_API DescriptorSetWrite {
        const DescriptorSet* set;
        uint32_t binding;
        DescriptorImageUpdate* image_info;
        DescriptorType type;
    };
#pragma endregion

    ShaderStageFlags NOVA_API operator|=(ShaderStageFlags lhs, ShaderStageFlags rhs);
} // namespace nova::renderer::rhi
