//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include <memory>
#include <thread>
#include <view/objects/mesh_accessor.h>
#include <data_loading/data_model.h>
#include <view/objects/shaders/shader_facade.h>
#include "utils/export.h"

#include "interfaces/iwindow.h"
#include "mc_interface/mc_objects.h"

#include "mc_interface/nova.h"
#include "data_loading/texture_manager.h"
#include "data_loading/settings.h"
#include "view/uniform_buffer_store.h"
#include "view/windowing/glfw_gl_window.h"

namespace nova {
    namespace view {
        /*!
         * \brief Initializes everything this mod needs, creating its own window
         *
         * Idea: Replace the Display class too, maybe?
         *
         * This class is kinda a facade and kinda a God class that holds all the everything that the mod needs. I'd like it to
         * be more of a facade but idk. Facades are hard.
         *
         * This class's instance runs completely in a separate thread. Whatever you want to use it for, it runs in a separate
         * thread. Calling froman application with a million threads already? Too bad, separate thread.
         *
         * I'm not worried about data races. Data moves into this thread, then gets rendered. The only data racey thing are the
         * flags that specify rendering commands are available. However, I'm using atomics for those, so I don't expect too many
         * problems. If I notice the renderer missing render commands, I'll re-evaluate the data integrity scheme
         */
        class NOVA_API nova_renderer {
        public:
            /*!
             * \brief A singleton for the nova_renderer instance
             *
             * I want only one nova_renderer active at a time, and the C code needs a good way to reference the nova_renderer
             * instance. A singleton fulfils both those requirements.
             */
            static std::unique_ptr<nova_renderer> instance;

            /*!
             * \brief Initializes the static instance of the Nova renderer
             */
            static void init_instance();

            /*!
             * \brief Initiazes the nova_renderer
             *
             * Initializing the nova_renderer is a lot of work. I create an OpenGL context, create a GLFW window, initialize
             * the texture manager, shader manager, UBO manager, etc. and set up initial OpenGL state.
             */
            nova_renderer();

            /*!
             * \brief Destructor
             */
            ~nova_renderer();

            /*!
             * \brief Tells all the data holding things to check for new data
             */
            void update();

            /*!
             * \brief Renders a single frame
             *
             * This method runs in a separate thread from the rest of the methods in this class. This is because the other
             * methods are called by Minecraft, from the Minecraft thread, while this method is run in a separate thread. This
             * is done to keep the OpenGL contexts separate. Hopefully it'll work
             */
            void render_frame();

            /*!
             * \brief determines whether or not the Nova Renderer, and by extension Minecraft, should shut down. Called directly
             * by the C interface
             *
             * \return True if we should shutdown, false otherwise
             */
            bool should_end();

            model::data_model& get_model();

        private:
            model::data_model model;

            glfw_gl_window game_window;

            mesh_accessor meshes;

            uniform_buffer_store ubo_manager;

            shader_facade shaders;

            /*!
             * \brief Renders the GUI of Minecraft
             */
            void render_gui();

            void render_shadow_pass();

            void render_gbuffers();

            void render_composite_passes();

            void render_final_pass();

            void enable_debug();

            void init_opengl_state() const;

            /*!
             * \brief Checks if any new shaders have been loaded, and uploads them to the GPU
             */
            void check_for_new_shaders();
        };

        void link_up_uniform_buffers(std::unordered_map<std::string, gl_shader_program>& shaders, const uniform_buffer_store &ubos);
    }
}

#endif //RENDERER_VULKAN_MOD_H
