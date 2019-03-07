/*!
 * \author ddubois
 * \date 07-Feb-19.
 */

#include "fmt/format.h"
#include "swapchain.hpp"
#include "vulkan_render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer {
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
        // Future iterations of this code will likely be more clever, so that "load shaderpack" gets scheduled for the beginning of the
        // frame
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

    void vulkan_render_engine::reset_render_finished_semaphores() { render_finished_semaphores_by_frame[current_frame].clear(); }

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
            mesh_draw_command.indexCount = static_cast<uint32_t>((cmd.model_matrix_offset - cmd.indices_offset) / sizeof(uint32_t));
            mesh_draw_command.instanceCount = 1;
            mesh_draw_command.firstIndex = 0;
            mesh_draw_command.vertexOffset = static_cast<int32_t>(mem->offset);
            mesh_draw_command.firstInstance = 0;

            meshes[cmd.mesh_id] = {mem, cmd.indices_offset, cmd.model_matrix_offset, mesh_draw_command, cmd.mesh_id};

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
            }
            else {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                color_barriers.push_back(barrier);
            }
        }

        VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

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

        vkCmdPipelineBarrier(cmds,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             static_cast<uint32_t>(color_barriers.size()),
                             color_barriers.data());

        vkCmdPipelineBarrier(cmds,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             static_cast<uint32_t>(depth_barriers.size()),
                             depth_barriers.data());

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

    void vulkan_render_engine::execute_renderpass(const std::string* renderpass_name) {
        const vk_render_pass& renderpass = render_passes.at(*renderpass_name);
        vkResetFences(device, 1, &renderpass.fence);

        VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

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

        nova::ttl::condition_counter pipelines_rendering_counter;
        uint32_t i = 0;
        for(const vk_pipeline& pipe : pipelines) {
            render_pipeline(&pipe, &secondary_command_buffers[i], renderpass);
            i++;
        }

        vkBeginCommandBuffer(cmds, &begin_info);

#pragma region Texture attachment layout transition
        if(!renderpass.read_texture_barriers.empty()) {
            vkCmdPipelineBarrier(cmds,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 static_cast<uint32_t>(renderpass.read_texture_barriers.size()),
                                 renderpass.read_texture_barriers.data());
        }

        if(!renderpass.write_texture_barriers.empty()) {
            vkCmdPipelineBarrier(cmds,
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 static_cast<uint32_t>(renderpass.write_texture_barriers.size()),
                                 renderpass.write_texture_barriers.data());
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
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Each swapchain image **will** be rendered to before it is
                                                                          // presented
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            vkCmdPipelineBarrier(cmds,
                                 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);
        }

        // TODO: Any barriers for aliased textures if we're at an aliased boundary
        // TODO: Something about the depth buffer
#pragma endregion

        VkClearValue clear_value = {};
        clear_value.color = {{0, 0, 0, 0}};

        const std::array<VkClearValue, 2> clear_values = {clear_value, clear_value};

        VkRenderPassBeginInfo rp_begin_info = {};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = renderpass.pass;
        rp_begin_info.framebuffer = renderpass.framebuffer.framebuffer;
        rp_begin_info.renderArea = renderpass.render_area;
        rp_begin_info.clearValueCount = 2;
        rp_begin_info.pClearValues = clear_values.data();

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

            vkCmdPipelineBarrier(cmds,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);
        }

        vkEndCommandBuffer(cmds);

        // If we write to the backbuffer, we need to signal the full-frame fence. If we do not, we can signal the individual renderpass's
        // fence
        if(renderpass.writes_to_backbuffer) {
            submit_to_queue(cmds, graphics_queue, frame_fences.at(current_frame), {image_available_semaphores.at(current_frame)});
        }
        else {
            submit_to_queue(cmds, graphics_queue, renderpass.fence, {});
        }
    }

    void vulkan_render_engine::render_pipeline(const vk_pipeline* pipeline, VkCommandBuffer* cmds, const vk_render_pass& renderpass) {
        // This function is intended to be run inside a separate fiber than its caller, so it needs to get the
        // command pool for its thread, since command pools need to be externally synchronized
        VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

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

            result<std::string> per_model_buffer_binding = find_per_model_buffer_binding(pass);

            draw_all_for_material(pass, *cmds, per_model_buffer_binding);
        }

        vkEndCommandBuffer(*cmds);
    }

    result<std::string> vulkan_render_engine::find_per_model_buffer_binding(const material_pass& mat_pass) {
        for(const auto& [descriptor_name, resource_name] : mat_pass.bindings) {
            if(resource_name == "NovaPerModelUBO") {
                return result<std::string>{descriptor_name};
            }
        }

        return result<std::string>("Could not find per-model UBO binding"_err);
    }

    void vulkan_render_engine::bind_material_resources(const material_pass& mat_pass, const vk_pipeline& pipeline, VkCommandBuffer cmds) {
        vkCmdBindDescriptorSets(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, mat_pass.descriptor_sets.data(), 0, nullptr);
    }

    void vulkan_render_engine::draw_all_for_material(const material_pass& pass,
                                                     VkCommandBuffer cmds,
                                                     const result<std::string>& per_model_buffer_binding) {
        // Version 1: Put indirect draw commands into a buffer right here, send that data to the GPU, and render that
        // Version 2: Let the host application tell us which render objects are visible and which are not, and incorporate that information
        // Version 3: Send data about what is and isn't visible to the GPU and construct the indirect draw commands buffer in a compute
        // shader
        // Version 4: Incorporate occlusion queries so we know what with all certainty what is and isn't visible

        // At the current time I'm making version 1

        // TODO: _Anything_ smarter

        if(renderables_by_material.find(pass.name) == renderables_by_material.end()) {
            // Nothing to render? Don't render it!

            // smh

            return;
        }

        const std::unordered_map<VkBuffer, vk_renderables>& renderables_by_buffer = renderables_by_material.at(pass.name);

        /*
         * For the compute shader that generates draw commands:
         *
         *
         * ZerfYesterday at 11:52 PM
         * so in that case, do something like this
         * curr_index = 0
         * for each modeltype
         *   start_index = curr_index
         *   for each instance of modeltype
         *      if ( is_culled ) {
         *         continue
         *      }
         *      matrix_indices[curr_index] = matrix_index;
         *      ++curr_index
         *   }
         *   if ( curr_index != start_index ) {
         *      write render_indirect data
         *      instanceCount = curr_index - start_index
         *      firstInstance = start_index
         *   }
         * then in your shader, you can get the offset into matrix_indices by using gl_InstanceIndex
         * and if you need the current instance, you can calculate it by using
         * int curr_instance = gl_InstanceIndex - gl_BaseInstance
         */

        for(const auto& [buffer, renderables] : renderables_by_buffer) {
            VkBufferCreateInfo buffer_create_info = {};
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand) * renderables.static_meshes.size();
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

            NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator,
                                                   &buffer_create_info,
                                                   &alloc_create_info,
                                                   &indirect_draw_commands_buffer,
                                                   &allocation,
                                                   &alloc_info),
                                   buffer_allocate_failed);

            // Version 1: write commands for all things to the indirect draw buffer
            auto* indirect_commands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(alloc_info.pMappedData);
            std::unordered_map<uint32_t, uint32_t> matrix_indices;
            uint32_t start_index = 0;
            uint32_t cur_index = 0;
            uint32_t draw_command_write_index = 0;
            for(const auto& [mesh_id, static_meshes] : renderables.static_meshes) {
                start_index = cur_index;
                for(const vk_static_mesh_renderable& static_mesh : static_meshes) {
                    if(static_mesh.is_visible) {
                        matrix_indices[cur_index] = static_mesh.model_matrix_slot->index;
                        ++cur_index;
                    }
                }

                if(cur_index != start_index) {
                    const vk_mesh& mesh = meshes.at(mesh_id);

                    VkDrawIndexedIndirectCommand& cmd = indirect_commands[draw_command_write_index];
                    cmd.vertexOffset = mesh.draw_cmd.vertexOffset;
                    cmd.firstIndex = mesh.draw_cmd.firstIndex;
                    cmd.indexCount = mesh.draw_cmd.indexCount;
                    cmd.firstInstance = start_index;
                    cmd.instanceCount = cur_index - start_index;

                    ++draw_command_write_index;
                }
            }

            VkDeviceSize offsets[7] = {0, 0, 0, 0, 0, 0, 0};
            VkBuffer buffers[7] = {buffer, buffer, buffer, buffer, buffer, buffer, buffer};
            vkCmdBindVertexBuffers(cmds, 0, 7, buffers, offsets);
            vkCmdBindIndexBuffer(cmds, buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexedIndirect(cmds, indirect_draw_commands_buffer, 0, static_cast<uint32_t>(renderables.static_meshes.size()), 0);
        }
    }

    void vulkan_render_engine::submit_to_queue(VkCommandBuffer cmds,
                                               VkQueue queue,
                                               VkFence cmd_buffer_done_fence,
                                               const std::vector<VkSemaphore>& wait_semaphores) {
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

} // namespace nova::renderer
