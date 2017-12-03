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
     */
    class renderpass {
        friend class renderpass_builder;

    public:
        renderpass(vk::RenderPassCreateInfo& create_info, vk::Extent2D size);

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
    };
}

#endif //RENDERER_FRAMEBUFFER_H
