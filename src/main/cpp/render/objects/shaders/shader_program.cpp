/*!
 * \author David
 * \date 17-May-16.
 */

#include <cstdlib>
#include <algorithm>

#include <easylogging++.h>
#include "shader_program.h"
#include "../../vulkan/render_context.h"
#include "shader_resource_manager.h"
#include "../../nova_renderer.h"
#include "../../windowing/glfw_vk_window.h"
#include "../meshes/vertex_attributes.h"

namespace nova {
    vk_shader_program::vk_shader_program(const shader_definition &source, const material_state& material, const vk::RenderPass renderpass, vk::PipelineCache pipeline_cache, vk::Device device, std::shared_ptr<shader_resource_manager> shader_resources, glm::ivec2& window_size)
            : name(source.name), device(device), filter(source.filter_expression), shader_resources(shader_resources) {

        LOG(TRACE) << "Creating shader with filter expression " << source.filter_expression;
        create_shader_module(source.vertex_source, vk::ShaderStageFlagBits::eVertex);
        LOG(TRACE) << "Created vertex shader";
        create_shader_module(source.fragment_source, vk::ShaderStageFlagBits::eFragment);
        LOG(TRACE) << "Created fragment shader";

        create_pipeline(renderpass, material, pipeline_cache, window_size);
        LOG(TRACE) << "Created pipeline";
    }

    vk_shader_program::vk_shader_program(vk_shader_program &&other) noexcept :
            name(std::move(other.name)),
            pipeline(other.pipeline),
            vertex_module(other.vertex_module), fragment_module(other.fragment_module),
            geometry_module(std::move(other.geometry_module)),
            tessellation_evaluation_module(std::move(other.tessellation_evaluation_module)),
            tessellation_control_module(std::move(other.tessellation_control_module)),
            filter(std::move(other.filter)),
            shader_resources(other.shader_resources) {

        LOG(WARNING) << "Moving shader " << name;

        other.name = "";
        other.filter = "";
        other.pipeline = vk::Pipeline{};
        other.vertex_module = vk::ShaderModule{};
        other.fragment_module = vk::ShaderModule{};
        other.geometry_module = std::experimental::optional<vk::ShaderModule>{};
        other.tessellation_control_module = std::experimental::optional<vk::ShaderModule>{};
        other.tessellation_evaluation_module = std::experimental::optional<vk::ShaderModule>{};
    }

    void vk_shader_program::create_pipeline(vk::RenderPass pass, const material_state &material, vk::PipelineCache cache, glm::ivec2& window_size) {
        // Creates a pipeline out of compiled shaders
        auto states_vec = material.states.value_or(std::vector<state_enum>{});
        const auto& states_end = states_vec.end();

        vk::GraphicsPipelineCreateInfo pipeline_create_info = {};

        /**
         * Shader stages
         */

        std::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos;

        vk::PipelineShaderStageCreateInfo vertex_create_info = {};
        vertex_create_info.stage = vk::ShaderStageFlagBits::eVertex;
        vertex_create_info.module = vertex_module;
        vertex_create_info.pName = "main";
        stage_create_infos.push_back(vertex_create_info);

        vk::PipelineShaderStageCreateInfo fragment_create_info = {};
        fragment_create_info.stage = vk::ShaderStageFlagBits::eFragment;
        fragment_create_info.module = fragment_module;
        fragment_create_info.pName = "main";
        stage_create_infos.push_back(fragment_create_info);

        pipeline_create_info.stageCount = static_cast<uint32_t>(stage_create_infos.size());
        pipeline_create_info.pStages = stage_create_infos.data();

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

        // Location in shader, buffer binding, data format, offset in buffer
        attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, 0);
        attribute_descriptions.emplace_back(1, 0, vk::Format::eR32G32Sfloat,    12);
        attribute_descriptions.emplace_back(2, 0, vk::Format::eR32G32B32A32Sfloat, 20);

        /*attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, 0);     // Position
        attribute_descriptions.emplace_back(1, 0, vk::Format::eR8G8B8A8Unorm,   12);    // Color
        attribute_descriptions.emplace_back(2, 0, vk::Format::eR32G32Sfloat,    16);    // UV
        attribute_descriptions.emplace_back(3, 0, vk::Format::eR16G16Unorm,     24);    // Lightmap UV
        attribute_descriptions.emplace_back(4, 0, vk::Format::eR32G32B32Sfloat, 32);    // Normal
        attribute_descriptions.emplace_back(5, 0, vk::Format::eR32G32B32Sfloat, 48);    // Tangent*/

