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

    void vulkan_render_engine::transition_dynamic_textures() {
        NOVA_LOG(TRACE) << "Transitioning dynamic textures to color attachment layouts";
        std::vector<VkImageMemoryBarrier> color_barriers;
        color_barriers.reserve(textures.size());

        std::vector<VkImageMemoryBarrier> depth_barriers;
        depth_barriers.reserve(textures.size());

        for(const auto& [_, texture] : textures) {
            (void) _;
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = texture.image;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            if(texture.format == VK_FORMAT_D24_UNORM_S8_UINT || texture.format == VK_FORMAT_D32_SFLOAT) {
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                depth_barriers.push_back(barrier);

            } else {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                color_barriers.push_back(barrier);
            }
        }

        const VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(device, &alloc_info, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmds, &begin_info);

        vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(color_barriers.size()), color_barriers.data());

        vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(depth_barriers.size()), depth_barriers.data());

        vkEndCommandBuffer(cmds);

        VkFence transition_done_fence;

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        vkCreateFence(device, &fence_create_info, nullptr, &transition_done_fence);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        vkQueueSubmit(graphics_queue, 1, &submit_info, transition_done_fence);
        vkWaitForFences(device, 1, &transition_done_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkFreeCommandBuffers(device, command_pool, 1, &cmds);

        dynamic_textures_need_to_transition = false;
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

    VkShaderModule vulkan_render_engine::create_shader_module(const std::vector<uint32_t>& spirv) const {
        VkShaderModuleCreateInfo shader_module_create_info;
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.flags = 0;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size() * 4;

        VkShaderModule module;
        NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module), render_engine_initialization_exception);

        return module;
    }

    void vulkan_render_engine::upload_new_mesh_parts() {
        if(mesh_upload_queue.empty()) {
            // Early out yay
            return;
        }

        VkCommandBuffer mesh_upload_cmds;

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandBufferCount = 1;
        alloc_info.commandPool = get_command_buffer_pool_for_current_thread(copy_family_index);
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        vkAllocateCommandBuffers(device, &alloc_info, &mesh_upload_cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(mesh_upload_cmds, &begin_info);

        // Ensure that all reads from this buffer have finished. I don't care about writes because the only
        // way two dudes would be writing to the same region of a megamesh at the same time was if there was a
        // horrible problem

        mesh_memory->add_barriers_before_data_upload(mesh_upload_cmds);

        mesh_upload_queue_mutex.lock();
        meshes_mutex.lock();

        std::vector<vk_buffer> freed_buffers;
        freed_buffers.reserve(mesh_upload_queue.size() * 2);
        while(!mesh_upload_queue.empty()) {
            const mesh_staging_buffer_upload_command cmd = mesh_upload_queue.front();
            mesh_upload_queue.pop();

            compacting_block_allocator::allocation_info* mem = mesh_memory->allocate(cmd.staging_buffer.alloc_info.size);

            VkBufferCopy copy = {};
            copy.size = cmd.staging_buffer.alloc_info.size;
            copy.srcOffset = 0;
            copy.dstOffset = mem->offset;
            vkCmdCopyBuffer(mesh_upload_cmds, cmd.staging_buffer.buffer, mem->block->get_buffer(), 1, &copy);

            VkDrawIndexedIndirectCommand mesh_draw_command = {};
            mesh_draw_command.indexCount = (cmd.model_matrix_offset - cmd.indices_offset) / sizeof(uint32_t);
            mesh_draw_command.instanceCount = 1;
            mesh_draw_command.firstIndex = 0;
            mesh_draw_command.vertexOffset = static_cast<uint32_t>(mem->offset);
            mesh_draw_command.firstInstance = 0;

            meshes[cmd.mesh_id] = {mem, cmd.indices_offset, cmd.model_matrix_offset, mesh_draw_command};

            freed_buffers.insert(freed_buffers.end(), cmd.staging_buffer);
        }

        mesh_memory->add_barriers_after_data_upload(mesh_upload_cmds);

        mesh_upload_queue_mutex.unlock();
        meshes_mutex.unlock();

        vkEndCommandBuffer(mesh_upload_cmds);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mesh_upload_cmds;

        // Be super duper sure that mesh rendering is done
        for(const auto& [pass_name, pass] : render_passes) {
            (void) pass_name;
            vkWaitForFences(device, 1, &pass.fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        }
        vkQueueSubmit(copy_queue, 1, &submit_info, upload_to_megamesh_buffer_done);

        vkWaitForFences(device, 1, &upload_to_megamesh_buffer_done, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkResetFences(device, 1, &upload_to_megamesh_buffer_done);

        // Once the upload is done, return all the staging buffers to the pool
        std::lock_guard l(mesh_staging_buffers_mutex);
        available_mesh_staging_buffers.insert(available_mesh_staging_buffers.end(), freed_buffers.begin(), freed_buffers.end());
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

    void vulkan_render_engine::render_frame() {
        reset_render_finished_semaphores();

        current_semaphore_idx = 0;
        vkWaitForFences(device, 1, &frame_fences.at(current_frame), VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(device, 1, &frame_fences.at(current_frame));

        swapchain->acquire_next_swapchain_image(image_available_semaphores.at(current_frame));

        // Records and submits a command buffer that barriers until reading vertex data from the megamesh buffer has
        // finished, uploads new mesh parts, then barriers until transfers to the megamesh vertex buffer are finished
        upload_new_mesh_parts();

        // Record command buffers
        // We can't upload a new shaderpack in the middle of a frame!
        // Future iterations of this code will likely be more clever, so that "load shaderpack" gets scheduled for the beginning of the frame
        shaderpack_loading_mutex.lock();

        if(dynamic_textures_need_to_transition) {
            transition_dynamic_textures();
        }

        for(const std::string& renderpass_name : render_passes_by_order) {
            execute_renderpass(&renderpass_name);
        }
        shaderpack_loading_mutex.unlock();
        
        swapchain->present_current_image(render_finished_semaphores_by_frame.at(current_frame));
        current_frame = (current_frame + 1) % max_frames_in_queue;
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

    void vulkan_render_engine::execute_renderpass(const std::string* renderpass_name) {
        const vk_render_pass& renderpass = render_passes.at(*renderpass_name);
        vkResetFences(device, 1, &renderpass.fence);

        const VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        NOVA_THROW_IF_VK_ERROR(vkAllocateCommandBuffers(device, &alloc_info, &cmds), buffer_allocate_failed);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        const std::vector<vk_pipeline> pipelines = pipelines_by_renderpass.at(*renderpass_name);

        std::vector<VkCommandBuffer> secondary_command_buffers(pipelines.size());

        ttl::condition_counter pipelines_rendering_counter;
        uint32_t i = 0;
        for(const vk_pipeline& pipe : pipelines) {
            render_pipeline(&pipe, &secondary_command_buffers[i], renderpass);
            i++;
        }

        vkBeginCommandBuffer(cmds, &begin_info);

#pragma region Texture attachment layout transition
        if(!renderpass.read_texture_barriers.empty()) {
            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(renderpass.read_texture_barriers.size()), renderpass.read_texture_barriers.data());
        }

        if(!renderpass.write_texture_barriers.empty()) {
            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(renderpass.write_texture_barriers.size()), renderpass.write_texture_barriers.data());
        }

        if(renderpass.writes_to_backbuffer) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = swapchain->get_current_image();
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Each swapchain image **will** be rendered to before it is presented
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        // TODO: Any barriers for aliased textures if we're at an aliased boundary
        // TODO: Something about the depth buffer
#pragma endregion

        VkClearValue clear_value = {};
        clear_value.color = {{0, 0, 0, 0}};

        const VkClearValue clear_values[] = {clear_value, clear_value};

        VkRenderPassBeginInfo rp_begin_info = {};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = renderpass.pass;
        rp_begin_info.framebuffer = renderpass.framebuffer.framebuffer;
        rp_begin_info.renderArea = renderpass.render_area;
        rp_begin_info.clearValueCount = 2;
        rp_begin_info.pClearValues = clear_values;

        if(rp_begin_info.framebuffer == nullptr) {
            rp_begin_info.framebuffer = swapchain->get_current_framebuffer();
        }

        NOVA_LOG(TRACE) << "Starting renderpass " << *renderpass_name << " with framebuffer " << rp_begin_info.framebuffer;

        vkCmdBeginRenderPass(cmds, &rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vkCmdExecuteCommands(cmds, static_cast<uint32_t>(secondary_command_buffers.size()), secondary_command_buffers.data());

        vkCmdEndRenderPass(cmds);

        if(renderpass.writes_to_backbuffer) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = swapchain->get_current_image();
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        vkEndCommandBuffer(cmds);

        // If we write to the backbuffer, we need to signal the full-frame fence. If we do not, we can signal the individual renderpass's fence
        if(renderpass.writes_to_backbuffer) {
            submit_to_queue(cmds, graphics_queue, frame_fences.at(current_frame), {image_available_semaphores.at(current_frame)});

        } else {
            submit_to_queue(cmds, graphics_queue, renderpass.fence, {});
        }
    }

    void vulkan_render_engine::render_pipeline(const vk_pipeline* pipeline, VkCommandBuffer* cmds, const vk_render_pass& renderpass) {
        // This function is intended to be run inside a separate fiber than its caller, so it needs to get the
        // command pool for its thread, since command pools need to be externally synchronized
        const VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo cmds_info = {};
        cmds_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmds_info.commandPool = command_pool;
        cmds_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cmds_info.commandBufferCount = 1;

        vkAllocateCommandBuffers(device, &cmds_info, cmds);

        VkCommandBufferInheritanceInfo cmds_inheritance_info = {};
        cmds_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        cmds_inheritance_info.framebuffer = renderpass.framebuffer.framebuffer;
        cmds_inheritance_info.renderPass = renderpass.pass;

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        begin_info.pInheritanceInfo = &cmds_inheritance_info;

        // Start command buffer, start renderpass, and bind pipeline
        vkBeginCommandBuffer(*cmds, &begin_info);

        vkCmdBindPipeline(*cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        // Record all the draws for a material
        // I'll worry about depth sorting later
        const std::vector<material_pass> materials = material_passes_by_pipeline.at(pipeline->data.name);
        for(const material_pass& pass : materials) {
            bind_material_resources(pass, *pipeline, *cmds);

            draw_all_for_material(pass, *cmds);
        }

        vkEndCommandBuffer(*cmds);
    }

    void vulkan_render_engine::bind_material_resources(const material_pass& mat_pass, const vk_pipeline& pipeline, VkCommandBuffer cmds) {
        vkCmdBindDescriptorSets(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &mat_pass.descriptor_sets.at(0), 0, nullptr);
    }

    void vulkan_render_engine::draw_all_for_material(const material_pass& pass, VkCommandBuffer cmds) {
        // Version 1: Put indirect draw commands into a buffer right here, send that data to the GPU, and render that
        // Version 2: Let the host application tell us which render objects are visible and which are not, and incorporate that information
        // Version 3: Send data about what is and isn't visible to the GPU and construct the indirect draw commands buffer in a compute shader
        // Version 2: Incorporate occlusion queries so we know what with all certainty what is and isn't visible

        // At the current time I'm making version 1

        // TODO: _Anything_ smarter

        if(renderables_by_material.find(pass.name) == renderables_by_material.end()) {
            // Nothing to render? Don't render it!

            // smh

            return;
        }

        const std::unordered_map<VkBuffer, std::vector<render_object>>& renderables_by_buffer = renderables_by_material.at(pass.name);

        for(const auto& [buffer, renderables] : renderables_by_buffer) {
            VkBufferCreateInfo buffer_create_info = {};
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand) * renderables.size();
            buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_create_info.queueFamilyIndexCount = 1;
            buffer_create_info.pQueueFamilyIndices = &graphics_family_index;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

            VkBuffer indirect_draw_commands_buffer;
            VmaAllocation allocation;
            VmaAllocationInfo alloc_info;

            NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator, &buffer_create_info, &alloc_create_info, &indirect_draw_commands_buffer, &allocation, &alloc_info), buffer_allocate_failed);

            // Version 1: write commands for all things to the indirect draw buffer
            VkDrawIndexedIndirectCommand* indirect_commands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(alloc_info.pMappedData);

            for(uint32_t i = 0; i < renderables.size(); i++) {
                const render_object& cur_obj = renderables.at(i);
                indirect_commands[i] = cur_obj.mesh->draw_cmd;
            }

            vkCmdBindVertexBuffers(cmds, 0, 1, &buffer, nullptr);
            vkCmdBindIndexBuffer(cmds, buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexedIndirect(cmds, indirect_draw_commands_buffer, 0, static_cast<uint32_t>(renderables.size()), 0);
        }
    }

    void vulkan_render_engine::submit_to_queue(VkCommandBuffer cmds, VkQueue queue, VkFence cmd_buffer_done_fence, const std::vector<VkSemaphore>& wait_semaphores) {
        std::lock_guard l(render_done_sync_mutex);
        std::vector<VkSemaphore>& render_finished_semaphores = render_finished_semaphores_by_frame.at(current_frame);

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        while(render_finished_semaphores.size() <= current_semaphore_idx) {
            VkSemaphore semaphore;

            NOVA_THROW_IF_VK_ERROR(vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore), render_engine_initialization_exception);

            render_finished_semaphores.push_back(semaphore);
        }

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        const bool one_null_semaphore = wait_semaphores.size() == 1 && wait_semaphores.at(0) == VK_NULL_HANDLE;
        if(!one_null_semaphore) {
            submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
            submit_info.pWaitSemaphores = wait_semaphores.data();
        }
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_finished_semaphores.at(current_semaphore_idx);
        NOVA_THROW_IF_VK_ERROR(vkQueueSubmit(queue, 1, &submit_info, cmd_buffer_done_fence), render_engine_rendering_exception);

        current_semaphore_idx++;
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

    void vulkan_render_engine::reset_render_finished_semaphores() {
        render_finished_semaphores_by_frame[current_frame].clear();
    }

    void vulkan_render_engine::add_resource_to_bindings(std::unordered_map<std::string, vk_resource_binding>& bindings, const spirv_cross::CompilerGLSL& shader_compiler, const spirv_cross::Resource& resource, const VkDescriptorType type) {
        const uint32_t set = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        vk_resource_binding new_binding = {};
        new_binding.set = set;
        new_binding.binding = binding;
        new_binding.descriptorType = type;
        new_binding.descriptorCount = 1;

        if(bindings.find(resource.name) == bindings.end()) {
            // Totally new binding!
            bindings[resource.name] = new_binding;

        } else {
            // Existing binding. Is it the same as our binding?
            const vk_resource_binding& existing_binding = bindings.at(resource.name);
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name << " in different shader stages. This is not allowed. Use unique names";
            }
        }
    }

    void vulkan_render_engine::get_shader_module_descriptors(const std::vector<uint32_t>& spirv, std::unordered_map<std::string, vk_resource_binding>& bindings) {
        const spirv_cross::CompilerGLSL shader_compiler(spirv);
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const spirv_cross::Resource& resource : resources.sampled_images) {
            add_resource_to_bindings(bindings, shader_compiler, resource, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }

        for(const spirv_cross::Resource& resource : resources.uniform_buffers) {
            add_resource_to_bindings(bindings, shader_compiler, resource, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
    }

    std::vector<VkDescriptorSetLayout> vulkan_render_engine::create_descriptor_set_layouts(std::unordered_map<std::string, vk_resource_binding> all_bindings) const {
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings_by_set;

        for(const auto& named_binding : all_bindings) {
            const vk_resource_binding& binding = named_binding.second;
            VkDescriptorSetLayoutBinding new_binding = {};
            new_binding.binding = binding.binding;
            new_binding.descriptorCount = binding.descriptorCount;
            new_binding.descriptorType = binding.descriptorType;
            new_binding.pImmutableSamplers = binding.pImmutableSamplers;
            new_binding.stageFlags = VK_SHADER_STAGE_ALL;

            bindings_by_set[binding.set].push_back(new_binding);
        }

        std::vector<VkDescriptorSetLayoutCreateInfo> dsl_create_infos = {};
        dsl_create_infos.reserve(bindings_by_set.size());
        for(uint32_t i = 0; i < bindings_by_set.size(); i++) {
            if(bindings_by_set.find(i) == bindings_by_set.end()) {
                NOVA_LOG(ERROR) << "Could not get information for descriptor set " << i << "; most likely you skipped"
                                << " a descriptor set in your shader. Ensure that all shaders for this pipeline together don't have"
                                << " any gaps in the descriptor sets they declare";
                throw shader_layout_creation_failed("Descriptor set " + std::to_string(i) + " not present");
            }

            const std::vector<VkDescriptorSetLayoutBinding>& bindings = bindings_by_set.at(i);

            VkDescriptorSetLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            create_info.pBindings = bindings.data();

            dsl_create_infos.push_back(create_info);
        }

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.resize(dsl_create_infos.size());
        for(uint32_t i = 0; i < dsl_create_infos.size(); i++) {
            vkCreateDescriptorSetLayout(device, &dsl_create_infos[i], nullptr, &layouts[i]);
        }

        return layouts;
    }

    std::vector<VkImageMemoryBarrier> vulkan_render_engine::make_attachment_to_shader_read_only_barriers(const std::unordered_set<std::string>& textures) {
        std::vector<VkImageMemoryBarrier> barriers;
        barriers.reserve(textures.size());

        for(const std::string& tex_name : textures) {
            const vk_texture& tex = this->textures.at(tex_name);

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = tex.image;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            if(tex.is_depth_tex) {
                barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            } else {
                barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            barriers.push_back(barrier);
        }

        return barriers;
    }

    void vulkan_render_engine::write_texture_to_descriptor(const vk_texture& texture, VkWriteDescriptorSet& write, std::vector<VkDescriptorImageInfo>& image_infos) const {
        VkDescriptorImageInfo image_info = {};
        image_info.imageView = texture.image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.sampler = point_sampler;

        image_infos.push_back(image_info);

        write.pImageInfo = &image_infos.at(image_infos.size() - 1);
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    void vulkan_render_engine::write_buffer_to_descriptor(const vk_buffer& buffer, VkWriteDescriptorSet& write, std::vector<VkDescriptorBufferInfo>& buffer_infos) {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.buffer;
        buffer_info.offset = 0;
        buffer_info.range = buffer.alloc_info.size;

        buffer_infos.push_back(buffer_info);

        write.pBufferInfo = &buffer_infos[buffer_infos.size() - 1];
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
