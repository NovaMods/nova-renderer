/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#include <vulkan/vulkan.hpp>
#include "renderpass_manager.h"
#include "../../vulkan/render_context.h"
#include "../renderpass.h"
#include <easylogging++.h>

namespace nova {
    renderpass_manager::renderpass_manager() {
        // TODO: Inspect each shader, figure out which ones draw to which target, and build renderpasses based on that info
        // For now I'll just create the swapchain renderpass to make sure I hae the code

        auto& framebuffer_size = render_context::instance.swapchain_extent;

        rebuild_all(framebuffer_size);
    }

    void renderpass_manager::create_final_renderpass(vk::Extent2D& window_size) {
        std::vector<vk::AttachmentDescription> attachments;

        std::vector<vk::AttachmentReference> color_refs;

        // Just one color buffer in the final pass
        vk::AttachmentDescription color_attachment = {};
        color_attachment.format = render_context::instance.swapchain_format;
        color_attachment.samples = vk::SampleCountFlagBits::e1;
        color_attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
        color_attachment.initialLayout = vk::ImageLayout::eUndefined;
        color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        attachments.push_back(color_attachment);

        vk::AttachmentReference ref = {};
        ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
        color_refs.push_back(ref);

        // Don't forget the depth buffer!
        vk::AttachmentDescription depth_attachment = {};
        depth_attachment.format = render_context::instance.depth_format;
        depth_attachment.samples = vk::SampleCountFlagBits::e1;
        depth_attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
        depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
        depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        attachments.push_back(depth_attachment);

        vk::AttachmentReference depth_ref = {};
        depth_ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        depth_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
        subpass.pColorAttachments = color_refs.data();
        subpass.pDepthStencilAttachment = &depth_ref;

        vk::RenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 0;

        final_renderpass = std::make_shared<renderpass>(render_pass_create_info, window_size);

        LOG(TRACE) << "Created final renderpass";
    }

    void renderpass_manager::create_main_renderpass(vk::Extent2D& window_size) {
        /*main_renderpass = renderpass_builder()
                .set_framebuffer_size(window_size.width, window_size.height)
                .add_color_buffer()
                .build();

        LOG(TRACE) << "Created main renderpass";*/
    }

    std::shared_ptr<renderpass> renderpass_manager::get_main_renderpass() {
        return main_renderpass;
    }

    std::shared_ptr<renderpass> renderpass_manager::get_final_renderpass() {
        return final_renderpass;
    }

    void renderpass_manager::rebuild_all(vk::Extent2D &window_size) {
        create_main_renderpass(window_size);
        create_final_renderpass(window_size);
    }
}