        // Binding, stride, input rate
        binding_descriptions.emplace_back(0, 36, vk::VertexInputRate::eVertex);         // Position
        binding_descriptions.emplace_back(1, 36, vk::VertexInputRate::eVertex);         // Color
        binding_descriptions.emplace_back(2, 36, vk::VertexInputRate::eVertex);         // UV
        //binding_descriptions.emplace_back(3, 56, vk::VertexInputRate::eVertex);         // Lightmap
        //binding_descriptions.emplace_back(4, 56, vk::VertexInputRate::eVertex);         // Normal
        //binding_descriptions.emplace_back(5, 56, vk::VertexInputRate::eVertex);         // Tangent

        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();

        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = binding_descriptions.data();

        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;

        /**
         * Pipeline input assembly
         */

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};

        input_assembly_create_info.topology = material.primitive_mode.value_or(vk::PrimitiveTopology::eTriangleList);

        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;

        /**
         * Tessellation state
         */

        pipeline_create_info.pTessellationState = nullptr;

        /**
         * \brief Viewport state
         */

        vk::Viewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = window_size.x;
        viewport.height = window_size.y;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = vk::Extent2D{material.output_width.value_or(640), material.output_height.value_or(480)};

        vk::PipelineViewportStateCreateInfo viewport_create_info = {};
        viewport_create_info.scissorCount = 1;
        viewport_create_info.pScissors = &scissor;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pViewports = &viewport;

        pipeline_create_info.pViewportState = &viewport_create_info;

        /**
         * Renderpass
         */

        pipeline_create_info.renderPass = pass;

        /**
         * Rasterization state
         */

        vk::PipelineRasterizationStateCreateInfo raster_create_info = {};
        raster_create_info.polygonMode = vk::PolygonMode::eFill;
        raster_create_info.cullMode == vk::CullModeFlagBits::eBack;
        raster_create_info.frontFace = vk::FrontFace::eCounterClockwise;
        raster_create_info.lineWidth = 1;

        if(material.depth_bias) {
            raster_create_info.depthBiasEnable = static_cast<vk::Bool32>(true);
            raster_create_info.depthBiasConstantFactor = *material.depth_bias;

            if(material.slope_scaled_depth_bias) {
                raster_create_info.depthBiasSlopeFactor = *material.slope_scaled_depth_bias;
            }
        }

        pipeline_create_info.pRasterizationState = &raster_create_info;

        /**
         * Multisample state
         *
         * While Nova supports MSAA, it won't be useful on shaderpacks that implement deferred rendering and is thus
         * off by default
         */

        vk::PipelineMultisampleStateCreateInfo multisample_create_info = {};

        pipeline_create_info.pMultisampleState = &multisample_create_info;

        /**
         * Depth and stencil state
         */

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.depthTestEnable = static_cast<vk::Bool32>((std::find(states_vec.begin(), states_vec.end(), state_enum::DisableDepthTest) == states_end));
        depth_stencil_create_info.depthWriteEnable = static_cast<vk::Bool32>((std::find(states_vec.begin(), states_vec.end(), state_enum::DisableDepthWrite) == states_end));
        depth_stencil_create_info.depthCompareOp = material.depth_func.value_or(vk::CompareOp::eLess);

        depth_stencil_create_info.stencilTestEnable = static_cast<vk::Bool32>(std::find(states_vec.begin(), states_vec.end(), state_enum::EnableStencilTest) != states_end);
        if(material.back_face) {
            depth_stencil_create_info.back = material.back_face.value().to_vk_stencil_op_state();
        }
        if(material.front_face) {
            depth_stencil_create_info.front = material.front_face.value().to_vk_stencil_op_state();
        }
        depth_stencil_create_info.minDepthBounds = 0;
        depth_stencil_create_info.maxDepthBounds = 1;

        pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;

        /**
         * Color blend state
         */

        vk::PipelineColorBlendStateCreateInfo blend_create_info = {};
        blend_create_info.logicOpEnable = static_cast<vk::Bool32>(false);   // Not 100% how to use this so imma just disable it

        auto attachmentBlendStates = std::vector<vk::PipelineColorBlendAttachmentState>{};
        attachmentBlendStates.resize(material.outputs.value_or(std::vector<output_info>{}).size());
        for(const auto& output : *material.outputs) {
            attachmentBlendStates[output.index].blendEnable = static_cast<vk::Bool32>(output.blending);
            attachmentBlendStates[output.index].srcColorBlendFactor = material.source_blend_factor.value_or(vk::BlendFactor::eSrcAlpha);
            attachmentBlendStates[output.index].dstColorBlendFactor = material.destination_blend_factor.value_or(vk::BlendFactor::eOneMinusSrcAlpha);
            attachmentBlendStates[output.index].colorBlendOp = vk::BlendOp::eAdd;
            attachmentBlendStates[output.index].srcAlphaBlendFactor = material.source_blend_factor.value_or(vk::BlendFactor::eSrcAlpha);
            attachmentBlendStates[output.index].dstAlphaBlendFactor = material.destination_blend_factor.value_or(vk::BlendFactor::eOneMinusSrcAlpha);
            attachmentBlendStates[output.index].alphaBlendOp = vk::BlendOp::eAdd;
            attachmentBlendStates[output.index].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        }
        blend_create_info.attachmentCount = static_cast<uint32_t>(attachmentBlendStates.size());
        blend_create_info.pAttachments = attachmentBlendStates.data();

        // Ignoring blend constants here as well - they probably won't be used and I don't want to make the Nova
        // material struct too huge

        pipeline_create_info.pColorBlendState = &blend_create_info;

        /**
         * Descriptor sets
         */

        pipeline_create_info.layout = shader_resources->get_layout_for_pass(material.pass.value_or(pass_enum::Gbuffer));

        // TODO: Handle dynamic state

        pipeline = device.createGraphicsPipeline(cache, pipeline_create_info);
        LOG(INFO) << "Created pipeline " << (VkPipeline)pipeline << " for shader " << name;
    }

    vk_shader_program::~vk_shader_program() {
        if(vertex_module) {
            device.destroyShaderModule(vertex_module);
        }

        if(fragment_module) {
            device.destroyShaderModule(fragment_module);
        }

        if(geometry_module) {
            device.destroyShaderModule(*geometry_module);
        }
        if(tessellation_evaluation_module) {
            device.destroyShaderModule(*tessellation_evaluation_module);
        }
        if(tessellation_control_module) {
            device.destroyShaderModule(*tessellation_control_module);
        }

        if(pipeline) {
            device.destroyPipeline(pipeline);
        }
    }

    void vk_shader_program::create_shader_module(const std::vector<uint32_t> &shader_source, vk::ShaderStageFlags flags) {
        vk::ShaderModuleCreateInfo create_info = {};
        create_info.codeSize = shader_source.size() * sizeof(uint32_t);
        create_info.pCode = shader_source.data();

        auto module = device.createShaderModule(create_info);

        if(flags == vk::ShaderStageFlagBits::eVertex) {
            vertex_module = module;
        } else if(flags == vk::ShaderStageFlagBits::eFragment) {
            fragment_module = module;
        }
    }

    std::string & vk_shader_program::get_filter() noexcept {
        return filter;
    }

    std::string &vk_shader_program::get_name() noexcept {
        return name;
    }

    vk::Pipeline vk_shader_program::get_pipeline() noexcept {
        return pipeline;
    }





    vk::ShaderModule create_shader_module(const shader_file& source, vk::ShaderStageFlags stages);

    vk::Pipeline make_pipeline(const pipeline& pipeline_create_info, const vk::RenderPass renderpass, const vk::Device device);

    std::unordered_map<std::string, std::vector<vk::Pipeline>> make_pipelines(const shaderpack_data& shaderpack,
                                                                              const std::unordered_map<std::string, std::pair<vk::RenderPass, vk::Framebuffer>>& renderpasses_by_pass,
                                                                              std::shared_ptr<render_context> context) {

        for(const auto& pipelines : shaderpack.pipelines_by_pass) {
            const auto& renderpass = renderpasses_by_pass[pipelines.first].first;
            for(const auto& pipeline_create_info : pipelines.second) {
                make_pipeline(pipeline_create_info, renderpass, context->device);
            }
        }
    }

    vk::Pipeline make_pipeline(const pipeline& pipeline_create_info, const vk::RenderPass renderpass, const vk::Device device) {
        // Creates a pipeline out of compiled shaders
        auto states_vec = pipeline_create_info.states.value_or(std::vector<state_enum>{});
        const auto& states_end = states_vec.end();

        vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info = {};

        /**
         * Shader stages
         */

        auto vertex_module = create_shader_module(pipeline_create_info.shader_sources.vertex_source, vk::ShaderStageFlagBits::eVertex);
        auto fragment_module = create_shader_module(pipeline_create_info.shader_sources.fragment_source, vk::ShaderStageFlagBits::eFragment);

        std::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos;

        vk::PipelineShaderStageCreateInfo vertex_create_info = {};
        vertex_create_info.stage = vk::ShaderStageFlagBits::eVertex;
        vertex_create_info.module = vertex_module;
        vertex_create_info.pName = "main";
        stage_create_infos.push_back(vertex_create_info);

        vk::PipelineShaderStageCreateInfo fragment_create_info = {};
        fragment_create_info.stage = vk::ShaderStageFlagBits::eFragment;
        fragment_create_info.module = fragment_module;
        fragment_create_info.pName = "main";
        stage_create_infos.push_back(fragment_create_info);

        graphics_pipeline_create_info.stageCount = static_cast<uint32_t>(stage_create_infos.size());
        graphics_pipeline_create_info.pStages = stage_create_infos.data();

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

            return vk::Pipeline();
        }

        uint32_t total_vertex_size = get_total_vertex_size();
        int32_t cur_binding = -1;
        for(const auto& vertex_field : pipeline_create_info.vertex_fields.value()) {
            cur_binding++;
            if(vertex_field == vertex_field_enum::Empty) {
                continue;
            }
            const auto& attribute = all_vertex_attributes[vertex_field.to_string()];
            attribute_descriptions.emplace_back(cur_binding, attribute.format, attribute.offset);
            binding_descriptions.emplace_back(cur_binding, total_vertex_size, vk::VertexInputRate::eVertex);
        }

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
        viewport.width = window_size.x;
        viewport.height = window_size.y;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = vk::Extent2D{pipeline_create_info.output_width.value_or(640), pipeline_create_info.output_height.value_or(480)};

        vk::PipelineViewportStateCreateInfo viewport_create_info = {};
        viewport_create_info.scissorCount = 1;
        viewport_create_info.pScissors = &scissor;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pViewports = &viewport;

        graphics_pipeline_create_info.pViewportState = &viewport_create_info;

        /**
         * Renderpass
         */

        graphics_pipeline_create_info.renderPass = renderpass;

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
        depth_stencil_create_info.depthCompareOp = pipeline_create_info.depth_func.value_or(vk::CompareOp::eLess);

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
        attachmentBlendStates.resize(pipeline_create_info.outputs.value_or(std::vector<output_info>{}).size());
        for(const auto& output : *pipeline_create_info.outputs) {
            attachmentBlendStates[output.index].blendEnable = static_cast<vk::Bool32>(output.blending);
            attachmentBlendStates[output.index].srcColorBlendFactor = pipeline_create_info.source_blend_factor.value_or(vk::BlendFactor::eSrcAlpha);
            attachmentBlendStates[output.index].dstColorBlendFactor = pipeline_create_info.destination_blend_factor.value_or(vk::BlendFactor::eOneMinusSrcAlpha);
            attachmentBlendStates[output.index].colorBlendOp = vk::BlendOp::eAdd;
            attachmentBlendStates[output.index].srcAlphaBlendFactor = pipeline_create_info.source_blend_factor.value_or(vk::BlendFactor::eSrcAlpha);
            attachmentBlendStates[output.index].dstAlphaBlendFactor = pipeline_create_info.destination_blend_factor.value_or(vk::BlendFactor::eOneMinusSrcAlpha);
            attachmentBlendStates[output.index].alphaBlendOp = vk::BlendOp::eAdd;
            attachmentBlendStates[output.index].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        }
        blend_create_info.attachmentCount = static_cast<uint32_t>(attachmentBlendStates.size());
        blend_create_info.pAttachments = attachmentBlendStates.data();

        // Ignoring blend constants here as well - they probably won't be used and I don't want to make the Nova
        // material struct too huge

        graphics_pipeline_create_info.pColorBlendState = &blend_create_info;

        /**
         * Descriptor sets
         */

        graphics_pipeline_create_info.layout = shader_resources->get_layout_for_pass(pipeline_create_info.pass.value_or(pass_enum::Gbuffer));

        // TODO: Handle dynamic state

        auto pipeline = context->device.createGraphicsPipeline(cache, pipeline_create_info);
        LOG(INFO) << "Created pipeline " << pipeline_create_info.name << " (VkPipeline " << (VkPipeline)pipeline << ")";

        return pipeline;
    }

    vk::ShaderModule create_shader_module(const shader_file& source, vk::ShaderStageFlags stages) {
        vk::ShaderModuleCreateInfo create_info = {};
        create_info.codeSize = shader_source.size() * sizeof(uint32_t);
        create_info.pCode = shader_source.data();

        auto module = device.createShaderModule(create_info);

        if(flags == vk::ShaderStageFlagBits::eVertex) {
            vertex_module = module;
        } else if(flags == vk::ShaderStageFlagBits::eFragment) {
            fragment_module = module;
        }
    }
}
