//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"
#include "../utils/utils.h"
#include "../data_loading/loaders/loaders.h"
#include "../utils/profiler.h"

#include <easylogging++.h>
#include <glm/gtc/matrix_transform.hpp>

INITIALIZE_EASYLOGGINGPP

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    nova_renderer::nova_renderer() {
        game_window = std::make_unique<glfw_vk_window>();
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

        LOG(INFO) << "Vulkan code initialized";

        ubo_manager = std::make_unique<uniform_buffer_store>();
        textures = std::make_unique<texture_manager>();
        meshes = std::make_unique<mesh_store>();
        inputs = std::make_unique<input_handler>();

		render_settings->register_change_listener(ubo_manager.get());
		render_settings->register_change_listener(game_window.get());
        render_settings->register_change_listener(this);

        render_settings->update_config_loaded();
		render_settings->update_config_changed();

        LOG(INFO) << "Finished sending out initial config";
    }

    nova_renderer::~nova_renderer() {
        inputs.reset();
        meshes.reset();
        textures.reset();
        ubo_manager.reset();

        render_context::instance.vk_instance.destroy();
        game_window.reset();
    }

    void nova_renderer::render_frame() {
        profiler::log_all_profiler_data();
        player_camera.recalculate_frustum();

        // Make geometry for any new chunks
        meshes->upload_new_geometry();


        // upload shadow UBO things

        render_shadow_pass();

        // main_framebuffer->bind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        update_gbuffer_ubos();

        render_gbuffers();

        render_composite_passes();

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_final_pass();

        // We want to draw the GUI on top of the other things, so we'll render it last
        // Additionally, I could use the stencil buffer to not draw MC underneath the GUI. Could be a fun
        // optimization - I'd have to watch out for when the user hides the GUI, though. I can just re-render the
        // stencil buffer when the GUI screen changes
        render_gui();

        game_window->end_frame();
    }

    void nova_renderer::render_shadow_pass() {
        LOG(TRACE) << "Rendering shadow pass";
    }

    void nova_renderer::render_gbuffers() {
        LOG(TRACE) << "Rendering gbuffer pass";

        // TODO: Get shaders with gbuffers prefix, draw transparents last, etc
        auto& terrain_shader = loaded_shaderpack->get_shader("gbuffers_terrain");
        render_shader(terrain_shader);
        auto& water_shader = loaded_shaderpack->get_shader("gbuffers_water");
        render_shader(water_shader);
    }

    void nova_renderer::render_composite_passes() {
        LOG(TRACE) << "Rendering composite passes";
    }

    void nova_renderer::render_final_pass() {
        LOG(TRACE) << "Rendering final pass";
        //meshes->get_fullscreen_quad->set_active();
        //meshes->get_fullscreen_quad->draw();
    }

    void nova_renderer::render_gui() {
        LOG(TRACE) << "Rendering GUI";
        //glClear(GL_DEPTH_BUFFER_BIT);

        // Bind all the GUI data
        auto &gui_shader = loaded_shaderpack->get_shader("gui");
        gui_shader.bind();

        upload_gui_model_matrix(gui_shader);

        // Render GUI objects
        std::vector<render_object>& gui_geometry = meshes->get_meshes_for_shader("gui");
        for(const auto& geom : gui_geometry) {
            if (!geom.color_texture.empty()) {
                auto color_texture = textures->get_texture(geom.color_texture);
                color_texture.bind(0);
            }
            geom.geometry->set_active();
            geom.geometry->draw();
        }
    }

    bool nova_renderer::should_end() {
        // If the window wants to close, the user probably clicked on the "X" button
        return game_window->should_close();
    }

	std::unique_ptr<settings> nova_renderer::render_settings;

    void nova_renderer::init() {
		render_settings = std::make_unique<settings>("config/config.json");

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
            LOG(DEBUG) << "There's currenty no shaderpack, so we're loading a new one";
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
		LOG(INFO) << "Loading a new shaderpack";
        LOG(INFO) << "Name of shaderpack " << new_shaderpack_name;
        loaded_shaderpack = std::make_shared<shaderpack>(load_shaderpack(new_shaderpack_name));
        LOG(DEBUG) << "Shaderpack loaded, wiring everything together";
        LOG(INFO) << "Loading complete";
		
        link_up_uniform_buffers(loaded_shaderpack->get_loaded_shaders(), *ubo_manager);
        LOG(DEBUG) << "Linked up UBOs";

        create_framebuffers_from_shaderpack();
    }

    void nova_renderer::create_framebuffers_from_shaderpack() {
        // TODO: Examine the shaderpack and determine what's needed
        render_passes = std::make_unique<renderpass_manager>(loaded_shaderpack);
    }

    void nova_renderer::deinit() {
        instance.release();
    }

    void nova_renderer::render_shader(gl_shader_program &shader) {
        LOG(TRACE) << "Rendering everything for shader " << shader.get_name();
        profiler::start(shader.get_name());
        shader.bind();

        profiler::start("get_meshes_for_shader");
        auto& geometry = meshes->get_meshes_for_shader(shader.get_name());
        profiler::end("get_meshes_for_shader");
        profiler::start("process_all");
        for(auto& geom : geometry) {
            profiler::start("process_renderable");

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

                profiler::start("drawcall");
                geom.geometry->set_active();
                geom.geometry->draw();
                profiler::end("drawcall");
            } else {
                LOG(TRACE) << "Skipping some geometry since it has no data";
            }
            profiler::end("process_renderable");
        }
        profiler::end("process_all");

        profiler::end(shader.get_name());
    }

    inline void nova_renderer::upload_model_matrix(render_object &geom, gl_shader_program &program) const {
        glm::mat4 model_matrix = glm::translate(glm::mat4(1), geom.position);

        auto model_matrix_location = program.get_uniform_location("gbufferModel");
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

        auto model_matrix_location = program.get_uniform_location("gbufferModel");

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

    void link_up_uniform_buffers(std::unordered_map<std::string, gl_shader_program> &shaders, uniform_buffer_store &ubos) {
        nova::foreach(shaders, [&](auto shader) { ubos.register_all_buffers_with_shader(shader.second); });
    }
}

