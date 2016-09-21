/*!
 * \brief The entire thread for all the rendering
 *
 * \author ddubois 
 * \date 19-Sep-16.
 */

#ifndef RENDERER_RENDER_THREAD_H
#define RENDERER_RENDER_THREAD_H

#include "view/windowing/glfw_gl_window.h"
#include "adapter/adapter_facade.h"

namespace nova {
    namespace view {
        /*!
         * \brief This is the Render Thread
         *
         * The Render Thread exists outside of time and space as we know it. The Render Thread is eternal. The Render
         * Thread is forever. All hail the Render Tread!
         *
         * The render thread floats around in Render Thread memory. It renders frames as fast as it can (or at the
         * framerate the user defines, if the user is a total square). It polls the Adapter to get the current frame's
         * rendering data.
         *
         * What does that data look like? I'm not 100% sure, but it should include groups of geometry with a shader for
         * each group. It's give you all the solid geometry with the gbuffers_terrain shader, all the entities with the
         * gbuffers_entities shader, etc. It won't be super grouped by shader, though, because some shaders will almost
         * certainly be missing. So, what we'll get is an array of Render Pass description things, where each one tells
         * us how to render a given frame.
         *
         * I'll put more information about this in the Adapter code
         */
        class renderer {
        public:
            renderer(adapter::adapter_facade& adapter);

        private:
            adapter::adapter_facade& adapter;

            glfw_gl_window game_window;


            /*!
             * \brief Called every frame to, you know, render the frame
             */
            void render_frame();
        };
    }
}


#endif //RENDERER_RENDER_THREAD_H
