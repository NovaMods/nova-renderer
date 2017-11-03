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
#include "../vulkan/render_context.h"

namespace nova {
    /*!
     * \brief A renderpass object
     *
     * I've made this wrapper to better manage adding and accessing framebuffer attachments,
     */
    class renderpass {
        friend class renderpass_builder;

    public:
        explicit renderpass(vk::RenderPassCreateInfo& create_info, vk::Extent2D size);

        renderpass(renderpass &&other) noexcept;

        ~renderpass();

        vk::RenderPass get_renderpass();

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

        void create_framebuffers(vk::Extent2D size);
    };
}

#endif //RENDERER_FRAMEBUFFER_H
