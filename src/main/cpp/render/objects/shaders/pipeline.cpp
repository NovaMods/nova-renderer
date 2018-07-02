/*!
 * \author David
 * \date 17-May-16.
 */

#include <cstdlib>
#include <algorithm>

#include <easylogging++.h>
#include <cstdint>
#include <nova/profiler.h>
#include "pipeline.h"
#include "../../vulkan/render_context.h"
#include "../resources/shader_resource_manager.h"
#include "../../nova_renderer.h"
#include "../../windowing/glfw_vk_window.h"
#include "../meshes/vertex_attributes.h"
#include "../renderpasses/renderpass_builder.h"
#include "../../../3rdparty/SPIRV-Cross/spirv_glsl.hpp"

namespace nova {
    std::unordered_map<std::string, std::vector<pipeline_object>> make_pipelines(const shaderpack_data& shaderpack,
                                                                              std::unordered_map<std::string, pass_vulkan_information> renderpasses_by_pass,
                                                                              std::shared_ptr<render_context> context) {
        NOVA_PROFILER_SCOPE;;
        auto ret_val = std::unordered_map<std::string, std::vector<pipeline_object>>{};

        for(const auto& pipelines : shaderpack.pipelines_by_pass) {
            const auto& renderpass = renderpasses_by_pass[pipelines.first];
            auto cur_list = std::vector<pipeline_object>{};

            for(const auto& pipeline_create_info : pipelines.second) {
                cur_list.push_back(make_pipeline(pipeline_create_info, renderpass, context->device));
            }

            ret_val[pipelines.first] = cur_list;
        }

        return ret_val;
    }

