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
#include "objects/resources/shader_resource_manager.h"
#include "objects/renderpasses/render_passes.h"
#include "objects/renderpasses/renderpass_builder.h"
#include "objects/shaders/pipeline.h"
#include "vulkan/command_pool.h"
#include "objects/resources/texture_manager.h"
#include "swapchain_manager.h"

namespace nova {
    class uniform_buffer_store;
    class glfw_vk_window;
    class mesh_store;
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
         * \brief A singleton for the nova_renderer instanceA
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

        input_handler& get_input_handler();

        glfw_vk_window& get_game_window();

        mesh_store& get_mesh_store();

        camera& get_player_camera();

        std::shared_ptr<render_context> get_render_context();

        std::vector<material>& get_materials();

        std::shared_ptr<shader_resource_manager> get_shader_resources();

        /*
         * Overrides from iconfig_listener
         */

        void on_config_change(nlohmann::json& new_config) override;

        void on_config_loaded(nlohmann::json& config) override;

    private:

        /*
         * Singletons
         */

		static std::shared_ptr<settings> render_settings;

        std::shared_ptr<render_context> context;

        std::shared_ptr<glfw_vk_window> game_window;

        std::shared_ptr<input_handler> inputs;

        std::shared_ptr<mesh_store> meshes;

        std::shared_ptr<shader_resource_manager> shader_resources;

        std::shared_ptr<swapchain_manager> swapchain;

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
        std::unordered_map<std::string, std::vector<pipeline_data>> pipelines_by_pass;
        std::unordered_map<std::string, pass_vulkan_information> renderpasses_by_pass;
        std::unordered_map<std::string, std::vector<pipeline_object>> pipelines_by_renderpass;
        std::vector<material> materials;
        std::unordered_map<std::string, std::vector<material_pass>> material_passes_by_pipeline;

        /*
         * Housekeeping
         */

        vk::Semaphore render_finished_semaphore;

        // Will be replaced when I merge in the render graph, for now this is kinda a hack to let me validate the rest
        // of the Nova Vulkan code
        vk::Fence render_done_fence;

        /*
         * Internal variables
         *
         * This is the stuff that might be exposed to the scripting environment or the shader environment
         */

        camera player_camera;

        glm::mat4x4 gui_model;

        /**
         * Private functions
         */

        /*
         * Data loading
         */

        void load_new_shaderpack(const std::string &new_shaderpack_name);

        std::unordered_map<std::string, std::vector<material_pass>> extract_material_passes(const std::vector<material>& materials);

        /*
         * Render loop
         */

        void begin_frame();

        void execute_pass(const render_pass &pass, vk::CommandBuffer& buffer);

        void render_pipeline(pipeline_object &pipeline_data, vk::CommandBuffer& buffer);

        void render_all_for_material_pass(const material_pass& pass, vk::CommandBuffer &buffer, pipeline_object &info);

        void
        render_mesh(const render_object &mesh, vk::CommandBuffer &buffer, pipeline_object &info, std::string string);

        void end_frame();

        /*
         * Shader parameters
         */

        /*!
         * \brief Updated all the UBOs that Nova knows how to update
         *
         * Updates the per-model buffer, the buffer of per-frame uniforms, and anything else I deem necessary and proper
         * to the execution of a well-designed renderer
         */
        void update_nova_ubos();

        /*!
         * \brief Calculates the GUI model matrix for the current frame, then copies that to each GUI object's
         * allocation in the per-model buffer
         *
         * It would be more performant to have each GUI object share an allocation in a buffer, but there's only a
         * handful of GUI objects so I'm not super worried
         */
        void update_gui_model_matrices();

        /*!
         * \brief Sends the provided model matrix to the given render object's allocation in the per-model buffer
         *
         * \param gui_obj The render_object for the GUI object to set the model matrix of
         * \param model_matrix The GUI model matrix
         */
        inline void update_gui_model_matrix(const render_object& gui_obj, const glm::mat4& model_matrix, const vk::Device& device);

        /*!
         * \brief Calculates the object's model matrix from its position and eventually rotation, then uploads it to
         * this model's allocation in the per-model buffer
         *
         * \param renderable The render_object to calculate and upload the model matrix for
         */
        void update_model_matrix(const render_object &renderable);

        void
    insert_special_geometry(const std::unordered_map<std::string, std::vector<material_pass>> &material_passes_by_pipeline);
    };

    std::vector<render_pass> compile_into_list(std::unordered_map<std::string, render_pass> passes);
}

#endif //RENDERER_VULKAN_MOD_H
