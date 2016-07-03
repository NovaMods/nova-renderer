//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>

#include "../gl/windowing/glfw_gl_window.h"
#include "../utils/utils.h"
#include "../gl/objects/gl_vertex_buffer.h"

INITIALIZE_EASYLOGGINGPP

std::unique_ptr<nova_renderer> nova_renderer::instance;

nova_renderer::nova_renderer() : game_window(new glfw_gl_window()),
                                 gui_renderer_instance(tex_manager, shaders, ubo_manager) {

    nova_config = std::unique_ptr<config>(new config("config/config.json"));
    nova_config->register_change_listener(game_window);
    nova_config->register_change_listener(&shaders);
    nova_config->update_change_listeners();

    gui_renderer_instance.do_init_tasks();

    create_ubos();

    enable_debug();

    glClearColor(1.0, 0.0, 1.0, 1.0);
}

nova_renderer::~nova_renderer() {
    game_window->destroy();
}

bool nova_renderer::has_render_available() {
    return false;
}

void nova_renderer::render_frame() {
    // Clear to the clear color
    glClear(GL_COLOR_BUFFER_BIT);

    // Render GUI to GUI buffer
    gui_renderer_instance.render();

    // Render solid geometry
    // Render entities
    // Render transparent things

    game_window->end_frame();
}

bool nova_renderer::should_end() {
    // If the window wants to close, the user probably clicked on the "X" button
    return game_window->should_close();
}

void nova_renderer::init_instance() {
    instance = std::unique_ptr<nova_renderer>(new nova_renderer());
}

texture_manager & nova_renderer::get_texture_manager() {
    return tex_manager;
}

shaderpack &nova_renderer::get_shaderpack() {
    return shaders;
}

uniform_buffer_store &nova_renderer::get_ubo_manager() {
    return ubo_manager;
}

gui_renderer & nova_renderer::get_gui_renderer() {
   return gui_renderer_instance;
}

void nova_renderer::create_ubos() {
    // Build all the UBOs

    ubo_manager.emplace("cameraData", new gl_uniform_buffer(sizeof(camera_data)));

    // TODO: Make a config file from UBO name to bind point
}

std::string translate_debug_source(GLenum source) {
    switch(source) {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "window system";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "shader compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "third party";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "application";
        case GL_DEBUG_SOURCE_OTHER:
            return "other";
        default:
            return "something else somehow";
    }
}

std::string translate_debug_type(GLenum type) {
    switch(type) {
        case GL_DEBUG_TYPE_ERROR:
            return "an error, probably from the API";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "some behavior marked deprecated has been used";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "something has invoked undefined behavior";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "some functionality the user relies upon is not portable";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "code has triggered possible performance issues";
        case GL_DEBUG_TYPE_MARKER:
            return "command stream annotation";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "group pushing";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "group popping";
        case GL_DEBUG_TYPE_OTHER:
            return "other";
        default:
            return "something else somwhow";
    }
}

void debug_logger(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * user_param) {
    std::string source_name = translate_debug_source(source);
    std::string type_name = translate_debug_type(type);

    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            LOG(ERROR) << id << " - Message from " << source_name << " of type " << type_name << ": " << message;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            LOG(INFO) << id << " - Message from " << source_name << " of type " << type_name << ": " << message;
            break;
        case GL_DEBUG_SEVERITY_LOW:
            LOG(DEBUG)<< id << " - Message from " << source_name << " of type " << type_name << ": " << message;
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            LOG(TRACE)<< id << " - Message from " << source_name << " of type " << type_name << ": " << message;
            break;
        default:
            LOG(INFO) << id << " - Message from " << source_name << " of type " << type_name << ": " << message;
    }
}

void nova_renderer::enable_debug() {
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debug_logger, NULL);
}


