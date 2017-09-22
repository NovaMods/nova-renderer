//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include <memory>
#include <thread>
#include "objects/shaders/gl_shader_program.h"
#include "objects/uniform_buffers/uniform_buffer_store.h"
#include "windowing/glfw_gl_window.h"
#include "../geometry_cache/mesh_store.h"
#include "objects/textures/texture_manager.h"
#include "../input/InputHandler.h"
#include "objects/framebuffer.h"
#include "objects/camera.h"

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

        static settings& get_render_settings();

        texture_manager& get_texture_manager();

        input_handler& get_input_handler();

        glfw_gl_window& get_game_window();

        mesh_store& get_mesh_store();

        camera& get_player_camera();

        std::shared_ptr<shaderpack> get_shaders();

        // Overrides from iconfig_listener

        void on_config_change(nlohmann::json& new_config);

        void on_config_loaded(nlohmann::json& config);

    private:

		static std::unique_ptr<settings> render_settings; 

        std::unique_ptr<glfw_gl_window> game_window;

        std::shared_ptr<shaderpack> loaded_shaderpack;

        std::unique_ptr<texture_manager> textures;

        std::unique_ptr<input_handler> inputs;

        std::unique_ptr<mesh_store> meshes;

        std::unique_ptr<uniform_buffer_store> ubo_manager;

        std::vector<GLuint> shadow_depth_textures;
        std::unique_ptr<framebuffer> shadow_framebuffer;
        framebuffer_builder shadow_framebuffer_builder;

        std::unique_ptr<framebuffer> main_framebuffer;
        std::vector<GLuint> gbuffer_depth_textures;
        framebuffer_builder main_framebuffer_builder;

        camera player_camera;

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

        void create_framebuffers_from_shaderpack();

        /*!
         * \brief Renders all the geometry that uses the specified shader, setting up textures and whatnot
         *
         * \param shader The shader to render things with
         */
        void render_shader(gl_shader_program& shader);

        inline void upload_gui_model_matrix(gl_shader_program &program);

        void upload_model_matrix(render_object &geom, gl_shader_program &program) const;

        void update_gbuffer_ubos();
    };

    void link_up_uniform_buffers(std::unordered_map<std::string, gl_shader_program> &shaders, uniform_buffer_store &ubos);
}

#endif //RENDERER_VULKAN_MOD_H
