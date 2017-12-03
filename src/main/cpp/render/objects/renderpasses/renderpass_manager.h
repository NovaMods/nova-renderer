/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#ifndef RENDERER_RENDER_PASS_MANAGER_H
#define RENDERER_RENDER_PASS_MANAGER_H

#include <glm/glm.hpp>

namespace nova {
    class shaderpack;

    /*!
     * \brief Holds all the render passes that we made from the loaded shaderpack
     *
     * Nova explicitly defines a few renderpasses:
     *  - Virtual texture pass to see what textures are needed
     *  - shadows
     *  - Gbuffers and lighting
     *      - Two options for the pass:
     *          - if the shaderpack uses deferres passes, opaques in one subpass, then block lights, then deferred
     *          passes, then transparents
     *          - If the shaderpack does not use deferred passes, opaques in one subpass, then transparents, then block l
     *          ights
     *      - Might want to use subpasses for the composites as well
     *  - Final
     *
     * each of those is split into subpases based on render type (fully opaque, cutout, transparent, etc)
     */
    class renderpass_manager {
    public:
        renderpass_manager(const vk::Extent2D& main_shadow_size, const vk::Extent2D& light_shadow_size, const vk::Extent2D& window_size);

        /*!
         * \brief Rebuilds the entire renderpasses
         *
         * Renderpasses are dependent on the framebuffer size so there we go
         *
         * And since there's new renderpasses we'll need to rebuild our pipelines as well...
         *
         * \param window_size The size of the window we're rendering to
         */
        void rebuild_all(const vk::Extent2D& main_shadow_size, const vk::Extent2D& light_shadow_size, const vk::Extent2D& window_size);

        const vk::RenderPass get_main_renderpass() const;
        const vk::RenderPass get_final_renderpass() const;

    private:
        vk::RenderPass main_shadow_pass;
        vk::RenderPass light_shadow_pass;
        vk::RenderPass main_pass;
        vk::RenderPass final_pass;

        std::vector<vk::Framebuffer> final_framebuffers;

        void create_final_renderpass(const vk::Extent2D& window_size);

        void create_main_renderpass(const vk::Extent2D& window_size);

        void create_final_framebuffers(const vk::Extent2D &window_size);
    };
}

#endif //RENDERER_RENDER_PASS_MANAGER_H
