/*!
 * \brief Defines a shader program ("program" im OpenGL parlace) and a number of exceptions it can throw
 *
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
    /*!
     * \brief Constructs this exceptions
     *
     * \param file_name The name of the file that could not be found
     */
    shader_file_not_found_exception( std::string &file_name );
    virtual const char * what() noexcept;
private:
    std::string msg;
};

/*!
 * \brief Represents an OpenGL shader program
 *
 * Shader programs can include between two and five shaders. At the bare minimum, a shader program needs a vertex shader
 * and a fragment shader. A shader program can also have a geometry shader, a tessellation control shader, and a
 * tessellation evaluation shader. Note that if a shader program has one of the tessellation shaders, it must also have
 * the other tessellation shader.
 */
class gl_shader_program : public ishader {
public:
    /*!
     * \brief Constructs a gl_shader_program
     */
    gl_shader_program();

    /*!
     * \brief Adds a shader to this shader program
     *
     * \param shader_type The type of the shader to add. Can be one of GL_VERTEX_SHADER, GL_GEOMETRY_SHADER,
     * GL_TESSELLATION_CONTROL_SHADER, GL_TESSELLATION_EVALUATION_SHADER, or GL_FRAGMENT_SHADER. Note that if you add a
     * shader and the shader_program you add it to already has a shader of that type, you'll get an exception
     * \param source_file_name The name of the file to read the shader source from. Right now this file path is
     * relative to the working directory. Upon release this file path will be relative to the root of the current
     * shaderpack. Shaderpacks aren't implemented yet, so I can't really make this work the proper way yet
     */
    void add_shader(GLenum shader_type, std::string source_file_name);

    /*!
     * \brief Links this shader program
     *
     * If this shader program fails to link, an exception is thrown
     */
    void link();

    void bind() noexcept;
    int get_uniform_location(std::string & uniform_name) const;
    int get_attribute_location(std::string & attribute_name) const;
    void set_uniform_data(unsigned int location, int data) noexcept;

    /*
     * Testing functions
     *
     * TODO: Remove these before release
     *
     * I'm sure there's a better way to accomplish this. All well.
     */
    std::vector<GLuint> & get_added_shaders();

    std::vector<std::string> & get_uniform_names();

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
