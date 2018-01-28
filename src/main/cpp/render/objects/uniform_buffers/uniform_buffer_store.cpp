/*!
 * \author David
 * \date 05-Jul-16.
 */

#include <fstream>
#include <easylogging++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../nova_renderer.h"
#include "uniform_buffer_store.h"

namespace nova {
    uniform_buffer_store::uniform_buffer_store() : per_frame_uniforms_buffer("per_frame_uniforms") {
		LOG(INFO) << "Initialized uniform buffer store";
    }

    void uniform_buffer_store::update() {
        update_per_frame_uniforms(nova_renderer::get_render_settings().get_options());
    }

    void uniform_buffer_store::on_config_change(nlohmann::json &new_config) {
        LOG(DEBUG) << "UBO store received updated config";

        // We'll probably also want to update the per frame uniforms so that we have the correct aspect ratio and
        // whatnot
        update_per_frame_uniforms(new_config);
    }

    void uniform_buffer_store::on_config_loaded(nlohmann::json &config) {}

    void uniform_buffer_store::register_all_buffers_with_shader(const vk_shader_program &shader) noexcept {
        per_frame_uniforms_buffer.link_to_shader(shader);
    }

    void uniform_buffer_store::update_per_frame_uniforms(nlohmann::json &config) {
		float view_width = config["viewWidth"];
		float view_height = config["viewHeight"];
        float scalefactor = config["scalefactor"];
        // The GUI matrix is super simple, just a viewport transformation
        glm::mat4 gui_model_view(1.0f);
        gui_model_view = glm::translate(gui_model_view, glm::vec3(-1.0f, 1.0f, 0.0f));
        gui_model_view = glm::scale(gui_model_view, glm::vec3(scalefactor, scalefactor, 1.0f));
        gui_model_view = glm::scale(gui_model_view, glm::vec3(1.0 / view_width, 1.0 / view_height, 1.0));
        gui_model_view = glm::scale(gui_model_view, glm::vec3(1.0f, -1.0f, 1.0f));

        per_frame_uniform_variables.gbufferModelView = glm::mat4(1);
        per_frame_uniform_variables.gbufferProjection = glm::mat4(1);
        per_frame_uniform_variables.aspectRatio = view_width / view_height;
        per_frame_uniform_variables.viewHeight = view_height;
        per_frame_uniform_variables.viewWidth = view_width;
        per_frame_uniform_variables.frameTimeCounter = 0;
        per_frame_uniform_variables.hideGUI = 0;

        per_frame_uniforms_buffer.send_data(per_frame_uniform_variables);
        LOG(DEBUG) << "Updated Per-Frame UBO";
    }

    gl_uniform_buffer<per_frame_uniforms>& uniform_buffer_store::get_per_frame_uniforms() {
        return per_frame_uniforms_buffer;
    }
}

