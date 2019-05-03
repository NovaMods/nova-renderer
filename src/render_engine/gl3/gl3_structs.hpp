/*!
 * \brief Gl2 definitions of the structs forward-declared in render_engine.hpp
 *
 * \author ddubois 
 * \date 01-Apr-19.
 */

#pragma once
#include "glad/glad.h"
#include "nova_renderer/rhi_types.hpp"
#include <string>
#include <unordered_map>

namespace nova::renderer::rhi {
    // Holds all the state for an OpenGL sampler object, which may or may not be an actual thing in 3.1
    struct Gl3Sampler {
    };

    struct Gl3Image : Image {
        GLuint id = 0;
    };

    struct Gl3Buffer : Buffer {
        GLuint id = 0;
    };

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

    struct Gl3DescriptorSet : DescriptorSet {
        // The index in the array is the binding in the set
        std::vector<Resource*> resources;
    };

    struct Gl3SamplerDescriptor : DescriptorSet {
        Gl3Sampler sampler;
    };

    struct Gl3DescriptorPool : DescriptorPool {
        std::vector<Gl3DescriptorSet> sets;
        std::vector<Gl3SamplerDescriptor> sampler_sets;
    };

    struct Gl3PipelineInterface : PipelineInterface {
        std::unordered_map<std::string, ResourceBindingDescription> bindings;
    };

    struct Gl3Pipeline : Pipeline {
		GLuint id = 0;

		std::unordered_map<ResourceBindingDescription, GLuint> uniform_cache;
    };
}
