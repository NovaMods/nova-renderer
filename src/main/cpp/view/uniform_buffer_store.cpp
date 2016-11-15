/*!
 * \author David
 * \date 05-Jul-16.
 */

#include <fstream>
#define ELPP_THREAD_SAFE
#include <easylogging++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "uniform_buffer_store.h"

namespace nova {
    namespace view {
        uniform_buffer_store::uniform_buffer_store() {
            create_ubos();

            LOG(INFO) << "Initialized uniform buffer store";
        }

        void uniform_buffer_store::create_ubos() {
            buffers.emplace("per_frame_uniforms", model::gl_uniform_buffer(sizeof(model::per_frame_uniforms)));
            buffers.emplace("gui_uniforms", model::gl_uniform_buffer(sizeof(model::gui_uniforms)));
        }

        void uniform_buffer_store::set_bind_points(nlohmann::json &config) {
            nlohmann::json &ubo_bind_points_node = config["uboBindPoints"];

            LOG(INFO) << "Provided configuration: " << config;

            for(auto &pair : buffers) {
                unsigned int bind_point = ubo_bind_points_node[pair.first];
                buffers[pair.first].set_bind_point(bind_point);
                buffers[pair.first].set_name(pair.first);
            }
        }

        void uniform_buffer_store::update() {
            update_gui_uniforms();

            update_per_frame_uniforms();
        }

        void uniform_buffer_store::on_config_change(nlohmann::json &new_config) {
            config = new_config;    // Yes, this is a slow copy
            // TODO: Optimize this

            LOG(DEBUG) << "UBO store received updated config";
        }

        void uniform_buffer_store::on_config_loaded(nlohmann::json &config) {
            // Set up initial configuration, which right now means setting the bind points for all the UBOs
            set_bind_points(config);
        }

        model::gl_uniform_buffer &uniform_buffer_store::operator[](std::string name) {
            return buffers[name];
        }

        void uniform_buffer_store::register_all_buffers_with_shader(model::gl_shader_program &shader) const noexcept {
            for(auto &buffer : buffers) {
                shader.link_to_uniform_buffer(buffer.second);
            }
        }

        void uniform_buffer_store::update_gui_uniforms() {
            float view_width = config["viewWidth"];
            float view_height = config["viewHeight"];

            // The GUI matrix is super simple, just a viewport transformation
            glm::mat4 gui_model_view(1.0f);
            gui_model_view = glm::scale(gui_model_view, glm::vec3(1.0 / view_width, 1.0 / view_height, 1.0));
            gui_model_view = glm::scale(gui_model_view, glm::vec3(4.0f, 4.0f, 1.0f));
            gui_model_view = glm::translate(gui_model_view, glm::vec3(-1.0f, -1.0f, 0.0f));
            gui_model_view = glm::scale(gui_model_view, glm::vec3(1.0f, -1.0f, 1.0f));

            gui_uniforms.gbufferModelView = gui_model_view;

            buffers["gui_uniforms"].send_data(gui_uniforms);
            LOG(DEBUG) << "Updated all uniforms";
        }

        void uniform_buffer_store::update_per_frame_uniforms() {
            buffers["per_frame_uniforms"].send_data(per_frame_uniforms);
        }
    }
}

