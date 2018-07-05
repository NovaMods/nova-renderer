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

#include <nova/profiler.h>
#include "nova.h"
#include "../render/nova_renderer.h"
#include "../render/objects/resources/texture_manager.h"
#include "../input/InputHandler.h"
#include "../render/windowing/glfw_vk_window.h"
#include "../utils/utils.h"
#include "../render/objects/meshes/mesh_store.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

using namespace nova;

#define NOVA_RENDERER nova_renderer::instance
#define TEXTURE_MANAGER NOVA_RENDERER->get_shader_resources()->get_texture_manager()
#define INPUT_HANDLER NOVA_RENDERER->get_input_handler()
#define MESH_STORE NOVA_RENDERER->get_mesh_store()

// runs in thread 5

NOVA_API void initialize() {
    nova_renderer::init();
}

NOVA_API void add_texture(mc_atlas_texture* texture) {
    NOVA_PROFILER_SCOPE;
    TEXTURE_MANAGER.add_texture(*texture);
}

NOVA_API void reset_texture_manager() {
    NOVA_PROFILER_SCOPE;
    TEXTURE_MANAGER.reset();
}

NOVA_API void send_lightmap_texture(int* data, int count, int width, int height) {
    NOVA_PROFILER_SCOPE;
    auto& lightmap = TEXTURE_MANAGER.get_texture("NovaLightmap");
    lightmap.set_data(data, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
}

NOVA_API void add_texture_location(mc_texture_atlas_location* location) {
    NOVA_PROFILER_SCOPE;
    TEXTURE_MANAGER.add_texture_location(*location);
}

NOVA_API int get_max_texture_size() {
    return TEXTURE_MANAGER.get_max_texture_size();
}

NOVA_API void add_chunk_geometry_for_filter(const char* filter_name, mc_chunk_render_object * chunk) {
    NOVA_PROFILER_SCOPE;
    MESH_STORE.add_chunk_render_object(std::string(filter_name), *chunk);
}

NOVA_API void execute_frame() {
    NOVA_PROFILER_SCOPE;
    NOVA_RENDERER->render_frame();
    LOG(INFO) << "execute_frame done";
}

NOVA_API void set_fullscreen(int fullscreen) {
    NOVA_PROFILER_SCOPE;
    bool temp_bool = false;
    if(fullscreen == 1) {
        temp_bool = true;
    }
    NOVA_RENDERER->get_game_window().set_fullscreen(temp_bool);
}

NOVA_API bool should_close() {
    return NOVA_RENDERER->should_end();
}

NOVA_API bool display_is_active() {
    return NOVA_RENDERER->get_game_window().is_active();
}

NOVA_API void add_gui_geometry(const char * geo_type, mc_gui_geometry * gui_geometry) {
    NOVA_PROFILER_SCOPE;
    NOVA_RENDERER->get_mesh_store().add_gui_buffers(geo_type, gui_geometry);
}

NOVA_API struct window_size get_window_size()
{
    glm::vec2 size = NOVA_RENDERER->get_game_window().get_size();
    return {(int )size.y,(int)size.x};
}

NOVA_API void clear_gui_buffers() {
    NOVA_PROFILER_SCOPE;
    NOVA_RENDERER->get_mesh_store().remove_gui_render_objects();
}

NOVA_API void set_string_setting(const char * setting_name, const char * setting_value) {
    NOVA_PROFILER_SCOPE;
    settings& settings = NOVA_RENDERER->get_render_settings();
    settings.get_options()["settings"][setting_name] = setting_value;
    settings.update_config_changed();
}

NOVA_API void set_float_setting(const char * setting_name, float setting_value) {
    NOVA_PROFILER_SCOPE;
    settings& settings = NOVA_RENDERER->get_render_settings();
    settings.get_options()["settings"][setting_name] = setting_value;
    settings.update_config_changed();
}

NOVA_API void set_player_camera_transform(double x, double y, double z, float yaw, float pitch) {
    NOVA_PROFILER_SCOPE;
    auto& player_camera = NOVA_RENDERER->get_player_camera();

    player_camera.position = {x, y, z};
    player_camera.rotation = {yaw, pitch};
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

NOVA_API char* get_materials_and_filters() {
    NOVA_PROFILER_SCOPE;
    auto& materials = NOVA_RENDERER->get_materials();

    int num_chars = 0;
    for(auto& mat : materials) {
        num_chars += mat.name.size();
        num_chars += mat.geometry_filter.size();
        num_chars += 2;
    }

    LOG(DEBUG) << "There are a total of " << materials.size() << " shaders, which have a total of " << num_chars << " needed for their filters and names";

    auto* filters = new char[num_chars];
    int write_pos = 0;
    for(auto& m : materials) {
        std::strcpy(&filters[write_pos], m.name.data());
        write_pos += m.name.size();

        filters[write_pos] = '\n';
        write_pos++;

        std::strcpy(&filters[write_pos], m.geometry_filter.data());
        write_pos += m.geometry_filter.size();

        filters[write_pos] = '\n';
        write_pos++;
    }

    filters[num_chars - 1] = '\0';
    return filters;
}
