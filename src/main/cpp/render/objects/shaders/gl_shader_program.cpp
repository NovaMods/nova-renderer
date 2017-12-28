/*!
 * \author David
 * \date 17-May-16.
 */

#include <cstdlib>
#include <algorithm>

#include <easylogging++.h>
#include "gl_shader_program.h"
#include "../../vulkan/render_context.h"

namespace nova {
    gl_shader_program::gl_shader_program(const shader_definition &source, const material_state& material, const vk::RenderPass renderpass, vk::PipelineCache pipeline_cache) : name(source.name) {
        device = render_context::instance.device;
        LOG(TRACE) << "Creating shader with filter expression " << source.filter_expression;
        filter = source.filter_expression;
        LOG(TRACE) << "Created filter expression " << filter;
        create_shader_module(source.vertex_source, vk::ShaderStageFlagBits::eVertex);
        LOG(TRACE) << "Creatd vertex shader";
        create_shader_module(source.fragment_source, vk::ShaderStageFlagBits::eFragment);
        LOG(TRACE) << "Created fragment shader";

        create_pipeline(renderpass, material, pipeline_cache);
        LOG(TRACE) << "Created pipeline";
    }

    gl_shader_program::gl_shader_program(gl_shader_program &&other) noexcept :
            name(std::move(other.name)), filter(std::move(other.filter)) {
    }

    void
    gl_shader_program::create_pipeline(vk::RenderPass pass, const material_state &material, vk::PipelineCache cache) {
        // Creates a pipeline out of compiled shaders
        const auto& states_end = (*material.states).end();
        const auto& states = *material.states;

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

        // The vertex data is known by Nova. It just is. Each shader has inputs for all the vertex data because honestly
        // doing it differently is super hard. This will waste some VRAM but the number of vertices per chunk and
        // number of chunks will present a much easier win, especially since chunks are the big stuff and they will
        // always have all the vertex attributes
        vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};

        std::vector<vk::VertexInputBindingDescription> binding_descriptions;
        std::vector<vk::VertexInputAttributeDescription> attribute_descriptions;

        // Location in shader, buffer binding, data format, offset in buffer
        attribute_descriptions.emplace_back(0, 0, vk::Format::eR32G32B32Sfloat, 0);     // Position
        attribute_descriptions.emplace_back(1, 0, vk::Format::eR8G8B8A8Unorm,   12);    // Color
        attribute_descriptions.emplace_back(2, 0, vk::Format::eR32G32Sfloat,    16);    // UV
        attribute_descriptions.emplace_back(3, 0, vk::Format::eR16G16Unorm,     24);    // Lightmap UV
        attribute_descriptions.emplace_back(4, 0, vk::Format::eR32G32B32Sfloat, 32);    // Normal
        attribute_descriptions.emplace_back(5, 0, vk::Format::eR32G32B32Sfloat, 48);    // Tangent

        // Binding, stride, input rate
        binding_descriptions.emplace_back(0, 56, vk::VertexInputRate::eVertex);         // Position
        binding_descriptions.emplace_back(1, 56, vk::VertexInputRate::eVertex);         // Color
        binding_descriptions.emplace_back(2, 56, vk::VertexInputRate::eVertex);         // UV
        binding_descriptions.emplace_back(3, 56, vk::VertexInputRate::eVertex);         // Lightmap
        binding_descriptions.emplace_back(4, 56, vk::VertexInputRate::eVertex);         // Normal
        binding_descriptions.emplace_back(5, 56, vk::VertexInputRate::eVertex);         // Tangent

        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = attribute_descriptions.data();

        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = binding_descriptions.data();

        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;

        /**
         * Pipeline input assembly
         */

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
        input_assembly_create_info.topology = *material.primitive_mode;

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
        viewport.width = *material.output_width;
        viewport.height = *material.output_height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;

        vk::Rect2D scissor = {};
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = vk::Extent2D{*material.output_width, *material.output_height};

