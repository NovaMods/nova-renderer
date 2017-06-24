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

using namespace nova;

#define NOVA_RENDERER nova_renderer::instance
#define TEXTURE_MANAGER NOVA_RENDERER->get_texture_manager()
#define INPUT_HANDLER NOVA_RENDERER->get_input_handler()
#define MESH_STORE NOVA_RENDERER->get_mesh_store()
// runs in thread 5

NOVA_API void initialize() {
    nova_renderer::init();
}

NOVA_API void add_texture(mc_atlas_texture & texture) {
    TEXTURE_MANAGER.add_texture(texture);
}

NOVA_API void reset_texture_manager() {
    TEXTURE_MANAGER.reset();
}

NOVA_API void add_texture_location(mc_texture_atlas_location location) {
    TEXTURE_MANAGER.add_texture_location(location);
}

NOVA_API int get_max_texture_size() {
    return TEXTURE_MANAGER.get_max_texture_size();
}

NOVA_API void add_chunk(mc_chunk & chunk) {
    MESH_STORE.add_or_update_chunk(chunk);
}

NOVA_API void register_block_definition(int id, mc_block_definition blockDefinition) {
    MESH_STORE.get_chunk_builder().get_block_definitions()[id] = blockDefinition;
}

NOVA_API void register_simple_model(const char * model_name, mc_simple_model * model) {
    MESH_STORE.register_model(std::string(model_name), *model);
}

NOVA_API void deregister_model(const char * model_name) {
    MESH_STORE.deregister_model(std::string(model_name));
}

NOVA_API void execute_frame() {
    NOVA_RENDERER->render_frame();
}

NOVA_API void set_fullscreen(int fullscreen) {
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

NOVA_API void send_gui_buffer_command(mc_gui_send_buffer_command * command) {
    NOVA_RENDERER->get_mesh_store().add_gui_buffers(command);
}

NOVA_API struct window_size get_window_size()
{
    glm::vec2 size = NOVA_RENDERER->get_game_window().get_size();
    return {(int )size.y,(int)size.x};
}

NOVA_API void clear_gui_buffers() {
    NOVA_RENDERER->get_mesh_store().remove_gui_render_objects();
}

NOVA_API void set_string_setting(const char * setting_name, const char * setting_value) {
    settings& settings = NOVA_RENDERER->get_render_settings();
    settings.get_options()["settings"][setting_name] = setting_value;
    settings.update_config_changed();
}

NOVA_API void set_float_setting(const char * setting_name, float setting_value) {
    settings& settings = NOVA_RENDERER->get_render_settings();
    settings.get_options()["settings"][setting_name] = setting_value;
    settings.update_config_changed();
}

NOVA_API void set_player_camera_transform(double x, double y, double z, float yaw, float pitch) {
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
    NOVA_RENDERER->get_game_window().set_mouse_grabbed(!!grabbed);
}
