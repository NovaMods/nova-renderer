/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#ifndef RENDERER_RENDER_PASS_MANAGER_H
#define RENDERER_RENDER_PASS_MANAGER_H

namespace nova {
    class shaderpack;
    class renderpass;

    /*!
     * \brief Holds all the render passes that we made from the loaded shaderpack
     *
     * Nova explicitly defines a few renderpasses:
     *  - Virtual texture pass to see what textures are needed
     *  - shadows
     *  - Gbuffers and lighting
     *      - Two options for the pass:
     *          - if the shaderpack uses deferres passes, opaques in one subpass, then block lights, then deferred passes, then transparents
     *          - If the shaderpack does not use deferred passes, opaques in one subpass, then transparents, then block lights
     *      - Might want to use subpasses for the composites as well
     *  - Final
     *
     * each of those is split into subpases based on render type (fully opaque, cutout, transparent, etc)
     */
    class renderpass_manager {
    public:
        explicit renderpass_manager(std::shared_ptr<shaderpack> shaders);

    private:
        void create_final_renderpass();

        std::unique_ptr<nova::renderpass> final_render_pass;
    };
}

#endif //RENDERER_RENDER_PASS_MANAGER_H
