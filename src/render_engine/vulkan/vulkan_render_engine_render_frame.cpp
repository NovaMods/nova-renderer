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
        NOVA_LOG(DEBUG) << "\n*******************************\n*         FRAME START         *\n*******************************\n";

        const uint32_t cur_frame = current_swapchain_image;

        // Apparently these fences are still in use?
        NOVA_LOG(TRACE) << "Waiting on fence " << frame_fences.at(cur_frame);
        NOVA_CHECK_RESULT(vkWaitForFences(device, 1, &frame_fences.at(cur_frame), VK_TRUE, std::numeric_limits<uint64_t>::max()));
        NOVA_CHECK_RESULT(vkResetFences(device, 1, &frame_fences.at(current_swapchain_image)));

        swapchain->acquire_next_swapchain_image(image_available_semaphores.at(cur_frame));

        upload_new_ubos();

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

        VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        NOVA_CHECK_RESULT(vkAllocateCommandBuffers(device, &alloc_info, &cmds));

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmds, &begin_info);

        for(const std::string& renderpass_name : render_passes_by_order) {
            record_renderpass(&renderpass_name, cmds);
        }
        shaderpack_loading_mutex.unlock();

        NOVA_CHECK_RESULT(vkEndCommandBuffer(cmds));

        NOVA_LOG(TRACE) << "Submitting a command buffer that will wait on semaphore " << image_available_semaphores.at(cur_frame) << " and then signal fence " << frame_fences.at(cur_frame);
        submit_to_queue(cmds, graphics_queue, frame_fences.at(cur_frame), {image_available_semaphores.at(cur_frame)}, {render_finished_semaphores.at(cur_frame)});

        swapchain->present_current_image(render_finished_semaphores.at(cur_frame));

        current_frame++;
        current_swapchain_image = current_frame % max_in_flight_frames;
    }

    void vulkan_render_engine::reset_render_finished_semaphores() {
        for(const VkSemaphore& semaphore : render_finished_semaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }

        render_finished_semaphores.clear();
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
        alloc_info.commandPool = get_command_buffer_pool_for_current_thread(transfer_family_index);
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

        // Be super duper sure that mesh rendering is done
        // TODO: Something smarter
        for(const auto& [pass_name, pass] : render_passes) {
            (void) pass_name;
            NOVA_CHECK_RESULT(vkWaitForFences(device, 1, &pass.fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        }
        NOVA_CHECK_RESULT(vkResetFences(device, 1, &upload_to_megamesh_buffer_done));

        submit_to_queue(mesh_upload_cmds, copy_queue, upload_to_megamesh_buffer_done);

        NOVA_CHECK_RESULT(vkWaitForFences(device, 1, &upload_to_megamesh_buffer_done, VK_TRUE, std::numeric_limits<uint64_t>::max()));

        NOVA_CHECK_RESULT(vkResetFences(device, 1, &upload_to_megamesh_buffer_done));

        // Once the upload is done, return all the staging buffers to the pool
        // TODO: Do this asynchronosly once Nova has async things
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
            } else {
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

        NOVA_CHECK_RESULT(vkCreateFence(device, &fence_create_info, nullptr, &transition_done_fence));
        
        submit_to_queue(cmds, graphics_queue, transition_done_fence, {}, { render_finished_semaphores.at(current_swapchain_image) });
        NOVA_CHECK_RESULT(vkWaitForFences(device, 1, &transition_done_fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        vkDestroyFence(device, transition_done_fence, nullptr);

        vkFreeCommandBuffers(device, command_pool, 1, &cmds);

        dynamic_textures_need_to_transition = false;
    }

    void vulkan_render_engine::record_renderpass(const std::string* renderpass_name, VkCommandBuffer cmds) {
        const vk_render_pass& renderpass = render_passes.at(*renderpass_name);
        NOVA_CHECK_RESULT(vkResetFences(device, 1, &renderpass.fence));

        const std::vector<vk_pipeline> pipelines = pipelines_by_renderpass.at(*renderpass_name);

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

        vkCmdBeginRenderPass(cmds, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        uint32_t i = 0;
        for(const vk_pipeline& pipe : pipelines) {
            record_pipeline(&pipe, &cmds, renderpass);
            i++;
        }

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
    }

    void vulkan_render_engine::record_pipeline(const vk_pipeline* pipeline, VkCommandBuffer* cmds, const vk_render_pass& renderpass) {
        // This function is intended to be run inside a separate fiber than its caller, so it needs to get the
        // command pool for its thread, since command pools need to be externally synchronized
        // VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);
        //
        // VkCommandBufferAllocateInfo cmds_info = {};
        // cmds_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        // cmds_info.commandPool = command_pool;
        // cmds_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        // cmds_info.commandBufferCount = 1;
        //
        // NOVA_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmds_info, cmds));
        //
        // VkCommandBufferInheritanceInfo cmds_inheritance_info = {};
        // cmds_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        // cmds_inheritance_info.framebuffer = renderpass.framebuffer.framebuffer;
        // cmds_inheritance_info.renderPass = renderpass.pass;
        //
        // VkCommandBufferBeginInfo begin_info = {};
        // begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        // begin_info.pInheritanceInfo = &cmds_inheritance_info;
        //
        // // Start command buffer, start renderpass, and bind pipeline
        // NOVA_CHECK_RESULT(vkBeginCommandBuffer(*cmds, &begin_info));

        vkCmdBindPipeline(*cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        // Record all the draws for a material
        // I'll worry about depth sorting later
        const std::vector<material_pass> materials = material_passes_by_pipeline.at(pipeline->data.name);
        for(const material_pass& pass : materials) {
            bind_material_resources(pass, *pipeline, *cmds);
            record_drawing_all_for_material(pass, *cmds);
        }

        // vkEndCommandBuffer(*cmds);
    }

    void vulkan_render_engine::bind_material_resources(const material_pass& mat_pass, const vk_pipeline& pipeline, VkCommandBuffer cmds) {
        vkCmdBindDescriptorSets(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, mat_pass.descriptor_sets.data(), 0, nullptr);
    }

    void vulkan_render_engine::record_drawing_all_for_material(const material_pass& pass, VkCommandBuffer cmds) {

        NOVA_LOG(TRACE) << "Recording drawcalls for material pass " << pass.name << " in material " << pass.material_name;

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

            NOVA_CHECK_RESULT(vmaCreateBuffer(vma_allocator,
                                              &buffer_create_info,
                                              &alloc_create_info,
                                              &indirect_draw_commands_buffer,
                                              &allocation,
                                              &alloc_info));

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
                                               const std::vector<VkSemaphore>& wait_semaphores,
                                               const std::vector<VkSemaphore>& signal_semaphores) {
               
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        const bool one_null_semaphore = wait_semaphores.size() == 1 && wait_semaphores.at(0) == nullptr;
        if(!one_null_semaphore) {
            submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
            submit_info.pWaitSemaphores = wait_semaphores.data();
        }
        submit_info.signalSemaphoreCount = signal_semaphores.size();
        submit_info.pSignalSemaphores = signal_semaphores.data();
        NOVA_CHECK_RESULT(vkQueueSubmit(queue, 1, &submit_info, cmd_buffer_done_fence));

        NOVA_LOG(TRACE) << "Submitted command buffer " << cmds << " to queue " << queue;
    }

} // namespace nova::renderer
