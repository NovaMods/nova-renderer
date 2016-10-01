/*!
 * \brief
 *
 * \author ddubois 
 * \date 19-Sep-16.
 */

#include "renderer.h"

namespace nova {
    namespace view {
        renderer::renderer() {

        }

        void renderer::render_frame() {
            render_shadow_pass();

            render_gbuffers();

            render_composite_passes();

            render_final_pass();

            // We want to draw the GUI on top of the other things, so we'll render it last
            // Additionally, I could use the stencil buffer to not graw MC underneath the GUI. Could be a fun
            // optimization - I'd have to watch out for when the user hides the GUI, though. I can just re-render the
            // stencil buffer when the GUI screen changes
            render_gui();
        }

        void renderer::render_shadow_pass() {

        }

        void renderer::render_gbuffers() {

        }

        void renderer::render_composite_passes() {

        }

        void renderer::render_final_pass() {

        }

        void renderer::render_gui() {
            // Bind all the GUI data
            model.get_shader_program("gui").bind();

            model::gl_vertex_buffer& gui_geometry = geom_cache.get_gui_geometry();
            gui_geometry.draw();
        }
    }
}