        vk::PipelineViewportStateCreateInfo viewport_create_info = {};
        viewport_create_info.scissorCount = 1;
        viewport_create_info.pScissors = &scissor;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pViewports = &viewport;

        pipeline_create_info.pViewportState = &viewport_create_info;

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

        if(material.msaa_support) {
            if(*material.msaa_support != msaa_support_enum::none) {
                vk::PipelineMultisampleStateCreateInfo multisample_create_info = {};

                pipeline_create_info.pMultisampleState = &multisample_create_info;
            }
        }

        /**
         * Depth and stencil state
         */

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.depthTestEnable = static_cast<vk::Bool32>((std::find(states.begin(), states.end(), state_enum::disable_depth_test) == states_end));
        depth_stencil_create_info.depthWriteEnable = static_cast<vk::Bool32>((std::find(states.begin(), states.end(), state_enum::disable_depth_write) == states_end));
        depth_stencil_create_info.depthCompareOp = *material.depth_func;

        depth_stencil_create_info.stencilTestEnable = static_cast<vk::Bool32>(std::find(states.begin(), states.end(), state_enum::enable_stencil_test) != states_end);
        depth_stencil_create_info.back = (*material.back_face).to_vk_stencil_op_state();
        depth_stencil_create_info.front = (*material.front_face).to_vk_stencil_op_state();
        depth_stencil_create_info.minDepthBounds = 0;
        depth_stencil_create_info.maxDepthBounds = 1;

        pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;

        /**
         * Color blend state
         */

        vk::PipelineColorBlendStateCreateInfo blend_create_info = {};
        blend_create_info.logicOpEnable = static_cast<vk::Bool32>(false);   // Not 100% how to use this so imma just disable it

        auto attachmentBlendStates = std::vector<vk::PipelineColorBlendAttachmentState>{};
        attachmentBlendStates.resize(8);    // TODO: Change for the total number of framebuffers (once we detect that)
        for(const auto& output : *material.outputs) {
            attachmentBlendStates[output.index].blendEnable = static_cast<vk::Bool32>(output.blending);
            attachmentBlendStates[output.index].srcColorBlendFactor = *material.source_blend_factor;
            attachmentBlendStates[output.index].dstColorBlendFactor = *material.destination_blend_factor;
            attachmentBlendStates[output.index].colorBlendOp = vk::BlendOp::eAdd;
            attachmentBlendStates[output.index].srcAlphaBlendFactor = *material.source_blend_factor;
            attachmentBlendStates[output.index].dstAlphaBlendFactor = *material.destination_blend_factor;
            attachmentBlendStates[output.index].alphaBlendOp = vk::BlendOp::eAdd;
            attachmentBlendStates[output.index].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        }
        blend_create_info.attachmentCount = static_cast<uint32_t>(attachmentBlendStates.size());
        blend_create_info.pAttachments = attachmentBlendStates.data();

        // Ignoring blend constants here as well - they probably won't be used and I don't want to make the Nova
        // material struct too huge

        pipeline_create_info.pColorBlendState = &blend_create_info;

        // TODO: Handle dynamic state

        pipeline = device.createGraphicsPipeline(cache, pipeline_create_info);
    }

    gl_shader_program::~gl_shader_program() {
        device.destroyShaderModule(vertex_module);
        device.destroyShaderModule(fragment_module);

        if(geometry_module) {
            device.destroyShaderModule(*geometry_module);
        }
        if(tessellation_evaluation_module) {
            device.destroyShaderModule(*tessellation_evaluation_module);
        }
        if(tessellation_control_module) {
            device.destroyShaderModule(*tessellation_control_module);
        }
    }

    void gl_shader_program::create_shader_module(const std::vector<uint32_t> &shader_source, vk::ShaderStageFlags flags) {
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

    std::string & gl_shader_program::get_filter() noexcept {
        return filter;
    }

    std::string &gl_shader_program::get_name() noexcept {
        return name;
    }

}
