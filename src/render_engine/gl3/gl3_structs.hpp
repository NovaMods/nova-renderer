/*!
 * \brief NvGl4 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once

#include <condition_variable>
#include <mutex>
#include <unordered_map>

#include "nova_renderer/memory/allocators.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

#include "glad/glad.h"

namespace nova::renderer::rhi {
    // Holds all the state for an OpenGL sampler object, which may or may not be an actual thing in 3.1
    struct Gl3SamplerState {};

    struct Gl3Resource {
        GLuint id = 0;
    };

    struct Gl3Image : Image, Gl3Resource {};

    struct Gl3Buffer : Buffer, Gl3Resource {};

    struct Gl3Renderpass : Renderpass {};

    struct Gl3Framebuffer : Framebuffer {
        GLuint id = 0;
    };

    /*
     * Descriptor set usage:
     * - Create pipeline interface
     * - Create descriptor set based off of interface
     * - Update descriptor set with resources
     * - Bind descriptor set in command buffer
     *
     * OpenGL descriptor sets need to hold a pointer to the resource they were updated with
     */

    struct Gl3Descriptor {
        Gl3Resource* resource = nullptr;
    };

    struct Gl3DescriptorSet : DescriptorSet {
        // The index in the array is the binding in the set
        std::pmr::vector<Gl3Descriptor> descriptors;
    };

    struct Gl3SamplerDescriptor : DescriptorSet {
        Gl3SamplerState sampler;
    };

    struct Gl3DescriptorPool : DescriptorPool {
        memory::AllocatorHandle<Gl3DescriptorSet>* descriptor_allocator;
    };

    struct Gl3Pipeline : Pipeline {
        GLuint id = 0;
    };

    struct Gl3PipelineInterface : PipelineInterface {
        std::unordered_map<std::string, GLuint> uniform_cache;
    };

    struct Gl3Fence : Fence {
        std::mutex mutex;
        std::condition_variable cv;
        bool signaled = false;

        Gl3Fence() = default;

        explicit Gl3Fence(const bool signaled) : signaled(signaled) {}
    };

    struct Gl3Semaphore : Semaphore {
        std::mutex mutex;
        std::condition_variable cv;
        bool signaled = false;

        Gl3Semaphore() = default;

        explicit Gl3Semaphore(const bool signaled) : signaled(signaled) {}
    };
} // namespace nova::renderer::rhi
