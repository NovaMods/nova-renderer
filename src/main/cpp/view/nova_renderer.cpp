//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"

#include <easylogging++.h>
#include <utils/utils.h>

INITIALIZE_EASYLOGGINGPP

namespace nova {
    namespace view {

        std::unique_ptr<nova_renderer> nova_renderer::instance;
        std::unique_ptr<std::thread> nova_renderer::render_thread;

        void run_render() {
            nova_renderer::instance = std::unique_ptr<nova_renderer>(new nova_renderer());

            while(!nova_renderer::instance->should_end()) {
                nova_renderer::instance->render_frame();
            }
        }

        nova_renderer::nova_renderer() {
            model.get_render_settings().register_change_listener(&ubo_manager);
            model.trigger_config_update();

            link_up_uniform_buffers(model.get_all_shaders(), ubo_manager);

            enable_debug();

            init_opengl_state();
        }

        void nova_renderer::init_opengl_state() const { glClearColor(0.0, 0.0, 0.0, 1.0); }

        nova_renderer::~nova_renderer() {
            game_window.destroy();
        }

        void nova_renderer::render_frame() {
            // Clear to the clear color
            glClear(GL_COLOR_BUFFER_BIT);

            render_shadow_pass();

            render_gbuffers();

            render_composite_passes();

            render_final_pass();

            // We want to draw the GUI on top of the other things, so we'll render it last
            // Additionally, I could use the stencil buffer to not graw MC underneath the GUI. Could be a fun
            // optimization - I'd have to watch out for when the user hides the GUI, though. I can just re-render the
            // stencil buffer when the GUI screen changes
            render_gui();

            game_window.end_frame();
        }

        void nova_renderer::render_shadow_pass() {

        }

        void nova_renderer::render_gbuffers() {

        }

        void nova_renderer::render_composite_passes() {

        }

        void nova_renderer::render_final_pass() {

        }

        void nova_renderer::render_gui() {
            // Bind all the GUI data
            model.get_shader_program("gui").bind();

            model::gl_vertex_buffer& gui_geometry = gui_geom_cache.get_gui_geometry();
            gui_geometry.draw();
        }

        bool nova_renderer::should_end() {
            // If the window wants to close, the user probably clicked on the "X" button
            return game_window.should_close();
        }

        void nova_renderer::init_instance() {
            render_thread = std::unique_ptr<std::thread>(new std::thread(run_render));
            render_thread->detach();
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

        void debug_logger(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                          const void *user_param) {
            std::string source_name = translate_debug_source(source);
            std::string type_name = translate_debug_type(type);

            switch(severity) {
                case GL_DEBUG_SEVERITY_HIGH:
                    LOG(ERROR) << id << " - Message from " << source_name << " of type " << type_name << ": "
                               << message;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    LOG(INFO) << id << " - Message from " << source_name << " of type " << type_name << ": " << message;
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    LOG(DEBUG) << id << " - Message from " << source_name << " of type " << type_name << ": "
                               << message;
                    break;
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    LOG(TRACE) << id << " - Message from " << source_name << " of type " << type_name << ": "
                               << message;
                    break;
                default:
                    LOG(INFO) << id << " - Message from " << source_name << " of type " << type_name << ": " << message;
            }
        }

        void nova_renderer::enable_debug() {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(debug_logger, NULL);
        }

        void nova_renderer::link_up_uniform_buffers(std::vector<nova::model::gl_shader_program *> shaders,
                                                    nova::view::uniform_buffer_store &ubos) {
            nova::foreach(shaders, [&](auto shader) { ubos.register_all_buffers_with_shader(*shader); });
        }

        model::data_model &nova_renderer::get_model() {
            return model;
        }
    }
}

