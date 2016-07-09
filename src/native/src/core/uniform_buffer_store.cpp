/*!
 * \author David
 * \date 05-Jul-16.
 */

#include <core/shaders/uniform_buffer_definitions.h>
#include <fstream>
#include <easylogging++.h>
#include "uniform_buffer_store.h"

uniform_buffer_store::uniform_buffer_store() {
    create_ubos();

    set_bind_points();
}

void uniform_buffer_store::create_ubos() {
    buffers.emplace("cameraData", gl_uniform_buffer(sizeof(camera_data)));
}

void uniform_buffer_store::set_bind_points() {
    nlohmann::json data = load_data();

    for(auto & pair : buffers) {
        // Set the bind point from the config file
        unsigned int bind_point = data["data"]["uboBindPoints"][pair.first];
        buffers[pair.first].set_bind_point(bind_point);
        buffers[pair.first].set_name(pair.first);
    }
}

nlohmann::json uniform_buffer_store::load_data() const {
    nlohmann::json data;

    std::ifstream config_file("config/data.json");
    if(config_file.is_open()) {
        std::string buf;
        std::string accum;

        while(getline(config_file, buf)) {
            accum += buf;
        }

        data = nlohmann::json::parse(accum.c_str());
    }

    return data;
}

void uniform_buffer_store::on_config_change(nlohmann::json &new_config) {
    // Grab the data we need from the config structure
    cam_data.viewHeight = new_config["viewWidth"];
    cam_data.viewWidth = new_config["viewHeight"];

    upload_data();
}

void uniform_buffer_store::upload_data() {
    buffers["cameraData"].send_data(cam_data);
}

gl_uniform_buffer & uniform_buffer_store::operator[](std::string name) {
    return buffers[name];
}

void uniform_buffer_store::register_all_buffers_with_shader(gl_shader_program &shader) const noexcept {
    for(auto & buffer : buffers) {
        shader.link_to_uniform_buffer(buffer.second);
    }
}



