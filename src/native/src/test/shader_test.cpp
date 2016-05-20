/*!
 * \brief Contains tests for shader loading and such
 *
 * \author David
 * \date 17-May-16.
 */

#include "shader_test.h"
#include "test_utils.h"
#include "../gl/objects/gl_shader_program.h"
#include "../core/nova_renderer.h"
#include <easylogging++.h>
#include <assert.h>

/*!
 * \brief Tests that the gl_shader_program constructor does not explode
 */
static void test_create_shader() {
    // Create a basic shader
    gl_shader_program test_shader;
}

/*!
 * \brief Tests that a fragment shader can be added
 */
static void test_add_fragment_shader() {
    // Create the shader program
    gl_shader_program test_shader;

    // Add a basic fragment program
    test_shader.add_shader(GL_FRAGMENT_SHADER, "uniform_test.frag");

    // Check that we actually added the shader
    std::vector<GLuint> & shaders = test_shader.get_added_shaders();
    assert(shaders.size() == 1);
    LOG(INFO) << "We have the right number of shaders";
}

/*!
 * \brief Tests that a vertex shader can be added
 */
static void test_add_vertex_shader() {
    // Create the shader program
    gl_shader_program test_shader;

    // Add a basic fragment program
    test_shader.add_shader(GL_VERTEX_SHADER, "uniform_test.vert");

    // Check that we actually added the shader
    std::vector<GLuint> & shaders = test_shader.get_added_shaders();
    assert(shaders.size() == 1);
    LOG(INFO) << "We have the right number of shaders";
}

static void test_link_shader() {
    gl_shader_program test_shader;

    test_shader.add_shader(GL_VERTEX_SHADER, "uniform_test.vert");
    test_shader.add_shader(GL_FRAGMENT_SHADER, "uniform_test.frag");

    test_shader.link_program();
}

static void test_parse_uniforms() {
    // Create the shader program
    gl_shader_program test_shader;

    // Add a basic fragment program
    test_shader.add_shader(GL_FRAGMENT_SHADER, "uniform_test.frag");

    // Get all the uniforms and ensure that we have all the ones we need
    std::vector<std::string> & uniforms = test_shader.get_uniform_names();

    assert(uniforms.size() == 4);
    LOG(INFO) << "We have the right number of uniforms";

    assert(std::find(uniforms.begin(), uniforms.end(), "gbufferModelviewInverse") != uniforms.end());
    assert(std::find(uniforms.begin(), uniforms.end(), "gbufferProjectionInverse") != uniforms.end());
    assert(std::find(uniforms.begin(), uniforms.end(), "sunPosition") != uniforms.end());
    assert(std::find(uniforms.begin(), uniforms.end(), "worldTime") != uniforms.end());

    LOG(INFO) << "We have all the uniforms we should";
}

void shader::run_all() {
    run_test(test_create_shader, "test_create_shader");
    run_test(test_add_fragment_shader, "test_add_fragment_shader");
    run_test(test_add_vertex_shader, "test_add_vertex_shader");
    run_test(test_link_shader, "test_link_shader");
    run_test(test_parse_uniforms, "test_parse_uniforms");
}

