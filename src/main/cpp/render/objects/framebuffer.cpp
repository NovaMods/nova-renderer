//
// Created by gold1 on 01-Jun-17.
//

#include "framebuffer.h"
#include <easylogging++.h>

namespace nova {
    /* framebuffer */

    framebuffer::framebuffer(unsigned int width, unsigned int height, unsigned int num_color_attachments) {
        glCreateFramebuffers(1, &framebuffer_id);

        color_attachments = new GLuint[num_color_attachments];
        glCreateTextures(GL_TEXTURE_2D, num_color_attachments, color_attachments);


        // TODO: Set texture format
        for(unsigned int i = 0; i < num_color_attachments; i++) {
            glTextureStorage2D(color_attachments[i], 1, GL_RGBA, width, height);
            glNamedFramebufferTexture(framebuffer_id, GL_COLOR_ATTACHMENT0 + i, color_attachments[i], 0);
            color_attachments_map[i] = color_attachments[i];
        }
    }

    framebuffer::framebuffer(framebuffer&& other) {
        for(int i = 0; i < other.color_attachments_map.size(); i++) {
            color_attachments[i] = other.color_attachments[i];
            other.color_attachments[i] = 0;
        }

        framebuffer_id = other.framebuffer_id;
        other.framebuffer_id = 0;

        color_attachments_map = std::move(other.color_attachments_map);
        has_depth_buffer = other.has_depth_buffer;
    }

    framebuffer::~framebuffer() {
        LOG(TRACE) << "Deleting framebuffer " << framebuffer_id;
        glDeleteTextures(color_attachments_map.size(), color_attachments);
        glDeleteFramebuffers(1, &framebuffer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebuffer::set_depth_buffer(GLuint depth_buffer) {
        glNamedFramebufferTexture(framebuffer_id, GL_DEPTH_ATTACHMENT, depth_buffer, 0);
        has_depth_buffer = true;

        check_status();
    }

    void framebuffer::check_status() {
        auto status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        switch(status) {
            case GL_FRAMEBUFFER_COMPLETE:
                LOG(DEBUG) << "Framebuffer " << framebuffer_id << " is complete";
                break;
            case GL_FRAMEBUFFER_UNDEFINED:
                LOG(ERROR) << "Somehow didn't bind the framebuffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                LOG(ERROR) << "A necessary attachment is not initialized";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                LOG(ERROR) << "There are no images attached to the framebuffer";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                LOG(ERROR) << "Every drawing buffer has an attachment";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                LOG(ERROR) << "The framebuffrer format is not supported";
                break;
        }}

    void framebuffer::bind() {
        auto drawbuffers_array = std::vector<GLenum>(drawbuffers.begin(), drawbuffers.end());
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
        glDrawBuffers(drawbuffers_array.size(), drawbuffers_array.data());
    }

    void framebuffer::enable_writing_to_attachment(unsigned int attachment) {
        drawbuffers.insert(GL_COLOR_ATTACHMENT0 + attachment);
    }

    void framebuffer::reset_drawbuffers() {
        drawbuffers.clear();
    }

    void framebuffer::generate_mipmaps() {
        for(const auto& item : color_attachments_map) {
            glGenerateTextureMipmap(item.second);
        }
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
