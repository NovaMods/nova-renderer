//
// Created by gold1 on 01-Jun-17.
//

#include "renderpass.h"
#include <easylogging++.h>

namespace nova {
    renderpass::renderpass(vk::RenderPassCreateInfo& create_info, vk::Extent2D size) {
        vk_renderpass = render_context::instance.device.createRenderPass(create_info);
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

    vk::RenderPass renderpass::get_renderpass() {
        return vk_renderpass;
    }
}
