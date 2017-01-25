//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include <memory>
#include <thread>
#include "objects/shaders/gl_shader_program.h"
#include "uniform_buffer_store.h"
#include "windowing/glfw_gl_window.h"
#include "../data_loading/geometry_cache/mesh_store.h"
#include "objects/textures/texture_manager.h"

namespace nova {
    /*!
     * \brief Initializes everything this mod needs, creating its own window
     *
     * Idea: Replace the Display class too, maybe?
     *
     * This class is kinda a facade and kinda a God class that holds all the everything that the mod needs. I'd like it to
     * be more of a facade but idk. Facades are hard.
     */
    class nova_renderer : public iconfig_listener {
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
        static void init();

        /*!
         * \brief Shuts down Nova, cleaning up anything that needs cleaning
         */
        static void deinit();

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
         * \brief Explicitly tells the render_settings to send its data to all its registered listeners
         *
         * Useful when you register a new thing and want to make sure it gets all the latest data
         */
        void trigger_config_update();

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

        settings& get_render_settings();

        texture_manager& get_texture_manager();

        mesh_store& get_mesh_store();

        // Overrides from iconfig_listener

        void on_config_change(nlohmann::json& new_config);

        void on_config_loaded(nlohmann::json& config);

    private:
        settings render_settings;

        glfw_gl_window game_window;

        std::experimental::optional<shaderpack> loaded_shaderpack;

        texture_manager textures;

        mesh_store meshes;

        uniform_buffer_store ubo_manager;

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

        void load_new_shaderpack(const std::string &new_shaderpack_name);
    };

    void link_up_uniform_buffers(std::unordered_map<std::string, gl_shader_program> &shaders, uniform_buffer_store &ubos);
}

#endif //RENDERER_VULKAN_MOD_H
