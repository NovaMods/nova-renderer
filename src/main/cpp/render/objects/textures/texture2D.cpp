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

    void texture2D::set_data(void* pixel_data, glm::u32vec2 &dimensions, vk::Format format) {
        auto& context = render_context::instance;
        vk::ImageCreateInfo image_create_info = {};
        image_create_info.samples = vk::SampleCountFlagBits::e1;
        image_create_info.imageType = vk::ImageType::e2D;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.extent = vk::Extent3D{dimensions.x, dimensions.y, 1};
        image_create_info.tiling = vk::ImageTiling::eLinear;    // Easier to cram data into I think?
        image_create_info.format = format;
        image_create_info.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        image_create_info.queueFamilyIndexCount = 1;
        image_create_info.pQueueFamilyIndices = &context.graphics_family_idx;
        image_create_info.initialLayout = vk::ImageLayout::eTransferDstOptimal;

        VmaAllocationCreateInfo alloc_create_info = {};
        alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        auto props = context.physical_device.getFormatProperties(format);
        auto masked_features = props.linearTilingFeatures & vk::FormatFeatureFlagBits::eTransferDstKHR;
        if(masked_features == vk::FormatFeatureFlagBits::eTransferDstKHR) {
            LOG(INFO) << "Desired format is supported as a transfer source";
        } else {
            LOG(ERROR) << "Desired format is not supported as a transfer source :(";
        }

        VkImage vk_image = image;
        vmaCreateImage(context.allocator, reinterpret_cast<VkImageCreateInfo*>(&image_create_info), &alloc_create_info, &vk_image, &allocation, nullptr);
        image = vk_image;

        if(vk_image == VK_NULL_HANDLE) {
            LOG(FATAL) << "Could not create image";
        }

        vk::ImageViewCreateInfo img_view_create_info = {};
        img_view_create_info.image = image;
        img_view_create_info.viewType = vk::ImageViewType::e2D;
        img_view_create_info.format = format;

        image_view = context.device.createImageView(img_view_create_info);

        LOG(DEBUG) << "Created new image";

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
