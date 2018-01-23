//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"
#include "../utils/utils.h"
#include "../data_loading/loaders/loaders.h"
#include "objects/render_object.h"
#include "../data_loading/settings.h"
#include "windowing/glfw_vk_window.h"
#include "vulkan/command_pool.h"
#include "objects/meshes/mesh_store.h"
#include "objects/render_object.h"
#include "objects/meshes/vk_mesh.h"
#include "objects/uniform_buffers/uniform_buffer_definitions.h"
#include "objects/uniform_buffers/uniform_buffer_store.h"
#include "../input/InputHandler.h"
#include "objects/renderpasses/renderpass_manager.h"
#include "vulkan/render_context.h"

#include <easylogging++.h>
#include <glm/gtc/matrix_transform.hpp>
#include <minitrace.h>

INITIALIZE_EASYLOGGINGPP

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;
    std::shared_ptr<settings> nova_renderer::render_settings;

    nova_renderer::nova_renderer() {
        game_window = std::make_shared<glfw_vk_window>();
        LOG(TRACE) << "Window initialized";

        render_context::instance.create_instance(*game_window);
        LOG(TRACE) << "Instance created";
        render_context::instance.setup_debug_callback();
        LOG(TRACE) << "Debug callback set up";
        game_window->create_surface();
        LOG(TRACE) << "Created surface";
        render_context::instance.find_device_and_queues();
        LOG(TRACE) << "Found device and queue";
        render_context::instance.create_semaphores();
        LOG(TRACE) << "Created semaphores";
        render_context::instance.create_command_pool_and_command_buffers();
        LOG(TRACE) << "Created command pool";
        render_context::instance.create_swapchain(game_window->get_size());
        LOG(TRACE) << "Created swapchain";
        render_context::instance.create_pipeline_cache();
        LOG(TRACE) << "Pipeline cache created";

        LOG(INFO) << "Vulkan code initialized";

        ubo_manager = std::make_shared<uniform_buffer_store>();
        textures = std::make_shared<texture_manager>();
        meshes = std::make_shared<mesh_store>();
        inputs = std::make_shared<input_handler>();

		render_settings->register_change_listener(ubo_manager.get());
		render_settings->register_change_listener(game_window.get());
        render_settings->register_change_listener(this);

        render_settings->update_config_loaded();
		render_settings->update_config_changed();

        LOG(INFO) << "Finished sending out initial config";

        vk::SemaphoreCreateInfo create_info = {};
        swapchain_image_acquire_semaphore = render_context::instance.device.createSemaphore(create_info);

        context = &render_context::instance;
    }

    nova_renderer::~nova_renderer() {
        inputs.reset();
        meshes.reset();
        textures.reset();
        ubo_manager.reset();

        render_context::instance.vk_instance.destroy();
        game_window.reset();

        mtr_shutdown();
    }

    void nova_renderer::render_frame() {
        begin_frame();

        auto main_command_buffer = context->command_buffer_pool->get_command_buffer(0);

        vk::CommandBufferBeginInfo cmd_buf_begin_info = {};
        main_command_buffer.buffer.begin(cmd_buf_begin_info);

        player_camera.recalculate_frustum();

        // Make geometry for any new chunks
        meshes->upload_new_geometry();


        // upload shadow UBO things

        render_shadow_pass();

        // main_framebuffer->bind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        update_gbuffer_ubos();

        render_gbuffers(main_command_buffer.buffer);

        render_composite_passes();

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_final_pass();

        // We want to draw the GUI on top of the other things, so we'll render it last
        // Additionally, I could use the stencil buffer to not draw MC underneath the GUI. Could be a fun
        // optimization - I'd have to watch out for when the user hides the GUI, though. I can just re-render the
        // stencil buffer when the GUI screen changes
        render_gui(main_command_buffer.buffer);

        main_command_buffer.buffer.end();

        vk::SubmitInfo submit_info = {};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &main_command_buffer.buffer;
        submit_info.pWaitSemaphores = &swapchain_image_acquire_semaphore;
        submit_info.waitSemaphoreCount = 1;
        // context->graphics_queue.submit(1, &submit_info, main_command_buffer.fences[0]);

        end_frame();
    }

    void nova_renderer::render_shadow_pass() {
        LOG(TRACE) << "Rendering shadow pass";
    }

    void nova_renderer::render_gbuffers(vk::CommandBuffer buffer) {
        LOG(TRACE) << "Rendering gbuffer pass";

        // TODO: Get shaders with gbuffers prefix, draw transparents last, etc
        auto& terrain_shader = loaded_shaderpack->get_shader("gbuffers_terrain");
        render_shader(buffer, terrain_shader);
        auto& water_shader = loaded_shaderpack->get_shader("gbuffers_water");
        render_shader(buffer, water_shader);
    }

    void nova_renderer::render_composite_passes() {
        LOG(TRACE) << "Rendering composite passes";
    }

    void nova_renderer::render_final_pass() {
        LOG(TRACE) << "Rendering final pass";
        //meshes->get_fullscreen_quad->set_active();
        //meshes->get_fullscreen_quad->draw();
    }

    void nova_renderer::render_gui(vk::CommandBuffer command) {
        LOG(TRACE) << "Rendering GUI";
        //glClear(GL_DEPTH_BUFFER_BIT);

        // Bind all the GUI data
        auto &gui_shader = loaded_shaderpack->get_shader("gui");

        upload_gui_model_matrix(gui_shader);

        // Render GUI objects
        std::vector<render_object>& gui_geometry = meshes->get_meshes_for_shader("gui");
        for(const auto& geom : gui_geometry) {
            if (!geom.color_texture.empty()) {
                auto color_texture = textures->get_texture(geom.color_texture);
                color_texture.bind(0);
            }
            geom.geometry->set_active(command);
            geom.geometry->draw();
        }
    }

    bool nova_renderer::should_end() {
        // If the window wants to close, the user probably clicked on the "X" button
        return game_window->should_close();
    }

    void nova_renderer::init() {
        mtr_init("nova_profile.json");
        MTR_META_PROCESS_NAME("Nova Renderer")
        MTR_META_THREAD_NAME("Main Nova Thread")

        MTR_SCOPE("INIT", "MainInit")
        render_settings = std::make_shared<settings>("config/config.json");

        try {
            instance = std::make_unique<nova_renderer>();
        } catch(std::exception& e) {
            LOG(ERROR) << "Could not initialize Nova cause " << e.what();
        }
    }

    void nova_renderer::on_config_change(nlohmann::json &new_config) {
		auto& shaderpack_name = new_config["loadedShaderpack"];
        LOG(INFO) << "Shaderpack in settings: " << shaderpack_name;

        if(!loaded_shaderpack) {
            LOG(DEBUG) << "There's currently no shaderpack, so we're loading a new one";
            load_new_shaderpack(shaderpack_name);
            return;
        }

        bool shaderpack_in_settings_is_new = shaderpack_name != loaded_shaderpack->get_name();
        if(shaderpack_in_settings_is_new) {
            LOG(DEBUG) << "Shaderpack " << shaderpack_name << " is about to replace shaderpack " << loaded_shaderpack->get_name();
            load_new_shaderpack(shaderpack_name);
        }

        LOG(DEBUG) << "Finished dealing with possible new shaderpack";
    }

    void nova_renderer::on_config_loaded(nlohmann::json &config) {
        // TODO: Probably want to do some setup here, don't need to do that now
    }

    settings &nova_renderer::get_render_settings() {
        return *render_settings;
    }

    texture_manager &nova_renderer::get_texture_manager() {
        return *textures;
    }

	glfw_vk_window &nova_renderer::get_game_window() {
		return *game_window;
	}

	input_handler &nova_renderer::get_input_handler() {
		return *inputs;
	}

    mesh_store &nova_renderer::get_mesh_store() {
        return *meshes;
    }

    void nova_renderer::load_new_shaderpack(const std::string &new_shaderpack_name) {
		LOG(INFO) << "Loading a new shaderpack named " << new_shaderpack_name;

        auto shader_definitions = load_shaderpack(new_shaderpack_name);

        LOG(DEBUG) << "Shaderpack loaded, wiring everything together";

        vk::Extent2D im_lazy;

        renderpasses = std::make_shared<renderpass_manager>(im_lazy, im_lazy, context->swapchain_extent);

        loaded_shaderpack = std::make_shared<shaderpack>(new_shaderpack_name, shader_definitions, renderpasses->get_final_renderpass());

        LOG(INFO) << "Loading complete";
		
        link_up_uniform_buffers(loaded_shaderpack->get_loaded_shaders(), ubo_manager);
        LOG(DEBUG) << "Linked up UBOs";
    }

    void nova_renderer::deinit() {
        instance.release();
    }

    void nova_renderer::render_shader(vk::CommandBuffer buffer, gl_shader_program &shader) {
        LOG(TRACE) << "Rendering everything for shader " << shader.get_name();

        MTR_SCOPE("RenderLoop", "render_shader");

        auto& geometry = meshes->get_meshes_for_shader(shader.get_name());
        LOG(INFO) << "Rendering " << geometry.size() << " things";

        MTR_BEGIN("RenderLoop", "process_all");
        for(auto& geom : geometry) {

            // if(!player_camera.has_object_in_frustum(geom.bounding_box)) {
            //     continue;
            // }

            if(geom.geometry->has_data()) {
                if(!geom.color_texture.empty()) {
                    auto color_texture = textures->get_texture(geom.color_texture);
                    color_texture.bind(0);
                }

                if(geom.normalmap) {
                    textures->get_texture(*geom.normalmap).bind(1);
                }

                if(geom.data_texture) {
                    textures->get_texture(*geom.data_texture).bind(2);
                }

                upload_model_matrix(geom, shader);

                geom.geometry->set_active(buffer);
                geom.geometry->draw();
            } else {
                LOG(TRACE) << "Skipping some geometry since it has no data";
            }
        }
        MTR_END("RenderLoop", "process_all")
    }

    inline void nova_renderer::upload_model_matrix(render_object &geom, gl_shader_program &program) const {
        glm::mat4 model_matrix = glm::translate(glm::mat4(1), geom.position);

        //glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, &model_matrix[0][0]);
    }

    void nova_renderer::upload_gui_model_matrix(gl_shader_program &program) {
        auto config = render_settings->get_options()["settings"];
        float view_width = config["viewWidth"];
        float view_height = config["viewHeight"];
        float scalefactor = config["scalefactor"];
        // The GUI matrix is super simple, just a viewport transformation
        glm::mat4 gui_model(1.0f);
        gui_model = glm::translate(gui_model, glm::vec3(-1.0f, 1.0f, 0.0f));
        gui_model = glm::scale(gui_model, glm::vec3(scalefactor, scalefactor, 1.0f));
        gui_model = glm::scale(gui_model, glm::vec3(1.0 / view_width, 1.0 / view_height, 1.0));
        gui_model = glm::scale(gui_model, glm::vec3(1.0f, -1.0f, 1.0f));


        //glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, &gui_model[0][0]);
    }

    void nova_renderer::update_gbuffer_ubos() {
        // Big thing here is to update the camera's matrices

        auto& per_frame_ubo = ubo_manager->get_per_frame_uniforms();

        auto per_frame_uniform_data = per_frame_uniforms{};
        per_frame_uniform_data.gbufferProjection = player_camera.get_projection_matrix();
        per_frame_uniform_data.gbufferModelView = player_camera.get_view_matrix();

        per_frame_ubo.send_data(per_frame_uniform_data);
    }

    camera &nova_renderer::get_player_camera() {
        return player_camera;
    }

    std::shared_ptr<shaderpack> nova_renderer::get_shaders() {
        return loaded_shaderpack;
    }

    void nova_renderer::end_frame() {
        vk::Result swapchain_result = {};

        vk::PresentInfoKHR present_info = {};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &render_context::instance.swapchain;
        present_info.pImageIndices = &cur_swapchain_image_index;
        present_info.pResults = &swapchain_result;

        render_context::instance.present_queue.presentKHR(present_info);
    }

    void nova_renderer::begin_frame() {
        cur_swapchain_image_index = render_context::instance.device.acquireNextImageKHR(render_context::instance.swapchain,
                                                                               std::numeric_limits<uint32_t>::max(),
                                                                               swapchain_image_acquire_semaphore,
                                                                               vk::Fence()).value;
    }

    void link_up_uniform_buffers(std::unordered_map<std::string, gl_shader_program> &shaders, std::shared_ptr<uniform_buffer_store> ubos) {
        for(auto& shader : shaders) {
            ubos->register_all_buffers_with_shader(shader.second);
        }
    }
}

