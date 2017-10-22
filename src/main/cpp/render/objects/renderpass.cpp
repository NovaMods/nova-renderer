//
// Created by gold1 on 01-Jun-17.
//

#include "renderpass.h"
#include "../vulkan/render_device.h"
#include <easylogging++.h>

namespace nova {
    /* framebuffer */

    renderpass::renderpass(vk::RenderPassCreateInfo& create_info) {
        vk_renderpass = render_device::instance.device.createRenderPass(create_info);
    }

    renderpass::renderpass(renderpass&& other) noexcept {
        color_attachments = other.color_attachments;
        other.color_attachments = nullptr;

        framebuffer_id = other.framebuffer_id;
        other.framebuffer_id = 0;

        has_depth_buffer = other.has_depth_buffer;
    }

    renderpass::~renderpass() {
        LOG(TRACE) << "Deleting framebuffer " << framebuffer_id;
        //glDeleteTextures(color_attachments_map.size(), color_attachments);
        //glDeleteFramebuffers(1, &framebuffer_id);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderpass::create_depth_buffer() {
                // TODO: Actually create the depth buffer. The tutorial doesn't mention that at this point
        has_depth_buffer = true;
    }

    void renderpass::bind() {
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
    }

    void renderpass::enable_writing_to_attachment(unsigned int attachment) {
        drawbuffers.insert(GL_COLOR_ATTACHMENT0 + attachment);
    }

    void renderpass::reset_drawbuffers() {
        drawbuffers.clear();
    }

    void renderpass::generate_mipmaps() {
    }

    /* renderpass_builder */

    renderpass_builder& renderpass_builder::set_framebuffer_size(unsigned int width, unsigned int height) {
        this->width = width;
        this->height = height;

        return *this;
    }

    renderpass_builder& renderpass_builder::add_color_buffer() {
        vk::AttachmentDescription color_attachment = {};
        color_attachment.format = render_device::instance.swapchain_format;
        color_attachment.samples = vk::SampleCountFlagBits::e1; // TODO: Potentially increase the sample count if using MSAA
        color_attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
        color_attachment.initialLayout = vk::ImageLayout::eUndefined;
        color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        attachments.push_back(color_attachment);

        vk::AttachmentReference ref = {};
        ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
        color_refs.push_back(ref);

        return *this;
    }

    std::unique_ptr<renderpass> renderpass_builder::build() {
        // Don't forget the depth buffer!
        vk::AttachmentDescription depth_attachment = {};
        depth_attachment.format = render_device::instance.depth_format;
        depth_attachment.samples = vk::SampleCountFlagBits::e1;
        depth_attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
        depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
        depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        attachments.push_back(depth_attachment);

        vk::AttachmentReference depth_ref = {};
        depth_ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        depth_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        // TODO: There will be all kinda of information about subpasses maybe but right now there isn't

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

        return std::make_unique<renderpass>(render_pass_create_info);
    }

    void renderpass_builder::reset() {
        attachments.clear();
        color_refs.clear();

        width = 0;
        height = 0;
    }
}
