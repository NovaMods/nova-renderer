/*!
 * \author gold1
 * \date 01-Jun-17.
 */

#ifndef RENDERER_FRAMEBUFFER_H
#define RENDERER_FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <set>
#include <vulkan/vulkan.hpp>
#include "../vulkan/render_device.h"

namespace nova {
    /*!
     * \brief A renderpass object
     *
     * I've made this wrapper to better manage adding and accessing framebuffer attachments,
     */
    class renderpass {
        friend class renderpass_builder;

    public:
        explicit renderpass(vk::RenderPassCreateInfo& create_info, glm::ivec2 size);

        renderpass(renderpass &&other) noexcept;

        ~renderpass();

    private:
        bool has_depth_buffer = false;

        vk::RenderPass vk_renderpass;

        // TODO: Figure out how to associate framebuffers with subpasses and how to manage swapping out shadow
        // framebuffers for, you know, shadows
        // Maybe I want to have a shadow_renderer that handles all that shadow stuff?
        // TODO: I don't always want to create one framebuffer per swapchain image. The final framebuffer is a special
        // case and I'm not sure how to handle it
        vk::Framebuffer framebuffer;

        std::vector<vk::ImageView> color_image_views;
        vk::ImageView depth_buffer_view;

        void create_framebuffers(glm::ivec2 size);
    };

    /*!
     * \brief A builder for renderbuffers
     */
    class renderpass_builder {
    public:
        /*!
         * \brief Sets the framebuffer size to the specified value
         *
         * \param width The desired width of the framebuffer
         * \param height The desired height of the framebuffer
         * \return This framebuffer_builder object
         */
        renderpass_builder &set_framebuffer_size(unsigned int width, unsigned int height);

        /*!
         * \brief Adds a color buffer to the renderpass
         *
         * Insertion order is maintained
         */
        renderpass_builder &add_color_buffer();

        /*!
         * \brief Creates a framebuffer with the settings specified in this framebuffer_builder
         *
         * \return The newly constructed framebuffer object
         */
        std::unique_ptr<renderpass> build();

        /*!
         * \brief Resets this framebuffer builder, clearing all the information stored within
         */
        void reset();

    private:
        unsigned int width;
        unsigned int height;

        std::vector<vk::AttachmentDescription> attachments;
        std::vector<vk::AttachmentReference> color_refs;
    };
}

#endif //RENDERER_FRAMEBUFFER_H
