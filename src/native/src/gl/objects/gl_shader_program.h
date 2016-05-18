/*!
 * \author David
 * \date 17-May-16.
 */

#ifndef RENDERER_GL_SHADER_H
#define RENDERER_GL_SHADER_H

#include <istream>
#include <unordered_map>
#include <vector>

#include "../../interfaces/ishader.h"
#include "../glad/glad.h"

class shader_program_already_linked_exception : public std::exception {
public:
    virtual const char * what() noexcept;
};

class program_linking_failure_exception : public std::exception {
public:
    virtual const char * what() noexcept;
};

class shader_file_not_found_exception : public std::exception {
public:
    shader_file_not_found_exception( std::string &file_name );
    virtual const char * what() noexcept;
private:
    std::string m_msg;
};

class gl_shader_program : public ishader {
public:
    gl_shader_program();

    void add_shader(GLenum shader_type, std::string source_file_name);

    void link_program();

    virtual void bind() noexcept;
    virtual int get_uniform_location(std::string & uniform_name) const;
    virtual int get_attribute_location(std::string & attribute_name) const;
    virtual void set_uniform_data(GLuint location, int data) noexcept;

private:
    std::unordered_map<std::string, GLuint> uniform_locations;
    std::unordered_map<std::string, GLuint> attribute_locations;
    bool linked;

    std::string vert_shader_name;
    std::string frag_shader_name;
    std::string geom_shader_name;
    std::string tese_shader_name;
    std::string tesc_shader_name;

    GLuint gl_name;

    std::vector<std::string> uniform_names;
    std::vector<std::string> attribute_names;
    std::vector<GLuint> added_shaders;

    std::string read_shader_file(std::string &filename);

    bool check_for_shader_errors(GLuint shader_to_check);

    void set_uniform_locations();

    bool check_for_linking_errors();
};


#endif //RENDERER_GL_SHADER_H
