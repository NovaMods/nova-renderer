//
// Created by gold1 on 01-Jun-17.
//

#include "renderpass.h"
#include <easylogging++.h>

namespace nova {
    /* framebuffer */

    renderpass::renderpass(vk::RenderPassCreateInfo& create_info, glm::ivec2 size) {
        vk_renderpass = render_context::instance.device.createRenderPass(create_info);

        // TODO: Create the depth buffer
        // TODO: Pass in information about the format of the colorbuffers
        // TODO: Create the color buffers

        create_framebuffers(size);
    }

    renderpass::renderpass(renderpass&& other) noexcept {
        has_depth_buffer = other.has_depth_buffer;
        vk_renderpass = other.vk_renderpass;
    }

    renderpass::~renderpass() {
        //glDeleteTextures(color_attachments_map.size(), color_attachments);
        //glDeleteFramebuffers(1, &framebuffer_id);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderpass::create_framebuffers(glm::ivec2 size) {
        vk::ImageView attachments[color_image_views.size() + 1];
        attachments[color_image_views.size()] = depth_buffer_view;

        for(int i = 0; i < color_image_views.size(); i++) {
            attachments[i] = color_image_views[i];
        }

        vk::FramebufferCreateInfo frambuffer_create_info = {};
        frambuffer_create_info.renderPass = vk_renderpass;
        frambuffer_create_info.attachmentCount = static_cast<uint32_t>(color_image_views.size() + 1);
        frambuffer_create_info.pAttachments = attachments;
        frambuffer_create_info.width = static_cast<uint32_t>(size.x);
        frambuffer_create_info.height = static_cast<uint32_t>(size.y);
        frambuffer_create_info.layers = 1;

        framebuffer = render_context::instance.device.createFramebuffer(frambuffer_create_info);
    }

    vk::RenderPass renderpass::get_renderpass() {
        return vk_renderpass;
    }
}
