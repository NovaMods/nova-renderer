#pragma once

#include <bvestl/polyalloc/polyalloc.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include "device_memory_resource.hpp"
#include "nova_renderer/rhi_enums.hpp"
#include "shaderpack_data.hpp"

namespace nova::renderer::rhi {

#pragma region Structs
    struct BufferCreateInfo {

        uint64_t size = 0;

        BufferUsage buffer_usage{};

        DeviceMemoryAllocation allocation;
    };

    struct DeviceMemory {};

    /*!
     * \brief A resource
     *
     * Resources may by dynamic of static. Dynamic resources are updated after they are created, possibly by a shader,
     * while static resources are loaded once and that's that
     */
    struct Resource {
        ResourceType type;
        bool is_dynamic = false;
    };

    struct Sampler {};

    struct Image : Resource {
        bool is_depth_tex = false;
    };

    struct Buffer : Resource {
        uint64_t size = 0;
    };

    struct Framebuffer {
        glm::uvec2 size;
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

    ShaderStageFlags operator|=(const ShaderStageFlags lhs, const ShaderStageFlags rhs);
} // namespace nova::renderer::rhi
