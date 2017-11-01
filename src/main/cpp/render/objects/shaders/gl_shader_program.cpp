/*!
 * \author David
 * \date 17-May-16.
 */

#include <cstdlib>
#include <algorithm>

#include <easylogging++.h>
#include "gl_shader_program.h"
#include "../../vulkan/render_context.h"

namespace nova {
    gl_shader_program::gl_shader_program(const shader_definition &source) : name(source.name) {
        device = render_context::instance.device;
        LOG(TRACE) << "Creating shader with filter expression " << source.filter_expression;
        filter = source.filter_expression;
        LOG(TRACE) << "Created filter expression " << filter;
        create_shader(source.vertex_source, vk::ShaderStageFlagBits::eVertex);
        LOG(TRACE) << "Creatd vertex shader";
        create_shader(source.fragment_source, vk::ShaderStageFlagBits::eFragment);
        LOG(TRACE) << "Created fragment shader";

        link();
    }

    gl_shader_program::gl_shader_program(gl_shader_program &&other) noexcept :
            name(std::move(other.name)), filter(std::move(other.filter)), shader_modules(std::move(other.shader_modules)) {

        this->gl_name = other.gl_name;

        // Make the other shader not a thing
        other.gl_name = 0;
        other.added_shaders.clear();
    }

    void gl_shader_program::link() {


        //gl_name = glCreateProgram();
        //glObjectLabel(GL_PROGRAM, gl_name, (GLsizei) name.length(), name.c_str());
        LOG(TRACE) << "Created shader program " << gl_name;

        for(GLuint shader : added_shaders) {
            //glAttachShader(gl_name, shader);
        }

        //glLinkProgram(gl_name);
        check_for_linking_errors();

        LOG(DEBUG) << "Program " << name << " linked successfully";

        for(GLuint shader : added_shaders) {
            // Clean up our resources. I'm told that this is a good thing.
            //glDetachShader(gl_name, shader);
            //glDeleteShader(shader);
        }

        LOG(DEBUG) << "Cleaned up resources";
    }

    void gl_shader_program::check_for_shader_errors(GLuint shader_to_check, const std::vector<shader_line>& line_map) {
        GLint success = 0;

        //glGetShaderiv(shader_to_check, GL_COMPILE_STATUS, &success);

        if(success == GL_FALSE) {
            GLint log_size = 0;
           // glGetShaderiv(shader_to_check, GL_INFO_LOG_LENGTH, &log_size);

            std::vector<GLchar> error_log((unsigned long long int) log_size);
            //glGetShaderInfoLog(shader_to_check, log_size, &log_size, &error_log[0]);

            if(log_size > 0) {
                //glDeleteShader(shader_to_check);
                LOG(ERROR) << error_log.data();
                throw compilation_error(error_log.data(), line_map);
            }
        }
    }

    void gl_shader_program::check_for_linking_errors() {
        GLint is_linked = 0;
        //glGetProgramiv(gl_name, GL_LINK_STATUS, &is_linked);

        if(is_linked == GL_FALSE) {
            GLint log_length = 0;
            //glGetProgramiv(gl_name, GL_INFO_LOG_LENGTH, &log_length);

            GLchar *info_log = (GLchar *) malloc(log_length * sizeof(GLchar));
            //glGetProgramInfoLog(gl_name, log_length, &log_length, info_log);

            if(log_length > 0) {
                //glDeleteProgram(gl_name);

                LOG(ERROR) << "Error linking program " << gl_name << ":\n" << info_log;

                throw program_linking_failure(name);
            }

        }
    }

    void gl_shader_program::bind() noexcept {
        //glUseProgram(gl_name);
    }

    gl_shader_program::~gl_shader_program() {
        for(auto& flags_module_pair : shader_modules) {
            device.destroyShaderModule(flags_module_pair.second);
        }
    }

    void gl_shader_program::create_shader(const std::vector<uint32_t>& shader_source, const vk::ShaderStageFlags flags) {
        vk::ShaderModuleCreateInfo create_info = {};
        create_info.codeSize = shader_source.size();
        create_info.pCode = shader_source.data();

        auto module = device.createShaderModule(create_info);
        shader_modules[flags] = module;
    }

    std::string & gl_shader_program::get_filter() noexcept {
        return filter;
    }

    std::string &gl_shader_program::get_name() noexcept {
        return name;
    }

    GLint gl_shader_program::get_uniform_location(const std::string uniform_name) {
        auto location_in_uniform_locations = uniform_locations.find(uniform_name);
        if(location_in_uniform_locations == uniform_locations.end()) {
            //uniform_locations[uniform_name] = glGetUniformLocation(gl_name, uniform_name.c_str());
        }

        return uniform_locations[uniform_name];
    }

    wrong_shader_version::wrong_shader_version(const std::string &version_line) :
            std::runtime_error(
                    "Invalid version line: '" + version_line + "'. Please only use GLSL version 450 (NOT compatibility profile)"
            ) {}

    compilation_error::compilation_error(const std::string &error_message,
                                         const std::vector<shader_line> source_lines) :
            std::runtime_error(error_message + get_original_line_message(error_message, source_lines)) {}

    std::string compilation_error::get_original_line_message(const std::string &error_message,
                                                             const std::vector<shader_line> source_lines) {
        return "This logic isn't implemented yet";
    }

}
