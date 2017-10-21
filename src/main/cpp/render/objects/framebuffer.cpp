//
// Created by gold1 on 01-Jun-17.
//

#include "framebuffer.h"
#include "../vulkan/render_device.h"
#include <easylogging++.h>

namespace nova {
    /* framebuffer */

    framebuffer::framebuffer(unsigned int width, unsigned int height, unsigned int num_color_attachments) {
        //glCreateFramebuffers(1, &framebuffer_id);

        color_attachments = new GLuint[num_color_attachments];
        //glCreateTextures(GL_TEXTURE_2D, num_color_attachments, color_attachments);


        // TODO: Set texture format
        for(unsigned int i = 0; i < num_color_attachments; i++) {
            //glTextureStorage2D(color_attachments[i], 1, GL_RGBA8, width, height);
            //glNamedFramebufferTexture(framebuffer_id, GL_COLOR_ATTACHMENT0 + i, color_attachments[i], 0);
            color_attachments_map[i] = color_attachments[i];
        }
    }

    framebuffer::framebuffer(framebuffer&& other) {
        color_attachments = other.color_attachments;
        other.color_attachments = nullptr;

        framebuffer_id = other.framebuffer_id;
        other.framebuffer_id = 0;

        color_attachments_map = std::move(other.color_attachments_map);
        has_depth_buffer = other.has_depth_buffer;
    }

    framebuffer::~framebuffer() {
        LOG(TRACE) << "Deleting framebuffer " << framebuffer_id;
        //glDeleteTextures(color_attachments_map.size(), color_attachments);
        //glDeleteFramebuffers(1, &framebuffer_id);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebuffer::create_depth_buffer() {
        // This block just kinda checks that thhe depth bufer we want is available
        {
            vk::Format formats[] = {
                    vk::Format::eD32SfloatS8Uint,
                    vk::Format::eD24UnormS8Uint
            };
            depth_format = choose_supported_format(formats, 2, vk::ImageTiling::eOptimal,
                                                   vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }

        // TODO: Actually create the depth buffer. The tutorial doesn't mention that at this point
        has_depth_buffer = true;
    }

    void framebuffer::bind() {
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
    }

    void framebuffer::enable_writing_to_attachment(unsigned int attachment) {
        drawbuffers.insert(GL_COLOR_ATTACHMENT0 + attachment);
    }

    void framebuffer::reset_drawbuffers() {
        drawbuffers.clear();
    }

    void framebuffer::generate_mipmaps() {
        for(const auto& item : color_attachments_map) {
            //glGenerateTextureMipmap(item.second);
        }
    }

    vk::Format framebuffer::choose_supported_format(vk::Format *formats, int num_formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for(int i = 0; i < num_formats; i++) {
            vk::Format& format = formats[i];

            vk::FormatProperties props = render_device::instance.physical_device.getFormatProperties(format);

            if(tiling == vk::ImageTiling::eLinear and (props.linearTilingFeatures & features) == features) {
                return format;
            } else if(tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        LOG(FATAL) << "Failed to fine a suitable depth buffer format";
    }

    /* framebuffer_builder */

    framebuffer_builder& framebuffer_builder::set_framebuffer_size(unsigned int width, unsigned int height) {
        this->width = width;
        this->height = height;

        return *this;
    }

    framebuffer_builder& framebuffer_builder::enable_color_attachment(unsigned int color_attachment) {
        enabled_color_attachments.insert(color_attachment);

        return *this;
    }

    framebuffer_builder& framebuffer_builder::disable_color_attachment(unsigned int color_attachment) {
        auto attachment_itr = enabled_color_attachments.find(color_attachment);
        if(attachment_itr != enabled_color_attachments.end()) {
            enabled_color_attachments.erase(attachment_itr);
        }

        return *this;
    }

    framebuffer framebuffer_builder::build() {
        auto num_color_attachments = *enabled_color_attachments.rbegin();

        return framebuffer(width, height, num_color_attachments);
    }

    void framebuffer_builder::reset() {
        enabled_color_attachments.clear();

        width = 0;
        height = 0;
    }
}
