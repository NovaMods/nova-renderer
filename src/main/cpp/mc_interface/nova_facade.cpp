/*!
 * \brief Provides implementations for all the functions that this library provides
 *
 * I'd like for the functions in this file to be a facade sort of thing, but idk if that will happen. These functions
 * look a lot like simple passthroughs.
 *
 * Although, many of them do things to convert from C-style things to C++-style things, so that's cool I guess
 *
 * The function in this file simply set data. Data is copied out of Minecraft objects. This might be a bit slow, but I
 * don't want to save a pointer that gets re-allocated by the JVM. I don't trust it enough (although maybe I should?)
 *
 * \author David
 */

#include "glad/glad.h"
#include "nova.h"
#include "../utils/export.h"
#include "../render/nova_renderer.h"
#include "../render/objects/textures/texture_manager.h"
#include "../data_loading/settings.h"
#include "../input/InputHandler.h"
#include "../render/windowing/glfw_gl_window.h"
#include "../utils/utils.h"
#include "../utils/profiler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../utils/stb_image_write.h"

using namespace nova;

#define NOVA_RENDERER nova_renderer::instance
#define TEXTURE_MANAGER NOVA_RENDERER->get_texture_manager()
#define INPUT_HANDLER NOVA_RENDERER->get_input_handler()
#define MESH_STORE NOVA_RENDERER->get_mesh_store()

#define PROFILER nova::profiler
// runs in thread 5

NOVA_API void initialize() {
    PROFILER::start("initialize");
    nova_renderer::init();
    PROFILER::end("initialize");
}

NOVA_API void add_texture(mc_atlas_texture & texture) {
    PROFILER::start("add_texture");
    TEXTURE_MANAGER.add_texture(texture);
    PROFILER::end("add_texture");
}

NOVA_API void reset_texture_manager() {
    PROFILER::start("reset_texture_manager");
    TEXTURE_MANAGER.reset();
    PROFILER::end("reset_texture_mamager");
}

NOVA_API void send_lightmap_texture(int* data, int count, int width, int height) {
    auto size = glm::ivec2{width, height};
    TEXTURE_MANAGER.update_texture("lightmap", data, size, GL_BGRA, GL_UNSIGNED_BYTE);
    auto& lightmap = TEXTURE_MANAGER.get_texture("lightmap");
    lightmap.bind(4);
}

NOVA_API void add_texture_location(mc_texture_atlas_location location) {
    PROFILER::start("add_texture_location");
    TEXTURE_MANAGER.add_texture_location(location);
    PROFILER::end("add_texture_location");
}

NOVA_API int get_max_texture_size() {
    return TEXTURE_MANAGER.get_max_texture_size();
}

NOVA_API void add_chunk_geometry_for_filter(const char* filter_name, mc_chunk_render_object * chunk) {
    PROFILER::start("add_chunk_geometry_for_filter");
    MESH_STORE.add_chunk_render_object(std::string(filter_name), *chunk);
    PROFILER::end("add_chunk_geometry_for_filter");
}

NOVA_API void execute_frame() {
    PROFILER::start("execute_frame");
    NOVA_RENDERER->render_frame();
    PROFILER::end("execute_frame");
}

NOVA_API void set_fullscreen(int fullscreen) {
    PROFILER::start("set_fullscreen");
    bool temp_bool = false;
    if(fullscreen == 1) {
        temp_bool = true;
    }
    NOVA_RENDERER->get_game_window().set_fullscreen(temp_bool);
    PROFILER::end("set_fullscreen");
}

NOVA_API bool should_close() {
    return NOVA_RENDERER->should_end();
}

NOVA_API bool display_is_active() {
    return NOVA_RENDERER->get_game_window().is_active();
}

NOVA_API void add_gui_geometry(mc_gui_geometry * gui_geometry) {
    PROFILER::start("add_gui_geometry");
    NOVA_RENDERER->get_mesh_store().add_gui_buffers(gui_geometry);
    PROFILER::end("add_gui_geometry");
}

NOVA_API struct window_size get_window_size()
{
    glm::vec2 size = NOVA_RENDERER->get_game_window().get_size();
    return {(int )size.y,(int)size.x};
}

NOVA_API void clear_gui_buffers() {
    PROFILER::start("clear_gui_buffers");
    NOVA_RENDERER->get_mesh_store().remove_gui_render_objects();
    PROFILER::end("clear_gui_buffers");
}

NOVA_API void set_string_setting(const char * setting_name, const char * setting_value) {
    PROFILER::start("set_string_setting");
    settings& settings = NOVA_RENDERER->get_render_settings();
    settings.get_options()["settings"][setting_name] = setting_value;
    settings.update_config_changed();
    PROFILER::end("set_string_setting");
}

NOVA_API void set_float_setting(const char * setting_name, float setting_value) {
    PROFILER::start("set_float_setting");
    settings& settings = NOVA_RENDERER->get_render_settings();
    settings.get_options()["settings"][setting_name] = setting_value;
    settings.update_config_changed();
    PROFILER::end("set_float_setting");
}

NOVA_API void set_player_camera_transform(double x, double y, double z, float yaw, float pitch) {
    PROFILER::start("set_player_camera_transform");
    auto& player_camera = NOVA_RENDERER->get_player_camera();

    player_camera.position = {x, y, z};
    player_camera.rotation = {yaw, pitch};
    PROFILER::end("set_player_camera_transform");
}

NOVA_API struct mouse_button_event  get_next_mouse_button_event() {
	return INPUT_HANDLER.dequeue_mouse_button_event();
}

NOVA_API struct mouse_position_event  get_next_mouse_position_event() {
	return INPUT_HANDLER.dequeue_mouse_position_event();
}

NOVA_API struct mouse_scroll_event  get_next_mouse_scroll_event() {
    return INPUT_HANDLER.dequeue_mouse_scroll_event();
}

NOVA_API struct key_press_event get_next_key_press_event()
{
	return INPUT_HANDLER.dequeue_key_press_event();
}

NOVA_API struct key_char_event get_next_key_char_event()
{
	return  INPUT_HANDLER.dequeue_key_char_event();
}

NOVA_API void set_mouse_grabbed(int grabbed) {
    NOVA_RENDERER->get_game_window().set_mouse_grabbed(grabbed != 0);
}

NOVA_API int get_num_loaded_shaders() {
    return static_cast<int>(NOVA_RENDERER->get_shaders()->get_loaded_shaders().size());
}

NOVA_API char* get_shaders_and_filters() {
    PROFILER::start("set_shaders_and_filters");
    auto& shaders = NOVA_RENDERER->get_shaders()->get_loaded_shaders();

    int num_chars = 0;
    for(auto& s : shaders) {
        num_chars += s.first.size();
        num_chars += s.second.get_filter().size();
        num_chars += 2;
    }

    auto* filters = new char[num_chars];
    int write_pos = 0;
    for(auto& entry : shaders) {
        std::strcpy(&filters[write_pos], entry.first.data());
        write_pos += entry.first.size();

        filters[write_pos] = '\n';
        write_pos++;

        std::strcpy(&filters[write_pos], entry.second.get_filter().data());
        write_pos += entry.second.get_filter().size();

        filters[write_pos] = '\n';
        write_pos++;
    }

    filters[num_chars - 1] = '\0';
    PROFILER::end("set_shaders_and_filters");
    return filters;
}