    pipeline_object make_pipeline(const pipeline_data& pipeline_create_info, const pass_vulkan_information& renderpass_info, const vk::Device device) {
        NOVA_PROFILER_SCOPE;;
        LOG(INFO) << "Making VkPipeline for pipeline " << pipeline_create_info.name;

        // Creates a pipeline out of compiled shaders
        auto states_vec = pipeline_create_info.states.value_or(std::vector<state_enum>{});
        const auto& states_end = states_vec.end();

        vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info = {};

        /**
         * Shader stages and Descriptor Sets
         */

        // We got an interface from each module.. now we have to combine them, somehow
        // If a descriptor with the same name exists in multiple shader modules, it MUST have the same set and binding
        // or this whole thing falls apart
        // so let's go through all the modules' interfaces, adding the descriptors from them to one big list. We'll
        // make sure that no one put a descriptor at one location in one file and a different location in another

        auto pipeline_data = pipeline_object{};
        pipeline_data.name = pipeline_create_info.name;

        std::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos;
        auto all_layouts = std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>>{};

        auto vertex_module = create_shader_module(pipeline_create_info.shader_sources.vertex_source, shaderc_vertex_shader, device);

        vk::PipelineShaderStageCreateInfo vertex_create_info = {};
        vertex_create_info.stage = vk::ShaderStageFlagBits::eVertex;
        vertex_create_info.module = vertex_module.module;
        vertex_create_info.pName = "main";
        stage_create_infos.push_back(vertex_create_info);
        LOG(TRACE) << "Using shader module " << (VkShaderModule)vertex_create_info.module;

        add_bindings_from_shader(pipeline_data, vertex_module, "vertex", all_layouts);

        auto fragment_module = create_shader_module(pipeline_create_info.shader_sources.fragment_source, shaderc_fragment_shader, device);

        vk::PipelineShaderStageCreateInfo fragment_create_info = {};
        fragment_create_info.stage = vk::ShaderStageFlagBits::eFragment;
        fragment_create_info.module = fragment_module.module;
        fragment_create_info.pName = "main";
        stage_create_infos.push_back(fragment_create_info);
        LOG(TRACE) << "Using shader module " << (VkShaderModule)fragment_create_info.module;

        add_bindings_from_shader(pipeline_data, fragment_module, "fragment", all_layouts);

        if(pipeline_create_info.shader_sources.geometry_source) {
            auto geometry_module = create_shader_module(pipeline_create_info.shader_sources.geometry_source.value(), shaderc_geometry_shader, device);

            vk::PipelineShaderStageCreateInfo geometry_create_info = {};
            geometry_create_info.stage = vk::ShaderStageFlagBits::eGeometry;
            geometry_create_info.module = geometry_module.module;
            geometry_create_info.pName = "main";
            stage_create_infos.push_back(geometry_create_info);

            add_bindings_from_shader(pipeline_data, geometry_module, "geometry", all_layouts);
            LOG(TRACE) << "Using shader module " << (VkShaderModule)geometry_create_info.module;
        }

        if(pipeline_create_info.shader_sources.tessellation_control_source) {
            auto tesc_module = create_shader_module(pipeline_create_info.shader_sources.tessellation_control_source.value(), shaderc_tess_control_shader, device);

            vk::PipelineShaderStageCreateInfo tesc_create_info = {};
            tesc_create_info.stage = vk::ShaderStageFlagBits::eTessellationControl;
            tesc_create_info.module = tesc_module.module;
            tesc_create_info.pName = "main";
            stage_create_infos.push_back(tesc_create_info);

            add_bindings_from_shader(pipeline_data, tesc_module, "tessellation control", all_layouts);
            LOG(TRACE) << "Using shader module " << (VkShaderModule)tesc_create_info.module;
        }

        if(pipeline_create_info.shader_sources.tessellation_evaluation_source) {
            auto tese_module = create_shader_module(pipeline_create_info.shader_sources.tessellation_evaluation_source.value(), shaderc_tess_evaluation_shader, device);

            vk::PipelineShaderStageCreateInfo tese_create_info = {};
            tese_create_info.stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
            tese_create_info.module = tese_module.module;
            tese_create_info.pName = "main";
            stage_create_infos.push_back(tese_create_info);

            add_bindings_from_shader(pipeline_data, tese_module, "tessellation evaluation", all_layouts);
            LOG(TRACE) << "Using shader module " << (VkShaderModule)tese_create_info.module;
        }

        graphics_pipeline_create_info.stageCount = static_cast<uint32_t>(stage_create_infos.size());
        graphics_pipeline_create_info.pStages = stage_create_infos.data();

        std::stringstream ss;
        ss << "Adding layouts\n";

        std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> bindings_for_set;
        for(const auto& layouts_for_set : pipeline_data.resource_bindings) {
            const resource_binding& layout = layouts_for_set.second;

            bindings_for_set[layout.set].push_back(layout.to_vk_binding());
            ss << "\t(set=" << layout.set << " binding=" << layout.binding << " type=" << vk::to_string(layout.descriptorType) << ")\n";
        }

        LOG(DEBUG) << ss.str();
        ss.str("");
        ss << "Creating descriptor set layouts\n";

        for(const auto& layouts_for_set : bindings_for_set) {
            const auto &layouts = layouts_for_set.second;
            auto dsl_create_info = vk::DescriptorSetLayoutCreateInfo()
                    .setBindingCount(layouts.size())
                    .setPBindings(layouts.data());

            pipeline_data.layouts[layouts_for_set.first] = device.createDescriptorSetLayout(dsl_create_info);
            ss << "\t" << (VkDescriptorSetLayout)pipeline_data.layouts[layouts_for_set.first] << " for set " << layouts_for_set.first << "\n";
        }

        LOG(DEBUG) << ss.str();
        ss.str("");
        ss << "Adding descriptor set layouts\n";

        // Order all the descriptor set layouts by set, checking for discontinuities

        const auto& layouts = pipeline_data.layouts;
        auto ordered_layouts = std::vector<vk::DescriptorSetLayout>{};
        for(uint32_t i = 0; i < layouts.size(); i++) {
            if(layouts.find(i) == layouts.end()) {
                LOG(WARNING) << "Discontinuity detected! You're skipping descriptor set " << i << " and this isn't supported because honestly I don't know how to deal with it. Nova won't load pipeline " << pipeline_create_info.name;
                return {};
            }

            ordered_layouts.push_back(layouts.at(i));
            ss << "\t" << (VkDescriptorSetLayout)layouts.at(i) << " for set " << i << "\n";
        }
        ss << "to pipeline layout";
        LOG(DEBUG) << ss.str();

        auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(static_cast<uint32_t>(ordered_layouts.size()))
            .setPSetLayouts(ordered_layouts.data());

        auto pipeline_layout = device.createPipelineLayout(pipeline_layout_create_info);
        graphics_pipeline_create_info.layout = pipeline_layout;
        pipeline_data.layout = pipeline_layout;

        /**
         * Vertex input state
         */

        // TOOD: Read this from the material

        // The vertex data is known by Nova. It just is. Each shader has inputs for all the vertex data because honestly
        // doing it differently is super hard. This will waste some VRAM but the number of vertices per chunk and
        // number of chunks will present a much easier win, especially since chunks are the big stuff and they will
        // always have all the vertex attributes
        vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};

