/*!
 * \author David
 * \date 05-Jul-16.
 */

#include <fstream>
#define ELPP_THREAD_SAFE
#include <easylogging++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nova_renderer.h"
#include "uniform_buffer_store.h"

namespace nova {
    uniform_buffer_store::uniform_buffer_store() : gui_uniform_buffer("gui_uniforms"), per_frame_uniforms_buffer("per_frame_uniforms") {
		
		LOG(INFO) << "Initialized uniform buffer store";
    }

    void uniform_buffer_store::update() {
        update_gui_uniforms(nova_renderer::get_render_settings().get_options());

        update_per_frame_uniforms();
    }

    void uniform_buffer_store::on_config_change(nlohmann::json &new_config) {
       

        LOG(DEBUG) << "UBO store received updated config";

        // The GUI shader uniforms only update when the screen size changes, so we can update them here and not worry
        update_gui_uniforms(new_config);

        // We'll probably also want to update the per frame uniforms so that we have the correct aspect ratio and
        // whatnot
        update_per_frame_uniforms();
    }

    void uniform_buffer_store::on_config_loaded(nlohmann::json &config) {}

    void uniform_buffer_store::register_all_buffers_with_shader(const gl_shader_program &shader) noexcept {
        gui_uniform_buffer.link_to_shader(shader);
        per_frame_uniforms_buffer.link_to_shader(shader);
    }

    void uniform_buffer_store::update_gui_uniforms(nlohmann::json &config) {
		


		float view_width = config["viewWidth"];
		float view_height = config["viewHeight"];
        // The GUI matrix is super simple, just a viewport transformation
        glm::mat4 gui_model_view(1.0f);
        gui_model_view = glm::scale(gui_model_view, glm::vec3(1.0 / view_width, 1.0 / view_height, 1.0));
        gui_model_view = glm::scale(gui_model_view, glm::vec3(4.0f, 4.0f, 1.0f));
        gui_model_view = glm::translate(gui_model_view, glm::vec3(-1.0f, -1.0f, 0.0f));
        gui_model_view = glm::scale(gui_model_view, glm::vec3(1.0f, -1.0f, 1.0f));

        gui_uniform_variables.gbufferModelView = gui_model_view;
        gui_uniform_variables.gbufferProjection = glm::mat4(1);
        gui_uniform_variables.aspectRatio = view_width / view_height;
        gui_uniform_variables.viewHeight = view_height;
        gui_uniform_variables.viewWidth = view_width;

        LOG(TRACE) << "gui_uniform_variables buffer: " << gui_uniform_variables;

        gui_uniform_buffer.send_data(gui_uniform_variables);
        LOG(DEBUG) << "Updated GUI UBO";
    }

    void uniform_buffer_store::update_per_frame_uniforms() {
        // TODO: Fill in the per frame uniforms
        per_frame_uniforms_buffer.send_data(per_frame_uniform_variables);
        LOG(DEBUG) << "Updated per frame UBO";
    }
}

