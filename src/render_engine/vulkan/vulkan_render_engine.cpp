/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#include "vulkan_render_engine.hpp"
#include "../../util/logger.hpp"
#include "vk_structs.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer {
    vulkan_render_engine::vulkan_render_engine(nova_settings& settings) : render_engine(settings) {}

    std::shared_ptr<window> vulkan_render_engine::get_window() const { return std::shared_ptr<window>(); }

    result<renderpass_t*> vulkan_render_engine::create_renderpass(const render_pass_create_info_t& data) {
        VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();

        vk_renderpass_t* renderpass = new vk_renderpass_t;

        VkSubpassDescription subpass_description;
        subpass_description.flags = 0;
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = 0;
        subpass_description.pInputAttachments = nullptr;
        subpass_description.preserveAttachmentCount = 0;
        subpass_description.pPreserveAttachments = nullptr;
        subpass_description.pResolveAttachments = nullptr;
        subpass_description.pDepthStencilAttachment = nullptr;

        VkSubpassDependency image_available_dependency;
        image_available_dependency.dependencyFlags = 0;
        image_available_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        image_available_dependency.dstSubpass = 0;
        image_available_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_dependency.srcAccessMask = 0;
        image_available_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info;
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.pNext = nullptr;
        render_pass_create_info.flags = 0;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass_description;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &image_available_dependency;

        std::vector<VkAttachmentReference> attachment_references;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkImageView> framebuffer_attachments;
        uint32_t framebuffer_width = 0;
        uint32_t framebuffer_height = 0;

        bool writes_to_backbuffer = false;
        // Collect framebuffer size information from color output attachments
        for(const texture_attachment_info_t& attachment : data.texture_outputs) {
            if(attachment.name == "Backbuffer") {
                // Handle backbuffer
                // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip
                // everything
                writes_to_backbuffer = true;

                VkAttachmentDescription desc = {};
                desc.flags = 0;
                desc.format = swapchain->get_swapchain_format();
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                attachments.push_back(desc);

                VkAttachmentReference ref = {};

                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

                attachment_references.push_back(ref);

                framebuffer_width = swapchain_extent.width;
                framebuffer_height = swapchain_extent.height;

                renderpass->writes_to_backbuffer = true;

                break;
            }

            VkAttachmentDescription desc = {};
            desc.flags = 0;
            desc.format = to_vk_format(attachment.pixel_format);
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = attachment.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachments.push_back(desc);

            VkAttachmentReference ref = {};

            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ref.attachment = static_cast<uint32_t>(attachments.size()) - 1;

            attachment_references.push_back(ref);
        }

        VkAttachmentReference depth_reference = {};
        // Collect framebuffer size information from the depth attachment
        if(data.depth_texture) {
            VkAttachmentDescription desc = {};
            desc.flags = 0;
            desc.format = to_vk_format(data.depth_texture->pixel_format);
            desc.samples = VK_SAMPLE_COUNT_1_BIT;
            desc.loadOp = data.depth_texture->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachments.push_back(desc);

            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_reference.attachment = static_cast<uint32_t>(attachments.size()) - 1;

            subpass_description.pDepthStencilAttachment = &depth_reference;
        }

        if(framebuffer_width == 0) {
            return result<renderpass_t*>(MAKE_ERROR(
                "Framebuffer width for pass {:s} is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero width",
                data.name));
        }

        if(framebuffer_height == 0) {
            return result<renderpass_t*>(MAKE_ERROR(
                "Framebuffer height for pass {:s} is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero height",
                data.name));
        }

        if(framebuffer_attachments.size() > gpu.props.limits.maxColorAttachments) {
            return result<renderpass_t*>(MAKE_ERROR(
                "Framebuffer for pass {:s} has {:d} color attachments, but your GPU only supports {:d}. Please reduce the number of attachments that this pass uses, possibly by changing some of your input attachments to bound textures",
                data.name,
                data.texture_outputs.size(),
                gpu.props.limits.maxColorAttachments));
        }

        subpass_description.colorAttachmentCount = static_cast<uint32_t>(attachment_references.size());
        subpass_description.pColorAttachments = attachment_references.data();

        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();

        NOVA_CHECK_RESULT(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &renderpass->pass));

        if(writes_to_backbuffer) {
            if(data.texture_outputs.size() > 1) {
                NOVA_LOG(ERROR)
                    << "Pass " << data.name
                    << " writes to the backbuffer, and other textures. Passes that write to the backbuffer are not allowed to write to any other textures";
            }
        }

        renderpass->render_area = {{0, 0}, {framebuffer_width, framebuffer_height}};

        if(settings.debug.enabled) {
            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(renderpass->pass);
            object_name.pObjectName = data.name.c_str();
            NOVA_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &object_name));
        }
    }

    pipeline_t* vulkan_render_engine::create_pipeline(const pipeline_create_info_t& data) { return nullptr; }

    resource_t* vulkan_render_engine::create_buffer(const buffer_create_info_t& info) { return nullptr; }

    resource_t* vulkan_render_engine::create_texture(const texture2d_create_info_t& info) { return nullptr; }

    semaphore_t* vulkan_render_engine::create_semaphore() { return nullptr; }

    std::vector<semaphore_t*> vulkan_render_engine::create_semaphores(uint32_t num_semaphores) { return std::vector<semaphore_t*>(); }

    fence_t* vulkan_render_engine::create_fence(bool signaled = false) { return nullptr; }

    std::vector<fence_t*> vulkan_render_engine::create_fences(uint32_t num_fences, bool signaled = false) {
        return std::vector<fence_t*>();
    }

    void vulkan_render_engine::destroy_renderpass(renderpass_t* pass) {}

    void vulkan_render_engine::destroy_pipeline(pipeline_t* pipeline) {}

    void vulkan_render_engine::destroy_resource(resource_t* resource) {}

    void vulkan_render_engine::destroy_semaphores(const std::vector<semaphore_t*>& semaphores) {}

    void vulkan_render_engine::destroy_fences(const std::vector<fence_t*>& fences) {}

    command_list_t* vulkan_render_engine::allocate_command_list(uint32_t thread_idx,
                                                                queue_type needed_queue_type,
                                                                command_list_t::level level) {
        return nullptr;
    }

    void vulkan_render_engine::submit_command_list(command_list_t* cmds,
                                                   queue_type queue,
                                                   fence_t* fence_to_signal = nullptr,
                                                   const std::vector<semaphore_t*>& wait_semaphores = {},
                                                   const std::vector<semaphore_t*>& signal_semaphores = {}) {}

    void vulkan_render_engine::open_window(uint32_t width, uint32_t height) {}
} // namespace nova::renderer
