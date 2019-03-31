//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <nova_renderer/util/platform.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <vector>
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "../../util/logger.hpp"
#include "../dx12/win32_window.hpp"
#include "vulkan_utils.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "vulkan_command_list.hpp"

#ifdef NOVA_LINUX
#include <cxxabi.h>
#include <execinfo.h>

#endif

namespace nova::renderer {
    vulkan_render_engine::~vulkan_render_engine() { vkDeviceWaitIdle(device); }

    std::shared_ptr<iwindow> vulkan_render_engine::get_window() const { return window; }

    command_list* vulkan_render_engine::allocate_command_list(uint32_t thread_idx, queue_type needed_queue_type, command_list::level command_list_type) {
        uint32_t queue_family_idx = graphics_family_index;
        switch (needed_queue_type) {
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
        switch (command_list_type) {
        case command_list::level::PRIMARY:
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            break;

        case command_list::level::SECONDARY:
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            break;
        }

        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(device, &alloc_info, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if(command_list_type == command_list::level::SECONDARY) {
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        }

        vkBeginCommandBuffer(cmds, &begin_info);

        return new vulkan_command_list(cmds);
    }

    VkCommandPool vulkan_render_engine::get_command_buffer_pool_for_current_thread(uint32_t thread_idx, uint32_t queue_index) {
        return command_pools_by_thread_idx.at(thread_idx).at(queue_index);
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
