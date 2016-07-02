/*!
 * \author David
 * \date 20-Jun-16.
 */

#include "shaderpack.h"
#include "../gl/objects/gl_shader_program.h"

#include <fstream>
#include <easylogging++.h>

shaderpack::shaderpack() {
    default_shader_names.push_back("gui");
}

void shaderpack::load_shaderpack(const std::string &shaderpack_name) {
    name = shaderpack_name;

    // check if the shaderpack is a zip file or not
    if(shaderpack_name.find(".zip") != std::string::npos) {
        load_zip_shaderpack(shaderpack_name);
    } else {
        load_folder_shaderpack(shaderpack_name);
    }
}

void shaderpack::load_zip_shaderpack(std::string shaderpack_name) {
    // TODO: Raven u wer suppozed to writ this
}

void shaderpack::load_folder_shaderpack(std::string shaderpack_name) {
    // I should look at a config file and use that to figure out what shaders they're using
    // However, I don't want to code that just yet, so I'm only going to load the default shaders

    // Shaders are at "$shaderpack_name/shaders", so let's go there

    const std::string shaders_base_dir = "shaderpacks/" + shaderpack_name + "/" + SHADERPACK_FOLDER_NAME + "/";

    LOG(INFO) << "Loading shaders from folder " << shaders_base_dir;

    for(const std::string & shader_name : default_shader_names) {
        load_program(shaders_base_dir, shader_name);
    }
}

void shaderpack::load_program(const std::string shader_path, const std::string shader_name) {

    gl_shader_program program(shader_name);

    const std::string full_shader_path = shader_path + shader_name;

    load_shader(full_shader_path, program, GL_VERTEX_SHADER);
    load_shader(full_shader_path, program, GL_FRAGMENT_SHADER);

    program.link();

    shaders.emplace(shader_name, program);

    // Only load vertex and fragment shaders for now

    // TODO: Support geometry and tessellation shaders
}

void shaderpack::load_shader(const std::string &shader_name,
                             gl_shader_program & program, GLenum shader_type) const {
    // I don't like this because of the duplicate code, Not sure what else to do, though
    switch(shader_type) {
        case GL_VERTEX_SHADER:
            if(!try_loading_shader(shader_name, program, shader_type, ".vsh")) {
                if(!try_loading_shader(shader_name, program, shader_type, ".vert")) {
                    throw shader_file_not_found_exception(shader_name + " vertex file");
                }
            }
            break;
        case GL_FRAGMENT_SHADER:
            if(!try_loading_shader(shader_name, program, shader_type, ".fsh")) {
                if(!try_loading_shader(shader_name, program, shader_type, ".frag")) {
                    throw shader_file_not_found_exception(shader_name + " fragment file");
                }
            }
            break;
        default:
            LOG(ERROR) << "Unsupported shader type";
    }
}

bool shaderpack::try_loading_shader(const std::string &shader_name, gl_shader_program & program, GLenum shader_type,
                                    const std::string extension) const {
    const std::string full_file_name = shader_name + extension;
    std::ifstream shader_file(full_file_name);

    LOG(INFO) << "Trying to load shader " << full_file_name;

    if(shader_file.is_open()) {
        program.add_shader(shader_type, shader_file);

        shader_file.close();

        LOG(INFO) << "Success!";
        return true;
    }

    return false;
}

void shaderpack::on_config_change(nlohmann::json &new_config) {
    std::string new_shaderpack_name = new_config["loadedShaderpack"];
    if(new_shaderpack_name != name) {
        LOG(INFO) << "Switched to shaderpack " << new_shaderpack_name;
        load_shaderpack(new_shaderpack_name);
    }
}

gl_shader_program & shaderpack::get_shader(std::string shader_name) {
    return shaders[shader_name];
}









