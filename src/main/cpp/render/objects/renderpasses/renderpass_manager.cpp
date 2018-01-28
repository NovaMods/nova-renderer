/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#include <vulkan/vulkan.hpp>
#include "renderpass_manager.h"
#include "../../vulkan/render_context.h"
#include <easylogging++.h>

namespace nova {
    renderpass_manager::renderpass_manager(const vk::Extent2D& main_shadow_size, const vk::Extent2D& light_shadow_size, const vk::Extent2D& window_size) {
        // TODO: Inspect each shader, figure out which ones draw to which target, and build renderpasses based on that info
        // For now I'll just create the swapchain renderpass to make sure I hae the code

        auto& framebuffer_size = render_context::instance.swapchain_extent;

        rebuild_all(main_shadow_size, light_shadow_size, framebuffer_size);
    }

    void renderpass_manager::create_final_renderpass(const vk::Extent2D& window_size) {
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

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
        subpass.pColorAttachments = color_refs.data();

        vk::RenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 0;

        final_pass = render_context::instance.device.createRenderPass(render_pass_create_info);

        LOG(TRACE) << "Created final renderpass";
    }

    void renderpass_manager::create_main_renderpass(const vk::Extent2D& window_size) {
        /*main_pass = renderpass_builder()
                .set_framebuffer_size(window_size.width, window_size.height)
                .add_color_buffer()
                .build();

        LOG(TRACE) << "Created main renderpass";*/
    }

    const vk::RenderPass renderpass_manager::get_main_renderpass() const {
        return main_pass;
    }

    const vk::RenderPass renderpass_manager::get_final_renderpass() const {
        return final_pass;
    }

    void renderpass_manager::rebuild_all(const vk::Extent2D& main_shadow_size, const vk::Extent2D& light_shadow_size, const vk::Extent2D& window_size) {
        create_main_renderpass(window_size);
        create_final_renderpass(window_size);

        create_final_framebuffers(window_size);
    }

    void renderpass_manager::create_final_framebuffers(const vk::Extent2D &window_size) {
        auto& ctx = render_context::instance;
        final_framebuffers.reserve(ctx.swapchain_images.size());

        for(size_t i = 0; i < ctx.swapchain_images.size(); i++) {
            vk::ImageView attachments[] = {
                    ctx.swapchain_images[i]
            };

            vk::FramebufferCreateInfo framebuffer_create_info = {};
            framebuffer_create_info.renderPass = final_pass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = attachments;
            framebuffer_create_info.width = window_size.width;
            framebuffer_create_info.height = window_size.height;
            framebuffer_create_info.layers = 1;

            final_framebuffers[i] = ctx.device.createFramebuffer(framebuffer_create_info);
        }
    }

    const vk::Framebuffer renderpass_manager::get_framebuffer(uint32_t framebuffer_idx) const {
        return final_framebuffers[framebuffer_idx];
    }
}
