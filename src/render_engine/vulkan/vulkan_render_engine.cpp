//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <set>
#include <vector>
#include "../../util/logger.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "../../../3rdparty/SPIRV-Cross/spirv_glsl.hpp"
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "../../platform.hpp"
#include "../../util/utils.hpp"
#include "../dx12/win32_window.hpp"
#include "vulkan_type_converters.hpp"
#include "vulkan_utils.hpp"
#include "swapchain.hpp"

#ifdef NOVA_LINUX
#include <execinfo.h>
#include <cxxabi.h>

#endif

namespace nova {
    vulkan_render_engine::~vulkan_render_engine() {
        vkDeviceWaitIdle(device);
    }

    bool vulkan_render_engine::does_device_support_extensions(VkPhysicalDevice device) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

        std::set<std::string> required = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        for(const auto& extension : available) {
            required.erase(extension.extensionName);
        }

        return required.empty();
    }

    VkCommandPool vulkan_render_engine::get_command_buffer_pool_for_current_thread(uint32_t queue_index) {
        const std::size_t cur_thread_idx = scheduler->get_current_thread_idx();
        return command_pools_by_thread_idx.at(cur_thread_idx).at(queue_index);
    }

    VkDescriptorPool vulkan_render_engine::get_descriptor_pool_for_current_thread() {
        const std::size_t cur_thread_idx = scheduler->get_current_thread_idx();
        return descriptor_pools_by_thread_idx.at(cur_thread_idx);
    }

    vk_buffer vulkan_render_engine::get_or_allocate_mesh_staging_buffer(const uint32_t needed_size) {
        std::lock_guard l(mesh_staging_buffers_mutex);

        if(!available_mesh_staging_buffers.empty()) {
            // Try to find a buffer that's big enough
            uint32_t potential_staging_buffer_idx = std::numeric_limits<uint32_t>::max();

            for(uint32_t i = 0; i < available_mesh_staging_buffers.size(); i++) {
                if(available_mesh_staging_buffers[i].alloc_info.size >= needed_size && available_mesh_staging_buffers[i].alloc_info.size > available_mesh_staging_buffers[potential_staging_buffer_idx].alloc_info.size) {
                    potential_staging_buffer_idx = i;
                }
            }

            if(potential_staging_buffer_idx < available_mesh_staging_buffers.size()) {
                const vk_buffer staging_buffer = available_mesh_staging_buffers[potential_staging_buffer_idx];
                available_mesh_staging_buffers.erase(available_mesh_staging_buffers.begin() + potential_staging_buffer_idx);
                return staging_buffer;
            }
        }

        vk_buffer new_buffer = {};

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.size = needed_size;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &copy_family_index;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator, &buffer_create_info, &allocation_create_info, &new_buffer.buffer, &new_buffer.allocation, &new_buffer.alloc_info), render_engine_rendering_exception);

        return new_buffer;
    }

    void vulkan_render_engine::free_mesh_staging_buffer(const vk_buffer& buffer) {
        std::lock_guard l(mesh_staging_buffers_mutex);

        available_mesh_staging_buffers.push_back(buffer);
    }

    mesh_id_t vulkan_render_engine::add_mesh(const mesh_data& input_mesh) {
        const uint32_t vertex_size = static_cast<uint32_t>(input_mesh.vertex_data.size()) * sizeof(full_vertex);
        const uint32_t index_size = static_cast<uint32_t>(input_mesh.indices.size()) * sizeof(uint32_t);

        // TODO: Make the extra memory allocation configurable
        const uint32_t total_memory_needed = static_cast<uint32_t>(std::round((vertex_size + index_size) * 1.1)); // Extra size so chunks can grow

        vk_buffer staging_buffer = get_or_allocate_mesh_staging_buffer(total_memory_needed);
        std::memcpy(staging_buffer.alloc_info.pMappedData, &input_mesh.vertex_data[0], vertex_size);
        std::memcpy(reinterpret_cast<uint8_t*>(staging_buffer.alloc_info.pMappedData) + vertex_size, &input_mesh.indices[0], index_size);

        const uint32_t mesh_id = next_mesh_id.fetch_add(1);

        std::lock_guard l(mesh_upload_queue_mutex);
        mesh_upload_queue.push(mesh_staging_buffer_upload_command{staging_buffer, mesh_id, vertex_size, vertex_size + index_size});
        
        return mesh_id;
    }

    void vulkan_render_engine::delete_mesh(uint32_t mesh_id) {
        const vk_mesh mesh = meshes.at(mesh_id);
        meshes.erase(mesh_id);

        mesh_memory->free(mesh.memory);
    }

    bool vk_resource_binding::operator==(const vk_resource_binding& other) const {
        return other.set == set && other.binding == binding && other.descriptorCount == descriptorCount && other.descriptorType == descriptorType;
    }

    bool vk_resource_binding::operator!=(const vk_resource_binding& other) const {
        return !(*this == other);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* /* pUserData */) {
        std::string type = "General";
        if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            type = "Validation";

        } else if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
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
                ss << vulkan::to_string(pCallbackData->pObjects[i].objectType);
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

        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            NOVA_LOG(ERROR) << "[" << type << "] " << msg;

        } else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            // Warnings may hint at unexpected / non-spec API usage
            NOVA_LOG(WARN) << "[" << type << "] " << msg;

        } else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            // Informal messages that may become handy during debugging
            NOVA_LOG(INFO) << "[" << type << "] " << msg;

        } else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            NOVA_LOG(DEBUG) << "[" << type << "] " << msg;

        } else {
            // Catch-all to be super sure
            NOVA_LOG(INFO) << "[" << type << "]" << msg;
        }

#ifdef NOVA_LINUX
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            nova_backtrace();
        }
#endif
        return VK_FALSE;
    }

    std::shared_ptr<iwindow> vulkan_render_engine::get_window() const {
        return window;
    }

    std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> vulkan_render_engine::to_vk_attachment_info(std::vector<std::string>& attachment_names) {
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

    void vulkan_render_engine::destroy_render_passes() {
        for(const auto& [pass_name, pass] : render_passes) {
            (void) pass_name;
            vkDestroyRenderPass(device, pass.pass, nullptr);
        }

        render_passes.clear();
        render_passes_by_order.clear();
    }
} // namespace nova
