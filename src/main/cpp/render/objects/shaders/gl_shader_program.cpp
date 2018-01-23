/*!
 * \author David
 * \date 17-May-16.
 */

#include <cstdlib>
#include <algorithm>

#include <easylogging++.h>
#include "gl_shader_program.h"

namespace nova {
    gl_shader_program::gl_shader_program(const shader_definition &source) : name(source.name) {
        LOG(TRACE) << "Creating shader with filter expression " << source.filter_expression;
        filter = source.filter_expression;
        LOG(TRACE) << "Created filter expression " << filter;
        create_shader(source.vertex_source, GL_VERTEX_SHADER);
        LOG(TRACE) << "Creatd vertex shader";
        create_shader(source.fragment_source, GL_FRAGMENT_SHADER);
        LOG(TRACE) << "Created fragment shader";

        link();
    }

    gl_shader_program::gl_shader_program(gl_shader_program &&other) noexcept :
            name(std::move(other.name)), filter(std::move(other.filter)) {

        this->gl_name = other.gl_name;

        // Make the other shader not a thing
        other.gl_name = 0;
        other.added_shaders.clear();
    }

    void gl_shader_program::link() {
        gl_name = glCreateProgram();
        glObjectLabel(GL_PROGRAM, gl_name, (GLsizei) name.length(), name.c_str());
        LOG(TRACE) << "Created shader program " << gl_name;

        for(GLuint shader : added_shaders) {
            glAttachShader(gl_name, shader);
        }

        glLinkProgram(gl_name);
        check_for_linking_errors();

        LOG(DEBUG) << "Program " << name << " linked successfully";

        for(GLuint shader : added_shaders) {
            // Clean up our resources. I'm told that this is a good thing.
            glDetachShader(gl_name, shader);
            glDeleteShader(shader);
        }

        LOG(DEBUG) << "Cleaned up resources";
    }

    void gl_shader_program::check_for_shader_errors(GLuint shader_to_check, const std::vector<shader_line>& line_map) {
        GLint success = 0;

        glGetShaderiv(shader_to_check, GL_COMPILE_STATUS, &success);

        if(success == GL_FALSE) {
            GLint log_size = 0;
            glGetShaderiv(shader_to_check, GL_INFO_LOG_LENGTH, &log_size);

            std::vector<GLchar> error_log((unsigned long long int) log_size);
            glGetShaderInfoLog(shader_to_check, log_size, &log_size, &error_log[0]);

            if(log_size > 0) {
                glDeleteShader(shader_to_check);
                LOG(ERROR) << error_log.data();
                throw compilation_error(error_log.data(), line_map);
            }
        }
    }

    void gl_shader_program::check_for_linking_errors() {
        GLint is_linked = 0;
        glGetProgramiv(gl_name, GL_LINK_STATUS, &is_linked);

        if(is_linked == GL_FALSE) {
            GLint log_length = 0;
            glGetProgramiv(gl_name, GL_INFO_LOG_LENGTH, &log_length);

            GLchar *info_log = (GLchar *) malloc(log_length * sizeof(GLchar));
            glGetProgramInfoLog(gl_name, log_length, &log_length, info_log);

            if(log_length > 0) {
                glDeleteProgram(gl_name);

                LOG(ERROR) << "Error linking program " << gl_name << ":\n" << info_log;

                throw program_linking_failure(name);
            }

        }
    }

    void gl_shader_program::bind() noexcept {
        //LOG(INFO) << "Binding program " << name;
        glUseProgram(gl_name);
    }

    gl_shader_program::~gl_shader_program() {
        // Commented because move semantics are hard
        //LOG(DEBUG) << "Deleting program " << gl_name;
        //glDeleteProgram(gl_name);
    }

    void gl_shader_program::create_shader(const std::vector<shader_line>& shader_source, const GLenum shader_type) {
        LOG(TRACE) << "Creating a shader from source\n" << shader_source;

        std::string full_shader_source;
        auto& version_line = shader_source[0].line;
        LOG(TRACE) << "Version line: '" << version_line << "'";

        if(version_line == "#version 450") {
            // GLSL 450 code! This is the simplest: just concatenate all the lines in the shader file
            std::for_each(
                    std::begin(shader_source), std::end(shader_source),
                    [&](auto &line) { full_shader_source.append(line.line + "\n"); }
            );

        } else {
            throw wrong_shader_version(shader_source[0].line);
        }

        auto shader_name = glCreateShader(shader_type);

        const char *shader_source_char = full_shader_source.c_str();

        glShaderSource(shader_name, 1, &shader_source_char, nullptr);

        glCompileShader(shader_name);

        check_for_shader_errors(shader_name, shader_source);

        added_shaders.push_back(shader_name);
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
            uniform_locations[uniform_name] = glGetUniformLocation(gl_name, uniform_name.c_str());
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
