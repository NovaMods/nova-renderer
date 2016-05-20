/*!
 * \author David
 * \date 17-May-16.
 */

#include "shader_store.h"
#include "../../gl/objects/gl_shader_program.h"

shader_store::shader_store() {
    // This function should load the default shader pack
    // For now, it just loads the GUI shader
    gl_shader_program gui_shader;
    gui_shader.add_shader(GL_VERTEX_SHADER, "shaderpacks/default/shaders/gui.vert");
    gui_shader.add_shader(GL_FRAGMENT_SHADER, "shaderpacks/default/shaders/gui.frag");
    gui_shader.link_program();

    shaders.emplace("gui", gui_shader);
}

ishader *shader_store::get_shader(const std::string &shader_name) const {
    return shaders[shader_name];
}

