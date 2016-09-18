/*!
 * \author David
 * \date 05-Jul-16.
 */

#include <core/shaders/uniform_buffer_definitions.h>
#include <fstream>
#include <easylogging++.h>
#include "uniform_buffer_store.h"

namespace nova {
    namespace model {
        uniform_buffer_store::uniform_buffer_store() {
            create_ubos();

            LOG(INFO) << "Initialized uniform buffer store";
        }

        void uniform_buffer_store::create_ubos() {
            buffers.emplace("cameraData", gl_uniform_buffer(16));
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

        void uniform_buffer_store::on_config_change(nlohmann::json &new_config) {
            cam_data.viewWidth = new_config["viewWidth"];
            cam_data.viewHeight = new_config["viewHeight"];

            upload_data();
            LOG(DEBUG) << "UBO store received updated config";
        }

        void uniform_buffer_store::on_config_loaded(nlohmann::json &config) {
            // Set up initial configuration, which right now means setting the bind points for all the UBOs
            set_bind_points(config);
        }

        void uniform_buffer_store::upload_data() {
            buffers["cameraData"].send_data(cam_data);
        }

        gl_uniform_buffer &uniform_buffer_store::operator[](std::string name) {
            return buffers[name];
        }

        void uniform_buffer_store::register_all_buffers_with_shader(gl_shader_program &shader) const noexcept {
            for(auto &buffer : buffers) {
                shader.link_to_uniform_buffer(buffer.second);
            }
        }
    }
}

