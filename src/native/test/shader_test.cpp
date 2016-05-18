/*!
 * \brief Contains tests for shader loading and such
 *
 * \author David
 * \date 17-May-16.
 */

#include "shader_test.h"
#include "test_utils.h"
#include "../src/gl/objects/gl_shader_program.h"
#include <easylogging++.h>

/*!
 * \brief Tests that the gl_shader_program constructor does not explode
 */
static void test_create_shader() {
    // Create a basic shader
    gl_shader_program test_shader;
}

/*!
 * \brief Tests that a fragment
 */
static void test_add_fragment_shader() {
    // Create the shader program
    gl_shader_program test_shader;

    // Add a basic fragment program
    test_shader.add_shader(GL_FRAGMENT_SHADER, "uniform_test.frag");
}

static void test_parse_uniforms() {

}

void shader::run_all() {
    run_test(test_create_shader, "test_create_shader");
}

