//
// Created by David on 25-Dec-15.
//

#include "nova_renderer.h"
#include "../utils/utils.h"
#include "../data_loading/loaders/loaders.h"

#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    nova_renderer::nova_renderer(){
		enable_debug();
		render_settings->register_change_listener(&ubo_manager);
		render_settings->register_change_listener(&game_window);
        render_settings->register_change_listener(this);

        render_settings->update_config_loaded();
		render_settings->update_config_changed();

        init_opengl_state();
    }

    void nova_renderer::init_opengl_state() const {
        glClearColor(0.0, 0.0, 0.0, 1.0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearDepth(1.0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    nova_renderer::~nova_renderer() {
        game_window.destroy();
    }

    void nova_renderer::render_frame() {
        // Clear to the clear color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_shadow_pass();

        render_gbuffers();

        render_composite_passes();

        render_final_pass();

        // We want to draw the GUI on top of the other things, so we'll render it last
        // Additionally, I could use the stencil buffer to not draw MC underneath the GUI. Could be a fun
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
        gl_shader_program &gui_shader = (*loaded_shaderpack)["gui"];
        gui_shader.bind();

        // Render GUI objects
        std::vector<render_object *> gui_geometry = meshes.get_meshes_for_shader("gui");
        for(const auto *geom : gui_geometry) {
            if (geom->color_texture != texture_manager::texture_type::no_texture) {
                auto color_texture = textures.get_texture_atlas(geom->color_texture);
                color_texture.bind(0);
            }
            geom->geometry->set_active();
            geom->geometry->draw();
        }
    }

    bool nova_renderer::should_end() {
        // If the window wants to close, the user probably clicked on the "X" button
        return game_window.should_close();
    }

	std::unique_ptr<settings> nova_renderer::render_settings;

    void nova_renderer::init() {
		render_settings = std::make_unique<settings>("config/config.json");
	
		instance = std::make_unique<nova_renderer>();
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
                return "error";
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

    void APIENTRY
    debug_logger(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
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

    void nova_renderer::on_config_change(nlohmann::json &new_config) {
		
		auto& shaderpack_name = new_config["loadedShaderpack"];
        load_new_shaderpack(shaderpack_name);
    }

    void nova_renderer::on_config_loaded(nlohmann::json &config) {
        // TODO: Probably want to do some setup here, don't need to do that now
    }

    settings &nova_renderer::get_render_settings() {
        return *render_settings;
    }

    texture_manager &nova_renderer::get_texture_manager() {
        return textures;
    }

	glfw_gl_window &nova_renderer::get_game_window() {
		return game_window;
	}

	input_handler &nova_renderer::get_input_handler() {
		return inputs;
	}

    mesh_store &nova_renderer::get_mesh_store() {
        return meshes;
    }

    void nova_renderer::load_new_shaderpack(const std::string &new_shaderpack_name) {
		
        LOG(INFO) << "Loading shaderpack " << new_shaderpack_name;
        loaded_shaderpack = std::experimental::make_optional<shaderpack>(load_shaderpack(new_shaderpack_name));
        meshes.set_shaderpack(*loaded_shaderpack);
        LOG(INFO) << "Loading complete";
		
        link_up_uniform_buffers(loaded_shaderpack->get_loaded_shaders(), ubo_manager);
        LOG(DEBUG) << "Linked up UBOs";
    }

    void nova_renderer::deinit() {
        instance.release();
    }

    void link_up_uniform_buffers(std::unordered_map<std::string, gl_shader_program> &shaders, uniform_buffer_store &ubos) {
        nova::foreach(shaders, [&](auto shader) { ubos.register_all_buffers_with_shader(shader.second); });
    }
}

