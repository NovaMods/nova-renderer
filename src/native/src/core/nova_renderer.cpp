//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

std::unique_ptr<nova_renderer> nova_renderer::instance;
pthread_t nova_renderer::render_thread;

void * run_render(void * ignored) {
    nova_renderer::instance = std::unique_ptr<nova_renderer>(new nova_renderer());

    while(!nova_renderer::instance->should_end()) {
        nova_renderer::instance->update();
        nova_renderer::instance->render_frame();
    }

    return nullptr;
}

nova_renderer::nova_renderer() : gui_renderer_instance(tex_manager, shaders, ubo_manager), nova_config("config/config.json") {

    nova_config.register_change_listener(&game_window);
    nova_config.register_change_listener(&shaders);
    nova_config.register_change_listener(&ubo_manager);

    nova_config.update_config_loaded();
    nova_config.update_config_changed();

    shaders.link_up_uniform_buffers(ubo_manager);

    enable_debug();

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

nova_renderer::~nova_renderer() {
    game_window.destroy();
}

void nova_renderer::render_frame() {
    // Clear to the clear color
    glClear(GL_COLOR_BUFFER_BIT);

    // Render GUI to GUI buffer
    gui_renderer_instance.render();

    // Render solid geometry
    // Render entities
    // Render transparent things

    game_window.end_frame();
}

bool nova_renderer::should_end() {
    // If the window wants to close, the user probably clicked on the "X" button
    return game_window.should_close();
}

void nova_renderer::init_instance() {
    pthread_create(&render_thread, nullptr, run_render, nullptr);
    pthread_join(render_thread, nullptr);
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

void nova_renderer::update() {
    /*
    nova_config.update();
    game_window.update();
    tex_manager.update();
    shaders.update();
    ubo_manager.update();
    */

    gui_renderer_instance.update();
}


