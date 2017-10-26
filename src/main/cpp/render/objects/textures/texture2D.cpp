//
// Created by David on 26-Apr-16.
//

#include "texture2D.h"
#include <stdexcept>
#include <easylogging++.h>
#include <vk_mem_alloc.h>
#include "../../../utils/utils.h"

namespace nova {
    texture2D::texture2D() : size(0) {
        //glGenTextures(1, &gl_name);
    }

    void texture2D::set_data(void* pixel_data, glm::u32vec2 &dimensions, vk::Format format, GLenum type, GLenum internal_format) {
        auto& context = render_context::instance;
        vk::ImageCreateInfo image_create_info = {};
        image_create_info.samples = vk::SampleCountFlagBits::e1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.extent = vk::Extent3D{dimensions.x, dimensions.y, 1};
        image_create_info.tiling = vk::ImageTiling::eLinear;    // Easier to cram data into I think?

        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        VkImage vk_image = image;
        //vmaCreateImage(context.allocator, &image_create_info, nullptr, &vk_image, nullptr, nullptr);
        image = vk_image;

        GLint previous_texture;
        /*glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
        glBindTexture(GL_TEXTURE_2D, gl_name);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, dimensions.x, dimensions.y, 0, format, type, pixel_data);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glBindTexture(GL_TEXTURE_2D, (GLuint) previous_texture);*/
    }

    void texture2D::bind(unsigned int binding) {
        //glBindTextureUnit(binding, gl_name);
        //current_location = binding;
    }

    void texture2D::unbind() {
        //glActiveTexture((GLenum) current_location);
        //glBindTexture(GL_TEXTURE_2D, 0);
        //current_location = -1;
    }

    int texture2D::get_width() {
        return size.x;
    }

    int texture2D::get_height() {
        return size.y;
    }

    GLint texture2D::get_format() {
        return 0;
    }

    void texture2D::set_filtering_parameters(texture_filtering_params &params) {
        // TODO
    }

    const unsigned int texture2D::get_gl_name() {
        return 0;
    }

    void texture2D::set_name(const std::string name) {
        this->name = name;
    }

    const std::string &texture2D::get_name() const {
        return name;
    }
}
