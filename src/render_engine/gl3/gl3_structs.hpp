/*!
 * \brief Gl2 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois
 * \date 01-Apr-19.
 */

#pragma once
#include <unordered_map>
#include "glad/glad.h"
#include "nova_renderer/rhi_types.hpp"

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
        std::vector<Gl3Descriptor> descriptor;
    };

    struct Gl3SamplerDescriptor : DescriptorSet {
        Gl3SamplerState sampler;
    };

    struct Gl3DescriptorPool : DescriptorPool {
        std::vector<Gl3Descriptor> descriptors;
        std::vector<Gl3SamplerDescriptor> sampler_sets;
    };

    struct Gl3PipelineInterface : PipelineInterface {
        // GL3 is annoying. I need a compiled and linked program to get uniform locations, so I can't put the uniform
        // locations in the pipeline interface even though that's conceptually what I should do :(
    };

    struct Gl3Pipeline : Pipeline {
        GLuint id = 0;

        std::unordered_map<ResourceBindingDescription, GLuint> uniform_cache;
    };
} // namespace nova::renderer::rhi
