//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_VULKAN_MOD_H
#define RENDERER_VULKAN_MOD_H

#include <vulkan/vulkan.hpp>
#include <memory>
#include <thread>
#include <unordered_map>
#include "objects/camera.h"
#include "../data_loading/settings.h"

namespace nova {
    class vk_shader_program;
    class uniform_buffer_store;
    class glfw_vk_window;
    class mesh_store;
    class texture_manager;
    class input_handler;
    class render_context;
    class renderpass_manager;
    class shaderpack;
    class render_object;

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
         * the texture manager, shader manager, UBO manager, etc. and set`1 up initial OpenGL state.
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

        glfw_vk_window& get_game_window();

        mesh_store& get_mesh_store();

        camera& get_player_camera();

        std::shared_ptr<shaderpack> get_shaders();

        // Overrides from iconfig_listener

        void on_config_change(nlohmann::json& new_config) override;

        void on_config_loaded(nlohmann::json& config) override;

    private:

		static std::shared_ptr<settings> render_settings;

        std::shared_ptr<glfw_vk_window> game_window;

        std::shared_ptr<shaderpack> loaded_shaderpack;

        std::shared_ptr<texture_manager> textures;

        std::shared_ptr<input_handler> inputs;

        std::shared_ptr<mesh_store> meshes;

        std::shared_ptr<uniform_buffer_store> ubo_manager;

        std::shared_ptr<renderpass_manager> renderpasses;

        uint32_t cur_swapchain_image_index;

        vk::Semaphore swapchain_image_acquire_semaphore;

        camera player_camera;

        /*!
         * \brief Renders the GUI of Minecraft
         */
        void render_gui(vk::CommandBuffer command);

        void render_shadow_pass();

        void render_gbuffers(vk::CommandBuffer buffer);

        void render_composite_passes();

        void render_final_pass();

        void load_new_shaderpack(const std::string &new_shaderpack_name);

        /*!
         * \brief Renders all the geometry that uses the specified shader, setting up textures and whatnot
         *
         * \param shader The shader to render things with
         */
        void render_shader(vk::CommandBuffer shader, vk_shader_program &program);

        inline void upload_gui_model_matrix(vk_shader_program &program);

        void upload_model_matrix(render_object &geom, vk_shader_program &program) const;

        void update_gbuffer_ubos();

        void end_frame();

        void begin_frame();

        render_context* context;
    };

    void link_up_uniform_buffers(std::unordered_map<std::string, vk_shader_program> &shaders, std::shared_ptr<uniform_buffer_store> ubos);
}

#endif //RENDERER_VULKAN_MOD_H