        std::vector<vk::VertexInputBindingDescription> binding_descriptions;
        std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;

        if(!pipeline_create_info.vertex_fields) {
            LOG(ERROR) << "Pipeline " << pipeline_create_info.name << " doesn't declare any vertex fields. This is an error and I won't stand for it!";

            return {};
        }

        uint32_t cur_binding = 0;
        for(const auto& vertex_field : pipeline_create_info.vertex_fields.value()) {
            if(vertex_field == vertex_field_enum::Empty) {
                continue;
            }
            const vertex_attribute& attribute = get_all_vertex_attributes()[vertex_field.to_string()];
            attribute_descriptions.emplace_back(cur_binding, 0, attribute.format, attribute.offset);
            cur_binding++;
        }

        uint32_t total_vertex_size = get_total_vertex_size();
        binding_descriptions.emplace_back(0, total_vertex_size, vk::VertexInputRate::eVertex);

        pipeline_data.attributes = pipeline_create_info.vertex_fields.value();

        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();

        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = binding_descriptions.data();

        graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;

        /**
         * Pipeline input assembly
         */

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};

        const auto topology = pipeline_create_info.primitive_mode.value_or(primitive_topology_enum::Triangles);

        input_assembly_create_info.topology = to_vk_topology(topology);

        graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;

        /**
         * Tessellation state
         */

        graphics_pipeline_create_info.pTessellationState = nullptr;

        /**
         * \brief Viewport state
         */

        vk::Viewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = renderpass_info.framebuffer_size.width;
        viewport.height = renderpass_info.framebuffer_size.height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = vk::Extent2D{renderpass_info.framebuffer_size.width, renderpass_info.framebuffer_size.height};

        vk::PipelineViewportStateCreateInfo viewport_create_info = {};
        viewport_create_info.scissorCount = 1;
        viewport_create_info.pScissors = &scissor;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pViewports = &viewport;

        graphics_pipeline_create_info.pViewportState = &viewport_create_info;

        /**
         * Renderpass
         */

        graphics_pipeline_create_info.renderPass = renderpass_info.renderpass;

        /**
         * Rasterization state
         */

        vk::PipelineRasterizationStateCreateInfo raster_create_info = {};
        raster_create_info.polygonMode = vk::PolygonMode::eFill;
        raster_create_info.cullMode == vk::CullModeFlagBits::eBack;
        raster_create_info.frontFace = vk::FrontFace::eCounterClockwise;
        raster_create_info.lineWidth = 1;

        if(pipeline_create_info.depth_bias) {
            raster_create_info.depthBiasEnable = static_cast<vk::Bool32>(true);
            raster_create_info.depthBiasConstantFactor = *pipeline_create_info.depth_bias;

            if(pipeline_create_info.slope_scaled_depth_bias) {
                raster_create_info.depthBiasSlopeFactor = *pipeline_create_info.slope_scaled_depth_bias;
            }
        }

        graphics_pipeline_create_info.pRasterizationState = &raster_create_info;

        /**
         * Multisample state
         *
         * While Nova supports MSAA, it won't be useful on shaderpacks that implement deferred rendering and is thus
         * off by default
         */

        vk::PipelineMultisampleStateCreateInfo multisample_create_info = {};

        graphics_pipeline_create_info.pMultisampleState = &multisample_create_info;

        /**
         * Depth and stencil state
         */

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.depthTestEnable = static_cast<vk::Bool32>((std::find(states_vec.begin(), states_vec.end(), state_enum::DisableDepthTest) == states_end));
        depth_stencil_create_info.depthWriteEnable = static_cast<vk::Bool32>((std::find(states_vec.begin(), states_vec.end(), state_enum::DisableDepthWrite) == states_end));
        auto depth_op = pipeline_create_info.depth_func.value_or(compare_op_enum::Less);
        depth_stencil_create_info.depthCompareOp = to_vk_compare_op(depth_op);

        depth_stencil_create_info.stencilTestEnable = static_cast<vk::Bool32>(std::find(states_vec.begin(), states_vec.end(), state_enum::EnableStencilTest) != states_end);
        if(pipeline_create_info.back_face) {
            depth_stencil_create_info.back = pipeline_create_info.back_face.value().to_vk_stencil_op_state();
        }
        if(pipeline_create_info.front_face) {
            depth_stencil_create_info.front = pipeline_create_info.front_face.value().to_vk_stencil_op_state();
        }
        depth_stencil_create_info.minDepthBounds = 0;
        depth_stencil_create_info.maxDepthBounds = 1;

        graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;

        /**
         * Color blend state
         */

        vk::PipelineColorBlendStateCreateInfo blend_create_info = {};
        blend_create_info.logicOpEnable = static_cast<vk::Bool32>(false);   // Not 100% how to use this so imma just disable it

        auto attachmentBlendStates = std::vector<vk::PipelineColorBlendAttachmentState>{};

        if(pipeline_create_info.states) {
            bool enable_blending = false;
            const auto& states_val = pipeline_create_info.states.value();
            if(std::find(states_val.begin(), states_val.end(), state_enum::Blending) != states_val.end()) {
                enable_blending = true;
            }

            attachmentBlendStates.resize(renderpass_info.num_attachments);
            for(auto &blend_state : attachmentBlendStates) {
                blend_state.blendEnable = static_cast<vk::Bool32>(enable_blending);

                blend_state.srcColorBlendFactor = to_vk_blend_factor(pipeline_create_info.source_blend_factor.value_or(blend_factor_enum::SrcAlpha));
                blend_state.dstColorBlendFactor = to_vk_blend_factor(pipeline_create_info.destination_blend_factor.value_or(blend_factor_enum::OneMinusSrcAlpha));
                blend_state.colorBlendOp = vk::BlendOp::eAdd;
                blend_state.srcAlphaBlendFactor = to_vk_blend_factor(pipeline_create_info.source_blend_factor.value_or(blend_factor_enum::SrcAlpha));
                blend_state.dstAlphaBlendFactor = to_vk_blend_factor(pipeline_create_info.destination_blend_factor.value_or(blend_factor_enum::OneMinusSrcAlpha));
                blend_state.alphaBlendOp = vk::BlendOp::eAdd;
                blend_state.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                             vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            }
        }
        blend_create_info.attachmentCount = static_cast<uint32_t>(attachmentBlendStates.size());
        blend_create_info.pAttachments = attachmentBlendStates.data();

        // Ignoring blend constants here as well - they probably won't be used and I don't want to make the Nova
        // material struct too huge

        graphics_pipeline_create_info.pColorBlendState = &blend_create_info;

        // TODO: Handle dynamic state


        LOG(TRACE) << "Creating pipeline with vertex module = " << (VkShaderModule)graphics_pipeline_create_info.pStages[0].module
                  << " and fragment module = " << (VkShaderModule)graphics_pipeline_create_info.pStages[1].module;
        pipeline_data.pipeline = device.createGraphicsPipeline(vk::PipelineCache(), graphics_pipeline_create_info);
        LOG(TRACE) << "Created pipeline " << pipeline_create_info.name << " (VkPipeline " << (VkPipeline)pipeline_data.pipeline << ")";

        return pipeline_data;
    }

    void add_bindings_from_shader(pipeline_object& pipeline_data, const shader_module &shader_module, const std::string& shader_stage_name, std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>>& all_layouts) {
        std::unordered_map<std::string, resource_binding>& all_bindings = pipeline_data.resource_bindings;

        for(const auto &new_named_binding : shader_module.bindings) {
            const resource_binding& new_binding = new_named_binding.second;

            if(all_bindings.find(new_named_binding.first) != all_bindings.end()) {
                // We already know about this descriptor. Validate it
                const auto &existing_binding = all_bindings.at(new_named_binding.first);
                if(existing_binding != new_binding) {
                    LOG(ERROR) << shader_stage_name << " shader module redeclares descriptor " << new_named_binding.first
                               << " from location (set=" << existing_binding.set << ", binding=" << existing_binding.binding
                               << ") to location (set=" << new_binding.set << ", binding=" << new_binding.binding << ") ";

                } else {
                    // We have a binding, now we need to merge the new binding into the existing binding
                    all_bindings[new_named_binding.first].stageFlags |= new_binding.stageFlags;

                    auto& layouts_for_set = all_layouts[new_binding.set];
                    auto old_layout = std::find_if(layouts_for_set.begin(), layouts_for_set.end(), [&](const auto& layout){return layout.binding == new_binding.binding;});
                    if(old_layout->descriptorType != new_binding.descriptorType) {
                        LOG(ERROR) << "You've used the same name for resources of different types. This won't work - Nova will ignore those bindings and things will act weird";
                    } else {
                        old_layout->stageFlags |= new_binding.stageFlags;
                    }
                }

            } else {
                // New binding! Let's add it in
                all_bindings[new_named_binding.first] = new_named_binding.second;

                all_layouts[new_binding.set].push_back(vk::DescriptorSetLayoutBinding()
                        .setDescriptorType(new_binding.descriptorType)
                        .setDescriptorCount(new_binding.descriptorCount)
                        .setBinding(new_binding.binding)
                        .setStageFlags(new_binding.stageFlags));
            }
        }
    }

    shader_module create_shader_module(const shader_file& source, const shaderc_shader_kind& stage, const vk::Device& device) {
        LOG(DEBUG) << "Creating a shader module for file " << source.lines[0].shader_name;
        std::vector<uint32_t> spirv_source;

        switch(source.language) {
            case shader_langauge_enum::GLSL:
                spirv_source = glsl_to_spirv(source.lines, stage);
                break;
            default:
                LOG(ERROR) << "Shader language " << source.language.to_string() << " is not implemented yet";
        }

        vk::ShaderModuleCreateInfo create_info = {};
        create_info.codeSize = spirv_source.size() * sizeof(uint32_t);
        create_info.pCode = spirv_source.data();

        auto module = shader_module{};
        module.module = device.createShaderModule(create_info);
        LOG(TRACE) << "Created shader module " << (VkShaderModule)module.module;

        vk::ShaderStageFlags stages;
        switch(stage) {
            case shaderc_vertex_shader:
                LOG(INFO) << "shader file " << source.lines[0].shader_name << " is vertex shader";
                stages = vk::ShaderStageFlagBits::eVertex;
                break;
            case shaderc_geometry_shader:
                stages = vk::ShaderStageFlagBits::eGeometry;
                LOG(INFO) << "shader file " << source.lines[0].shader_name << " is geometry shader";
                break;
            case shaderc_tess_control_shader:
                stages = vk::ShaderStageFlagBits::eTessellationControl;
                LOG(INFO) << "shader file " << source.lines[0].shader_name << " is tesc shader";
                break;
            case shaderc_tess_evaluation_shader:
                stages = vk::ShaderStageFlagBits::eTessellationEvaluation;
                LOG(INFO) << "shader file " << source.lines[0].shader_name << " is tese shader";
                break;
            case shaderc_fragment_shader:
                stages = vk::ShaderStageFlagBits::eFragment;
                LOG(INFO) << "shader file " << source.lines[0].shader_name << " is fragment shader";
                break;
            case shaderc_compute_shader:
                stages = vk::ShaderStageFlagBits::eCompute;
                LOG(INFO) << "shader file " << source.lines[0].shader_name << " is compute shader";
                break;
            default:
                LOG(ERROR) << "Shader kind " << stage << " not handled. Soz.";
        }

        LOG(INFO) << "Converted to vk stage " << vk::to_string(stages);

        module.bindings = get_interface_of_spirv(spirv_source, stages);

        output_compiled_shader(source, spirv_source);

        return module;
    }

    void output_compiled_shader(const shader_file &original_shader_file, const std::vector<uint32_t>& spirv) {
        std::stringstream filename;
        filename << original_shader_file.lines[0].shader_name.string();
        filename << ".debug.spirv";

        std::ofstream spirv_output{filename.str(), std::ios::binary};
        spirv_output.write((char*)spirv.data(), spirv.size() * 4);

        spirv_output.close();
    }

    std::vector<uint32_t> glsl_to_spirv(const std::vector<shader_line>& shader_lines, shaderc_shader_kind stages) {
        LOG(TRACE) << "Compiling as shader type " << stages;
        std::stringstream ss;
        for(auto& line : shader_lines) {
            ss << line.line << "\n";
        }

        // TODO: Cache this
        shaderc::Compiler compiler;
        shaderc::CompileOptions compile_options;
        compile_options.SetTargetEnvironment(shaderc_target_env_vulkan, 0);
        compile_options.SetSourceLanguage(shaderc_source_language_glsl);
        compile_options.SetWarningsAsErrors();  // TODO: Make this configurable from shaders.json or something
        compile_options.SetGenerateDebugInfo();
        // TODO: Let users set optimization level too

        std::string source = ss.str();
        auto result = compiler.CompileGlslToSpv(source, stages, shader_lines[0].shader_name.string().c_str(), compile_options);

        if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
            LOG(ERROR) << result.GetErrorMessage();
            return {};
        }

        return {result.cbegin(), result.cend()};
    }

    std::unordered_map<std::string, resource_binding> get_interface_of_spirv(const std::vector<uint32_t>& spirv_source, const vk::ShaderStageFlags& stages) {

        std::unordered_map<std::string, resource_binding> bindings;

        spirv_cross::CompilerGLSL glsl(spirv_source);

        auto resources = glsl.get_shader_resources();

        for(const auto& resource : resources.sampled_images) {
            auto set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            auto binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            auto descriptor_set_layout_binding = resource_binding{};
            descriptor_set_layout_binding.set = set;
            descriptor_set_layout_binding.setBinding(binding);
            descriptor_set_layout_binding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
            descriptor_set_layout_binding.setStageFlags(stages);
            descriptor_set_layout_binding.setDescriptorCount(1);

            bindings[resource.name] = descriptor_set_layout_binding;

            LOG(INFO) << "Descriptor " << resource.name << " uses stages " << vk::to_string(stages);
        }

        for(const auto& resource : resources.uniform_buffers) {
            auto set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
            auto binding = glsl.get_decoration(resource.id, spv::DecorationBinding);

            auto descriptor_set_layout_binding = resource_binding();
            descriptor_set_layout_binding.set = set;
            descriptor_set_layout_binding.setBinding(binding);
            descriptor_set_layout_binding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
            descriptor_set_layout_binding.setStageFlags(stages);
            descriptor_set_layout_binding.setDescriptorCount(1);

            bindings[resource.name] = descriptor_set_layout_binding;

            LOG(INFO) << "Descriptor " << resource.name << " uses stages " << vk::to_string(stages);
        }

        return bindings;
    }

    bool resource_binding::operator==(const resource_binding& other) const {
        return other.set == set && other.binding == binding;
    }

    bool resource_binding::operator!=(const resource_binding& other) const {
        return !(*this == other);
    }

    vk::DescriptorSetLayoutBinding resource_binding::to_vk_binding() const {
        return vk::DescriptorSetLayoutBinding()
            .setDescriptorType(descriptorType)
            .setDescriptorCount(descriptorCount)
            .setStageFlags(stageFlags)
            .setBinding(binding)
            .setPImmutableSamplers(pImmutableSamplers);
    }
}
