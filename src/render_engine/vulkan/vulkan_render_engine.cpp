#include "vulkan_render_engine.hpp"

#include <vector>

#define VMA_IMPLEMENTATION // Recheck if good to be here
#include <vk_mem_alloc.h>
#include "vulkan_command_list.hpp"

#include "vulkan_utils.hpp"

#ifdef NOVA_LINUX
#include <cxxabi.h>
#include <execinfo.h>
#endif

#include <spirv_cross/spirv_glsl.hpp>

#include "nova_renderer/util/platform.hpp"

#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "../../util/logger.hpp"

// TODO: Move windowing out of render engine folders
//       its confusing to have a DX12 include in the Vulkan renderer and breaks encapsulation
#ifdef NOVA_WINDOWS
#include "../dx12/win32_window.hpp"
#endif

#include "vulkan_utils.hpp"

namespace nova::renderer {
    vulkan_render_engine::~vulkan_render_engine() { vkDeviceWaitIdle(device); }

    std::shared_ptr<window> vulkan_render_engine::get_window() const { return window; }

    pipeline_t* vulkan_render_engine::create_pipeline(const pipeline_create_info_t& data) {
        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();

        NOVA_LOG(TRACE) << "Creating a VkPipeline for pipeline " << data.name;
        vk_pipeline nova_pipeline;
        nova_pipeline.data = data;

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        std::unordered_map<VkShaderStageFlags, VkShaderModule> shader_modules;

        NOVA_LOG(TRACE) << "Compiling vertex module";
        shader_modules[VK_SHADER_STAGE_VERTEX_BIT] = create_shader_module(data.vertex_shader.source);
        get_shader_module_descriptors(data.vertex_shader.source, VK_SHADER_STAGE_VERTEX_BIT, nova_pipeline.bindings);

        if(data.geometry_shader) {
            NOVA_LOG(TRACE) << "Compiling geometry module";
            shader_modules[VK_SHADER_STAGE_GEOMETRY_BIT] = create_shader_module(data.geometry_shader->source);
            get_shader_module_descriptors(data.geometry_shader->source, VK_SHADER_STAGE_GEOMETRY_BIT, nova_pipeline.bindings);
        }

        if(data.tessellation_control_shader) {
            NOVA_LOG(TRACE) << "Compiling tessellation_control module";
            shader_modules[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = create_shader_module(data.tessellation_control_shader->source);
            get_shader_module_descriptors(data.tessellation_control_shader->source,
                                          VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                          nova_pipeline.bindings);
        }

        if(data.tessellation_evaluation_shader) {
            NOVA_LOG(TRACE) << "Compiling tessellation_evaluation module";
            shader_modules[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = create_shader_module(data.tessellation_evaluation_shader->source);
            get_shader_module_descriptors(data.tessellation_evaluation_shader->source,
                                          VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                          nova_pipeline.bindings);
        }

        if(data.fragment_shader) {
            NOVA_LOG(TRACE) << "Compiling fragment module";
            shader_modules[VK_SHADER_STAGE_FRAGMENT_BIT] = create_shader_module(data.fragment_shader->source);
            get_shader_module_descriptors(data.fragment_shader->source, VK_SHADER_STAGE_FRAGMENT_BIT, nova_pipeline.bindings);
        }

        nova_pipeline.layouts = create_descriptor_set_layouts(nova_pipeline.bindings);

        VkPipelineLayoutCreateInfo pipeline_layout_create_info;
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.flags = 0;
        pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(nova_pipeline.layouts.size());
        pipeline_layout_create_info.pSetLayouts = nova_pipeline.layouts.data();
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        VkPipelineLayout layout;
        NOVA_CHECK_RESULT(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &layout));
        nova_pipeline.layout = layout;

        for(const auto& [stage, shader_module] : shader_modules) {
            VkPipelineShaderStageCreateInfo shader_stage_create_info;
            shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shader_stage_create_info.pNext = nullptr;
            shader_stage_create_info.flags = 0;
            shader_stage_create_info.stage = static_cast<VkShaderStageFlagBits>(stage);
            shader_stage_create_info.module = shader_module;
            shader_stage_create_info.pName = "main";
            shader_stage_create_info.pSpecializationInfo = nullptr;

            shader_stages.push_back(shader_stage_create_info);
        }

        const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions = get_vertex_input_binding_descriptions();
        const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions = get_vertex_input_attribute_descriptions();

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.pNext = nullptr;
        vertex_input_state_create_info.flags = 0;
        vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_binding_descriptions.size());
        vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
        vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.pNext = nullptr;
        input_assembly_create_info.flags = 0;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
        switch(data.primitive_mode) {
            case primitive_topology_enum::Triangles:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                break;
            case primitive_topology_enum::Lines:
                input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                break;
        }

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(swapchain_extent.width);
        viewport.height = static_cast<float>(swapchain_extent.height);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;

        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = swapchain_extent;

        VkPipelineViewportStateCreateInfo viewport_state_create_info;
        viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.pNext = nullptr;
        viewport_state_create_info.flags = 0;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.pViewports = &viewport;
        viewport_state_create_info.scissorCount = 1;
        viewport_state_create_info.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer_create_info;
        rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_create_info.pNext = nullptr;
        rasterizer_create_info.flags = 0;
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer_create_info.lineWidth = 1.0F;
        rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer_create_info.depthBiasEnable = VK_TRUE;
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.depthBiasConstantFactor = data.depth_bias;
        rasterizer_create_info.depthBiasClamp = 0.0F;
        rasterizer_create_info.depthBiasSlopeFactor = data.slope_scaled_depth_bias;

        VkPipelineMultisampleStateCreateInfo multisample_create_info;
        multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_create_info.pNext = nullptr;
        multisample_create_info.flags = 0;
        multisample_create_info.sampleShadingEnable = VK_FALSE;
        multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_create_info.minSampleShading = 1.0F;
        multisample_create_info.pSampleMask = nullptr;
        multisample_create_info.alphaToCoverageEnable = VK_FALSE;
        multisample_create_info.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
        depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_create_info.depthTestEnable = static_cast<VkBool32>(
            std::find(data.states.begin(), data.states.end(), state_enum::DisableDepthTest) == data.states.end());
        depth_stencil_create_info.depthWriteEnable = static_cast<VkBool32>(
            std::find(data.states.begin(), data.states.end(), state_enum::DisableDepthWrite) == data.states.end());
        depth_stencil_create_info.depthCompareOp = vulkan::to_compare_op(data.depth_func);
        depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_create_info.stencilTestEnable = static_cast<VkBool32>(
            std::find(data.states.begin(), data.states.end(), state_enum::EnableStencilTest) != data.states.end());
        if(data.front_face) {
            depth_stencil_create_info.front.failOp = vulkan::to_stencil_op(data.front_face->fail_op);
            depth_stencil_create_info.front.passOp = vulkan::to_stencil_op(data.front_face->pass_op);
            depth_stencil_create_info.front.depthFailOp = vulkan::to_stencil_op(data.front_face->depth_fail_op);
            depth_stencil_create_info.front.compareOp = vulkan::to_compare_op(data.front_face->compare_op);
            depth_stencil_create_info.front.compareMask = data.front_face->compare_mask;
            depth_stencil_create_info.front.writeMask = data.front_face->write_mask;
        }
        if(data.back_face) {
            depth_stencil_create_info.back.failOp = vulkan::to_stencil_op(data.back_face->fail_op);
            depth_stencil_create_info.back.passOp = vulkan::to_stencil_op(data.back_face->pass_op);
            depth_stencil_create_info.back.depthFailOp = vulkan::to_stencil_op(data.back_face->depth_fail_op);
            depth_stencil_create_info.back.compareOp = vulkan::to_compare_op(data.back_face->compare_op);
            depth_stencil_create_info.back.compareMask = data.back_face->compare_mask;
            depth_stencil_create_info.back.writeMask = data.back_face->write_mask;
        }

        VkPipelineColorBlendAttachmentState color_blend_attachment;
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                                VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = vulkan::to_blend_factor(data.source_blend_factor);
        color_blend_attachment.dstColorBlendFactor = vulkan::to_blend_factor(data.destination_blend_factor);
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = vulkan::to_blend_factor(data.alpha_src);
        color_blend_attachment.dstAlphaBlendFactor = vulkan::to_blend_factor(data.alpha_dst);
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_create_info;
        color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_create_info.pNext = nullptr;
        color_blend_create_info.flags = 0;
        color_blend_create_info.logicOpEnable = VK_FALSE;
        color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_create_info.attachmentCount = 1;
        color_blend_create_info.pAttachments = &color_blend_attachment;
        color_blend_create_info.blendConstants[0] = 0.0F;
        color_blend_create_info.blendConstants[1] = 0.0F;
        color_blend_create_info.blendConstants[2] = 0.0F;
        color_blend_create_info.blendConstants[3] = 0.0F;

        VkGraphicsPipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_create_info.pNext = nullptr;
        pipeline_create_info.flags = 0;
        pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
        pipeline_create_info.pStages = shader_stages.data();
        pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
        pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
        pipeline_create_info.pViewportState = &viewport_state_create_info;
        pipeline_create_info.pRasterizationState = &rasterizer_create_info;
        pipeline_create_info.pMultisampleState = &multisample_create_info;
        pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
        pipeline_create_info.pColorBlendState = &color_blend_create_info;
        pipeline_create_info.pDynamicState = nullptr;
        pipeline_create_info.layout = layout;
        pipeline_create_info.renderPass = render_passes.at(data.pass).pass;
        pipeline_create_info.subpass = 0;
        pipeline_create_info.basePipelineIndex = -1;

        NOVA_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &nova_pipeline.pipeline));

        pipelines_by_renderpass[data.pass].push_back(nova_pipeline);

        if(settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(nova_pipeline.pipeline);
            object_name.pObjectName = data.name.c_str();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
            NOVA_LOG(INFO) << "Set pipeline " << nova_pipeline.pipeline << " to have name " << data.name;
        }
    }

    command_list_t* vulkan_render_engine::allocate_command_list(uint32_t thread_idx,
                                                                queue_type needed_queue_type,
                                                                command_list_t::level command_list_type) {
        uint32_t queue_family_idx = graphics_family_index;
        switch(needed_queue_type) {
            case queue_type::GRAPHICS:
                queue_family_idx = graphics_family_index;
                break;

            case queue_type::TRANSFER:
                queue_family_idx = transfer_family_index;
                break;

            case queue_type::ASYNC_COMPUTE:
                queue_family_idx = compute_family_index;
                break;
        }

        VkCommandPool pool = get_command_buffer_pool_for_current_thread(thread_idx, queue_family_idx);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = pool;
        switch(command_list_type) {
            case command_list_t::level::PRIMARY:
                alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                break;

            case command_list_t::level::SECONDARY:
                alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                break;
        }

        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(device, &alloc_info, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if(command_list_type == command_list_t::level::SECONDARY) {
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        }

        vkBeginCommandBuffer(cmds, &begin_info);

        return new vulkan_command_list(cmds);
    }

    VkCommandPool vulkan_render_engine::get_command_buffer_pool_for_current_thread(uint32_t thread_idx, uint32_t queue_index) {
        return command_pools_by_thread_idx.at(thread_idx).at(queue_index);
    }

    uint32_t vulkan_render_engine::get_queue_family_index(const queue_type queue) const {
        switch(queue) {
            case queue_type::GRAPHICS:
                return graphics_family_index;
            case queue_type::TRANSFER:
                return transfer_family_index;
            case queue_type::ASYNC_COMPUTE:
                return compute_family_index;
        }
    }

    VkDescriptorPool vulkan_render_engine::get_descriptor_pool_for_current_thread() { return descriptor_pools_by_thread_idx.at(0); }

    std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> vulkan_render_engine::to_vk_attachment_info(
        std::vector<std::string>& attachment_names) {
        std::vector<VkAttachmentDescription> attachment_descriptions;
        attachment_descriptions.reserve(attachment_names.size());

        std::vector<VkAttachmentReference> attachment_references;
        attachment_references.reserve(attachment_names.size());

        for(const std::string& name : attachment_names) {
            const vk_texture& tex = textures.at(name);

            VkAttachmentDescription color_attachment;
            color_attachment.flags = 0;
            color_attachment.format = to_vk_format(tex.data.format.pixel_format);
            color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment_descriptions.push_back(color_attachment);

            VkAttachmentReference color_attachment_reference;
            color_attachment_reference.attachment = static_cast<uint32_t>(attachment_references.size());
            color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment_references.push_back(color_attachment_reference);
        }

        return {attachment_descriptions, attachment_references};
    }

    VkFormat vulkan_render_engine::to_vk_format(const pixel_format_enum format) {
        switch(format) {
            case pixel_format_enum::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case pixel_format_enum::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case pixel_format_enum::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case pixel_format_enum::Depth:
                return VK_FORMAT_D32_SFLOAT;

            case pixel_format_enum::DepthStencil:
                return VK_FORMAT_D24_UNORM_S8_UINT;
        }

        return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    }

    void vulkan_render_engine::destroy_graphics_pipelines() {
        for(const auto& [renderpass_name, pipelines] : pipelines_by_renderpass) {
            (void) renderpass_name;
            for(const auto& pipeline : pipelines) {
                vkDestroyPipeline(device, pipeline.pipeline, nullptr);
            }
        }

        pipelines_by_renderpass.clear();
    }

    void vulkan_render_engine::destroy_dynamic_resources() {
        for(auto itr = std::begin(textures); itr != std::end(textures);) {
            const vk_texture& tex = itr->second;
            if(tex.is_dynamic) {
                vkDestroyImageView(device, tex.image_view, nullptr);
                vmaDestroyImage(vma_allocator, tex.image, tex.allocation);

                itr = textures.erase(itr);
            } else {
                ++itr;
            }
        }

        for(auto itr = std::begin(buffers); itr != std::end(buffers);) {
            const vk_buffer& buf = itr->second;
            if(buf.is_dynamic) {
                vmaDestroyBuffer(vma_allocator, buf.buffer, buf.allocation);

                itr = buffers.erase(itr);
            } else {
                ++itr;
            }
        }
    }

    void vulkan_render_engine::destroy_render_passes() {
        for(const auto& [pass_name, pass] : render_passes) {
            (void) pass_name;
            vkDestroyRenderPass(device, pass.pass, nullptr);
        }

        render_passes.clear();
        render_passes_by_order.clear();
    }

    bool vk_resource_binding::operator==(const vk_resource_binding& other) const {
        return other.set == set && other.binding == binding && other.descriptorCount == descriptorCount &&
               other.descriptorType == descriptorType;
    }

    bool vk_resource_binding::operator!=(const vk_resource_binding& other) const { return !(*this == other); }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* /* pUserData */) {
        std::string type = "General";
        if((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0U) {
            type = "Validation";
        } else if((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0U) {
            type = "Performance";
        }

        std::stringstream ss;
        ss << "[" << type << "]";
        if(pCallbackData->queueLabelCount != 0) {
            ss << " Queues: ";
            for(uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
                ss << pCallbackData->pQueueLabels[i].pLabelName;
                if(i != pCallbackData->queueLabelCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(pCallbackData->cmdBufLabelCount != 0) {
            ss << " Command Buffers: ";
            for(uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
                ss << pCallbackData->pCmdBufLabels[i].pLabelName;
                if(i != pCallbackData->cmdBufLabelCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(pCallbackData->objectCount != 0) {
            ss << " Objects: ";
            for(uint32_t i = 0; i < pCallbackData->objectCount; i++) {
                ss << to_string(pCallbackData->pObjects[i].objectType);
                if(pCallbackData->pObjects[i].pObjectName != nullptr) {
                    ss << pCallbackData->pObjects[i].pObjectName;
                }
                ss << " (" << pCallbackData->pObjects[i].objectHandle << ") ";
                if(i != pCallbackData->objectCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(pCallbackData->pMessage != nullptr) {
            ss << pCallbackData->pMessage;
        }

        const std::string msg = ss.str();

        if((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            NOVA_LOG(ERROR) << "[" << type << "] " << msg;

        } else if((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
            // Warnings may hint at unexpected / non-spec API usage
            NOVA_LOG(WARN) << "[" << type << "] " << msg;

        } else if(((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) &&
                  ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U)) { // No validation info!
            // Informal messages that may become handy during debugging
            NOVA_LOG(INFO) << "[" << type << "] " << msg;

        } else if((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            NOVA_LOG(DEBUG) << "[" << type << "] " << msg;

        } else if((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U) { // No validation info!
            // Catch-all to be super sure
            NOVA_LOG(INFO) << "[" << type << "]" << msg;
        }

#ifdef NOVA_LINUX
        if((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            nova_backtrace();
        }
#endif
        return VK_FALSE;
    }

} // namespace nova::renderer
