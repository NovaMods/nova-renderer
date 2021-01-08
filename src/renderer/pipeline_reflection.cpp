#include "pipeline_reflection.hpp"

#include <rx/core/log.h>
#include <spirv_cross.hpp>

namespace nova::renderer {
    using namespace rhi;

    RX_LOG("PipelineReflection", logger);

    std::unordered_map<std::string, RhiResourceBindingDescription> get_all_descriptors(const RhiGraphicsPipelineState& pipeline_state) {
        std::unordered_map<std::string, RhiResourceBindingDescription> bindings;

        get_shader_module_descriptors(pipeline_state.vertex_shader.source, ShaderStage::Vertex, bindings);

        if(pipeline_state.geometry_shader) {
            get_shader_module_descriptors(pipeline_state.geometry_shader->source, ShaderStage::Geometry, bindings);
        }
        if(pipeline_state.pixel_shader) {
            get_shader_module_descriptors(pipeline_state.pixel_shader->source, ShaderStage::Pixel, bindings);
        }

        return bindings;
    }

    void get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                       const ShaderStage shader_stage,
                                       std::unordered_map<std::string, RhiResourceBindingDescription>& bindings) {
        const spirv_cross::Compiler shader_compiler{spirv.data(), spirv.size()};
        const spirv_cross::ShaderResources& resources = shader_compiler.get_shader_resources();

        for(const auto& resource : resources.separate_images) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, DescriptorType::Texture);
        }

        for(const auto& resource : resources.separate_samplers) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, DescriptorType::Sampler);
        }

        for(const auto& resource : resources.uniform_buffers) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, DescriptorType::UniformBuffer);
        }

        for(const auto& resource : resources.storage_buffers) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, DescriptorType::StorageBuffer);
        }
    }

    void add_resource_to_bindings(std::unordered_map<std::string, RhiResourceBindingDescription>& bindings,
                                  const ShaderStage shader_stage,
                                  const spirv_cross::Compiler& shader_compiler,
                                  const spirv_cross::Resource& resource,
                                  const DescriptorType type) {
        const uint32_t set_idx = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding_idx = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        RhiResourceBindingDescription new_binding = {};
        new_binding.set = set_idx;
        new_binding.binding = binding_idx;
        new_binding.type = type;
        new_binding.count = 1;
        new_binding.stages = shader_stage;

        logger->debug("Pipeline reflection found resource %s of type %s in binding %u.%u",
                        resource.name.c_str(),
                        descriptor_type_to_string(type),
                        set_idx,
                        binding_idx);

        const spirv_cross::SPIRType& type_information = shader_compiler.get_type(resource.type_id);
        if(!type_information.array.empty()) {
            new_binding.count = type_information.array[0];
            // All arrays are unbounded until I figure out how to use SPIRV-Cross to detect unbounded arrays
            new_binding.is_unbounded = true;
        }

        const std::string& resource_name = resource.name.c_str();

        if(auto* binding = bindings.find(resource_name)) {
            // Existing binding. Is it the same as our binding?
            RhiResourceBindingDescription& existing_binding = *binding;
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                logger->error("You have two different uniforms named %s in different shader stages. This is not allowed. Use unique names",
                              resource.name);

            } else {
                // Same binding, probably at different stages - let's fix that
                existing_binding.stages |= shader_stage;
            }

        } else {
            // Totally new binding!
            bindings.insert(resource_name, new_binding);
        }
    }
} // namespace nova::renderer
