#include "nova_renderer/pipeline_storage.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/render_device.hpp"

#include "spirv_glsl.hpp"

namespace nova::renderer {
    RX_LOG("PipelineStorage", logger);

    using namespace mem;
    using namespace ntl;
    using namespace renderpack;

    PipelineStorage::PipelineStorage(NovaRenderer& renderer, rx::memory::allocator& allocator)
        : renderer(renderer), device(renderer.get_device()), allocator(allocator) {}

    rx::optional<renderer::Pipeline> PipelineStorage::get_pipeline(const rx::string& pipeline_name) const {
        if(const auto* pipeline = pipelines.find(pipeline_name)) {
            return *pipeline;

        } else {
            return rx::nullopt;
        }
    }

    bool PipelineStorage::create_pipeline(const PipelineStateCreateInfo& create_info) {

        Result<rhi::RhiPipelineInterface*> pipeline_interface = create_pipeline_interface(create_info);
        if(!pipeline_interface) {
            logger(rx::log::level::k_error,
                   "Pipeline %s has an invalid interface: %s",
                   create_info.name,
                   pipeline_interface.error.to_string());
            return false;
        }

        Result<PipelineReturn> pipeline_result = create_graphics_pipeline(*pipeline_interface, create_info);
        if(pipeline_result) {
            auto [pipeline, pipeline_metadata] = *pipeline_result;

            pipelines.insert(create_info.name, pipeline);
            pipeline_metadatas.insert(create_info.name, pipeline_metadata);

            return true;
        } else {
            logger(rx::log::level::k_error, "Could not create pipeline %s:%s", create_info.name, pipeline_result.error.to_string());

            return false;
        }
    }

    Result<PipelineReturn> PipelineStorage::create_graphics_pipeline(rhi::RhiPipelineInterface* pipeline_interface,
                                                                     const PipelineStateCreateInfo& pipeline_create_info) const {
        Pipeline pipeline;
        PipelineMetadata metadata;

        metadata.data = pipeline_create_info;

        Result<rhi::RhiPipeline*> rhi_pipeline = device.create_pipeline(pipeline_interface, pipeline_create_info, allocator);
        if(rhi_pipeline) {
            pipeline.pipeline = *rhi_pipeline;
            pipeline.pipeline_interface = pipeline_interface;

        } else {
            NovaError error = NovaError{rx::string::format("Could not create pipeline %s", pipeline_create_info.name),
                                        allocator.create<NovaError>(rhi_pipeline.error)};
            return ntl::Result<PipelineReturn>(rx::utility::move(error));
        }

        return Result(PipelineReturn{pipeline, metadata});
    }

    Result<rhi::RhiPipelineInterface*> PipelineStorage::create_pipeline_interface(
        const PipelineStateCreateInfo& pipeline_create_info) const {

        rx::map<rx::string, rhi::RhiResourceBindingDescription> bindings;

        get_shader_module_descriptors(pipeline_create_info.vertex_shader.source, rhi::ShaderStage::Vertex, bindings);

        if(pipeline_create_info.geometry_shader) {
            get_shader_module_descriptors(pipeline_create_info.geometry_shader->source, rhi::ShaderStage::Geometry, bindings);
        }
        if(pipeline_create_info.pixel_shader) {
            get_shader_module_descriptors(pipeline_create_info.pixel_shader->source, rhi::ShaderStage::Fragment, bindings);
        }

        return device.create_pipeline_interface(bindings,
                                                pipeline_create_info.color_attachments,
                                                pipeline_create_info.depth_texture,
                                                allocator);
    }

    void PipelineStorage::get_shader_module_descriptors(const rx::vector<uint32_t>& spirv,
                                                        const rhi::ShaderStage shader_stage,
                                                        rx::map<rx::string, rhi::RhiResourceBindingDescription>& bindings) {
        const spirv_cross::CompilerGLSL shader_compiler{spirv.data(), spirv.size()};
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const auto& resource : resources.separate_images) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::Texture);
        }

        for(const auto& resource : resources.separate_samplers) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::Sampler);
        }

        for(const auto& resource : resources.sampled_images) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::CombinedImageSampler);
        }

        for(const auto& resource : resources.uniform_buffers) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::UniformBuffer);
        }

        for(const auto& resource : resources.storage_buffers) {
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::StorageBuffer);
        }

        for(const auto& resource : resources.push_constant_buffers) {
            logger(rx::log::level::k_verbose, "Found a push constant %s", resource.name.c_str());
        }
    }

    void PipelineStorage::add_resource_to_bindings(rx::map<rx::string, rhi::RhiResourceBindingDescription>& bindings,
                                                   const rhi::ShaderStage shader_stage,
                                                   const spirv_cross::CompilerGLSL& shader_compiler,
                                                   const spirv_cross::Resource& resource,
                                                   const rhi::DescriptorType type) {
        const uint32_t set_idx = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding_idx = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        rhi::RhiResourceBindingDescription new_binding = {};
        new_binding.set = set_idx;
        new_binding.binding = binding_idx;
        new_binding.type = type;
        new_binding.count = 1;
        new_binding.stages = shader_stage;

        const spirv_cross::SPIRType& type_information = shader_compiler.get_type(resource.type_id);
        if(!type_information.array.empty()) {
            new_binding.count = type_information.array[0];
            // All arrays are unbounded until I figure out how to use SPIRV-Cross to detect unbounded arrays
            new_binding.is_unbounded = true;
        }

        const rx::string& resource_name = resource.name.c_str();

        if(auto* binding = bindings.find(resource_name)) {
            // Existing binding. Is it the same as our binding?
            rhi::RhiResourceBindingDescription& existing_binding = *binding;
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                logger(rx::log::level::k_error,
                       "You have two different uniforms named %s in different shader stages. This is not allowed. Use unique names",
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
