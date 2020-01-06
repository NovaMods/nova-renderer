#include "nova_renderer/frontend/pipeline_storage.hpp"

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/rhi/render_engine.hpp"

#include "spirv_glsl.hpp"

namespace nova::renderer {
    PipelineStorage::PipelineStorage(NovaRenderer& renderer, mem::AllocatorHandle<>& allocator)
        : renderer(renderer), device(renderer.get_engine()), allocator(allocator) {}

    std::optional<const Pipeline&> PipelineStorage::get_pipeline(const std::string& pipeline_name) const {
        if(const auto itr = pipelines.find(pipeline_name); itr != pipelines.end()) {
            return std::make_optional(itr->second);

        } else {
            return {};
        }
    }

    bool PipelineStorage::add_pipeline_from_shaderpack(const shaderpack::PipelineCreateInfo& pipeline_create_info) {
        const auto rp_create = renderer.get_renderpass_metadata(pipeline_create_info.pass);
        if(!rp_create) {
            NOVA_LOG(ERROR) << "Pipeline " << pipeline_create_info.name << " wants to be rendered by renderpass "
                            << pipeline_create_info.pass << ", but that renderpass doesn't have any metadata";
            return  false;
        }

        std::unordered_map<std::string, rhi::ResourceBindingDescription> bindings;

        ntl::Result<rhi::PipelineInterface*> pipeline_interface = create_pipeline_interface(pipeline_create_info,
                                                                                            rp_create->texture_outputs,
                                                                                            rp_create->depth_texture);
        if(!pipeline_interface) {
            NOVA_LOG(ERROR) << "Pipeline " << pipeline_create_info.name
                            << " has an invalid interface: " << pipeline_interface.error.to_string();
            return false;
        }

        ntl::Result<PipelineReturn> pipeline_result = create_graphics_pipeline(*pipeline_interface, pipeline_create_info);
        if(pipeline_result) {
            auto [pipeline, pipeline_metadata] = *pipeline_result;

            pipelines.emplace(pipeline_create_info.name, pipeline);
            pipeline_metadatas.emplace(pipeline_create_info.name, pipeline_metadata);

            return true;
        } else {
            NOVA_LOG(ERROR) << "Could not create pipeline " << pipeline_create_info.name << ": " << pipeline_result.error.to_string();

            return false;
        }
    }

    ntl::Result<PipelineReturn> PipelineStorage::create_graphics_pipeline(
        rhi::PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& pipeline_create_info) const {
        Pipeline pipeline;
        PipelineMetadata metadata;

        metadata.data = pipeline_create_info;

        ntl::Result<rhi::Pipeline*> rhi_pipeline = device->create_pipeline(pipeline_interface, pipeline_create_info, allocator);
        if(rhi_pipeline) {
            pipeline.pipeline = *rhi_pipeline;

        } else {
            ntl::NovaError error = ntl::NovaError(format(fmt("Could not create pipeline {:s}"), pipeline_create_info.name.c_str()).c_str(),
                                                  std::move(rhi_pipeline.error));
            return ntl::Result<PipelineReturn>(std::move(error));
        }

        return ntl::Result(PipelineReturn{pipeline, metadata});
    }

    ntl::Result<rhi::PipelineInterface*> PipelineStorage::create_pipeline_interface(
        const shaderpack::PipelineCreateInfo& pipeline_create_info,
        const std::pmr::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
        const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) const {
        std::unordered_map<std::string, rhi::ResourceBindingDescription> bindings;
        bindings.reserve(32); // Probably a good estimate

        get_shader_module_descriptors(pipeline_create_info.vertex_shader.source, rhi::ShaderStage::Vertex, bindings);

        if(pipeline_create_info.tessellation_control_shader) {
            get_shader_module_descriptors(pipeline_create_info.tessellation_control_shader->source,
                                          rhi::ShaderStage::TessellationControl,
                                          bindings);
        }
        if(pipeline_create_info.tessellation_evaluation_shader) {
            get_shader_module_descriptors(pipeline_create_info.tessellation_evaluation_shader->source,
                                          rhi::ShaderStage::TessellationEvaluation,
                                          bindings);
        }
        if(pipeline_create_info.geometry_shader) {
            get_shader_module_descriptors(pipeline_create_info.geometry_shader->source, rhi::ShaderStage::Geometry, bindings);
        }
        if(pipeline_create_info.fragment_shader) {
            get_shader_module_descriptors(pipeline_create_info.fragment_shader->source, rhi::ShaderStage::Fragment, bindings);
        }

        return device->create_pipeline_interface(bindings, color_attachments, depth_texture, allocator);
    }

    void PipelineStorage::get_shader_module_descriptors(const std::pmr::vector<uint32_t>& spirv,
                                                        const rhi::ShaderStage shader_stage,
                                                        std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings) {
        std::pmr::vector<uint32_t> spirv_std(spirv.begin(), spirv.end());
        const spirv_cross::CompilerGLSL shader_compiler(spirv_std.data(), spirv_std.size());
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const spirv_cross::Resource& resource : resources.sampled_images) {
            NOVA_LOG(TRACE) << "Found a texture resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::CombinedImageSampler);
        }

        for(const spirv_cross::Resource& resource : resources.uniform_buffers) {
            NOVA_LOG(TRACE) << "Found a UBO resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::UniformBuffer);
        }

        for(const spirv_cross::Resource& resource : resources.storage_buffers) {
            NOVA_LOG(TRACE) << "Found a SSBO resource named " << resource.name;
            add_resource_to_bindings(bindings, shader_stage, shader_compiler, resource, rhi::DescriptorType::StorageBuffer);
        }
    }

    void PipelineStorage::add_resource_to_bindings(std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings,
                                                   const rhi::ShaderStage shader_stage,
                                                   const spirv_cross::CompilerGLSL& shader_compiler,
                                                   const spirv_cross::Resource& resource,
                                                   const rhi::DescriptorType type) {
        const uint32_t set = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        rhi::ResourceBindingDescription new_binding = {};
        new_binding.set = set;
        new_binding.binding = binding;
        new_binding.type = type;
        new_binding.count = 1;
        new_binding.stages = shader_stage;

        const spirv_cross::SPIRType& type_information = shader_compiler.get_type(resource.type_id);
        if(!type_information.array.empty()) {
            new_binding.count = type_information.array[0];
            // All arrays are unbounded until I figure out how to use SPIRV-Cross to detect unbounded arrays
            new_binding.is_unbounded = true;
        }

        const std::string& resource_name = resource.name;

        if(const auto itr = bindings.find(resource_name); itr != bindings.end()) {
            // Existing binding. Is it the same as our binding?
            rhi::ResourceBindingDescription& existing_binding = itr->second;
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name
                                << " in different shader stages. This is not allowed. Use unique names";

            } else {
                // Same binding, probably at different stages - let's fix that
                existing_binding.stages |= shader_stage;
            }

        } else {
            // Totally new binding!
            bindings[resource_name] = new_binding;
        }
    }
} // namespace nova::renderer
