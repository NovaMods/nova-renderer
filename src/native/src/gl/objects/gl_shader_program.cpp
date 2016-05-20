/*!
 * \author David
 * \date 17-May-16.
 */

#include <easylogging++.h>
#include "gl_shader_program.h"

gl_shader_program::gl_shader_program() : linked(false) {

}

void gl_shader_program::add_shader(GLenum shader_type, std::string source_file_name) {
    if(linked) {
        throw shader_program_already_linked_exception();
    }

    switch(shader_type) {
        case GL_VERTEX_SHADER:
            vert_shader_name = source_file_name;
            break;
        case GL_FRAGMENT_SHADER:
            frag_shader_name = source_file_name;
            break;
        case GL_GEOMETRY_SHADER:
            geom_shader_name = source_file_name;
            break;
        case GL_TESS_CONTROL_SHADER:
            tesc_shader_name = source_file_name;
            break;
        case GL_TESS_EVALUATION_SHADER:
            tese_shader_name = source_file_name;
            break;
        default:
            LOG(ERROR) << "Invalid shader type " << shader_type << " specified";
    }

    GLuint shader_name = glCreateShader(shader_type);

    // Read in the shader source, getting uniforms
    std::string shader_source = read_shader_file(source_file_name);

    const char *shader_source_char = shader_source.c_str();

    glShaderSource(shader_name, 1, &shader_source_char, NULL);

    glCompileShader(shader_name);

    if(!check_for_shader_errors(shader_name)) {
        added_shaders.push_back(shader_name);
    }
}

void gl_shader_program::link_program() {
    linked = true;

    gl_name = glCreateProgram();

    for(GLuint shader : added_shaders) {
        glAttachShader(gl_name, shader);
    }

    glLinkProgram(gl_name);
    if(check_for_linking_errors()) {
        glDeleteProgram(gl_name);

        throw program_linking_failure_exception();
    }

    for(GLuint shader : added_shaders) {
        // Clean up our resources. I'm told that this is a good thing.
        glDetachShader(gl_name, shader);
        glDeleteShader(shader);
    }

    // No errors during linking? Let's get locations for our variables
    set_uniform_locations();

    LOG(INFO) << "Program " << gl_name << " linked successfully";
}

std::string gl_shader_program::read_shader_file(std::string& filename) {
    // TODO: Add support for some kind of #include statement
    std::ifstream file(filename.c_str());

    if(file.is_open()) {
        std::string buf;
        std::string accum;
        while(getline(file, buf)) {
            accum += buf + "\n";

            std::string var_type;
            std::string var_name;
            std::size_t start_pos = buf.find("uniform");
            std::size_t end_pos = -1;

            // Parse the shader source, looking for the `uniform` keyword
            if(start_pos != std::string::npos) {
                // Add the length of 'uniform' and one space so we can get the type
                start_pos += 8;
                end_pos = buf.find(' ', start_pos);

                var_type = buf.substr(start_pos, end_pos - start_pos);

                start_pos = end_pos + 1;
                end_pos = buf.find(';', start_pos);

                var_name = buf.substr(start_pos, end_pos - start_pos);

                LOG(TRACE) << "Found uniform '" << var_type << "' '" << var_name << "'\n";

                uniform_names.push_back(var_name);
            }
        }
        return accum;
    } else {
        throw shader_file_not_found_exception(filename);
    }
}

bool gl_shader_program::check_for_shader_errors(GLuint shader_to_check) {
    GLint success = 0;

    glGetShaderiv(shader_to_check, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE) {
        GLint log_size = 0;
        glGetShaderiv(shader_to_check, GL_INFO_LOG_LENGTH, &log_size);

        std::vector<GLchar> error_log(log_size);
        glGetShaderInfoLog(shader_to_check, log_size, &log_size, &error_log[0]);

        LOG(ERROR) << "Error compiling shader: \n" << &error_log[0];

        glDeleteShader(shader_to_check);

        return true;
    }

    return false;
}

void gl_shader_program::set_uniform_locations() {
    for(const std::string &name : uniform_names) {
        int location = glGetUniformLocation(gl_name, name.c_str());
        uniform_locations.emplace(name, location);

        LOG(TRACE) << "Set location of variable " << name << "to" << location;
    }
}

bool gl_shader_program::check_for_linking_errors() {
    GLint is_linked = 0;
    glGetProgramiv(gl_name, GL_LINK_STATUS, &is_linked);

    if(is_linked == GL_FALSE) {
        GLint log_length = 0;
        glGetProgramiv(gl_name, GL_INFO_LOG_LENGTH, &log_length);

        GLchar *info_log = (GLchar *) malloc(log_length * sizeof(GLchar));
        glGetProgramInfoLog(gl_name, log_length, &log_length, info_log);

        LOG(ERROR) << "Error linking program " << gl_name << ":\n" << info_log;

        return true;
    }

    return false;
}

void gl_shader_program::bind() noexcept {
    LOG(INFO) << "Binding a shader";
}

int gl_shader_program::get_uniform_location(std::string &uniform_name) const {
    LOG(INFO) << "Geting uniform location for uniform " << uniform_name;
    return 0;
}

int gl_shader_program::get_attribute_location(std::string &attribute_name) const {
    LOG(INFO) << "Getting attribute location for attribute " << attribute_name;
    return 0;
}

void gl_shader_program::set_uniform_data(GLuint location, int data) noexcept {
    LOG(INFO) << "Setting uniform data for uniform at location " << location << " to " << data;
}

std::vector<GLuint> &gl_shader_program::get_added_shaders() {
    return added_shaders;
}

std::vector<std::string> &gl_shader_program::get_uniform_names() {
    return uniform_names;
}

shader_file_not_found_exception::shader_file_not_found_exception( std::string &file_name ) :
        m_msg( "Could not open shader file " + file_name ) {}

const char * shader_file_not_found_exception::what() noexcept {
    return m_msg.c_str();
}

const char * shader_program_already_linked_exception::what() noexcept {
    return "Program was already linked";
}

const char * program_linking_failure_exception::what() noexcept {
    return "Program failed to link";
}

