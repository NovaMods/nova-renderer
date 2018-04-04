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
#include "objects/shaders/shader_resource_manager.h"
#include "objects/renderpasses/render_passes.h"
#include "objects/renderpasses/renderpass_builder.h"
#include "objects/shaders/pipeline_creation.h"

namespace nova {
    class vk_shader_program;
    class uniform_buffer_store;
    class glfw_vk_window;
    class mesh_store;
    class texture_manager;
    class input_handler;
    class render_context;
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

        std::shared_ptr<render_context> get_render_context();

        std::shared_ptr<shaderpack> get_shaders();

        std::shared_ptr<shader_resource_manager> get_shader_resources();

        // Overrides from iconfig_listener

        void on_config_change(nlohmann::json& new_config) override;

        void on_config_loaded(nlohmann::json& config) override;

    private:

        /*
         * Singletons
         */

		static std::shared_ptr<settings> render_settings;

        std::shared_ptr<render_context> context;

        std::shared_ptr<glfw_vk_window> game_window;

        std::shared_ptr<texture_manager> textures;

        std::shared_ptr<input_handler> inputs;

        std::shared_ptr<mesh_store> meshes;

        std::shared_ptr<uniform_buffer_store> ubo_manager;

        std::shared_ptr<shader_resource_manager> shader_resources;

        /*
         * Swapchain bs
         */

        uint32_t cur_swapchain_image_index = 0;

        vk::Semaphore swapchain_image_acquire_semaphore;

        /*
         * Shaderpack data
         */

        std::string loaded_shaderpack_name;

        std::vector<render_pass> passes_list;
        std::unordered_map<std::string, std::vector<pipeline>> pipelines_by_pass;
        std::unordered_map<std::string, pass_vulkan_information> renderpasses_by_pass;
        std::unordered_map<std::string, std::vector<pipeline_info>> pipelines_by_renderpass;

        /*
         * Housekeeping
         */

        vk::Semaphore render_finished_semaphore;

        // Will be replaced when I merge in the render graph, for now this is kinda a hack to let me validate the rest
        // of the Nova Vulkan code
        vk::Fence render_done_fence;

        /*
         * Internal variables
         */

        camera player_camera;

        /*
         * Private functions
         */

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

        inline void upload_gui_model_matrix(const render_object& gui_obj, const glm::mat4& model_matrix);

        void update_gbuffer_ubos();

        void end_frame();

        void begin_frame();

        void update_gui_model_matrices();

        glm::mat4x4 gui_model;
    };

    std::vector<render_pass> compile_into_list(std::unordered_map<std::string, render_pass> passes);
}

#endif //RENDERER_VULKAN_MOD_H
