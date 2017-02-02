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
using namespace nova;

#define TEXTURE_MANAGER nova_renderer::instance->get_texture_manager()
#define INPUT_HANDLER nova_renderer::instance->get_input_handler()
// runs in thread 5

NOVA_API void initialize() {
    nova_renderer::init();
}

NOVA_API void add_texture(mc_atlas_texture & texture, int texture_type) {
    TEXTURE_MANAGER.add_texture(
            texture,
            nova::texture_manager::texture_type::all_values()[texture_type]
    );
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

NOVA_API void execute_frame() {
    nova_renderer::instance->render_frame();
}

NOVA_API bool should_close() {
    return nova_renderer::instance->should_end();
}

NOVA_API void send_gui_buffer_command(mc_gui_send_buffer_command * command) {
    nova_renderer::instance->get_mesh_store().add_gui_buffers(command);
}

NOVA_API void clear_gui_buffers() {
    nova_renderer::instance->get_mesh_store().remove_gui_render_objects();
}

NOVA_API void set_string_setting(const char * setting_name, const char * setting_value) {
    settings& settings = nova_renderer::instance->get_render_settings();
    settings.get_options()[setting_name] = setting_value;
    settings.update_config_changed();
}

NOVA_API struct mouse_button_event  get_next_mouse_button_event() {
	return INPUT_HANDLER.dequeue_mouse_button_event();
}

NOVA_API struct mouse_position_event  get_next_mouse_position_event() {
	return INPUT_HANDLER.dequeue_mouse_position_event();
}

NOVA_API struct key_press_event get_next_key_press_event()
{
	return INPUT_HANDLER.dequeue_key_press_event();
}

NOVA_API struct key_char_event get_next_key_char_event()
{
	return  INPUT_HANDLER.dequeue_key_char_event();
}

