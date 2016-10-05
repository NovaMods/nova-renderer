/*!
 * \brief Provides implementations for all the functions that this library provides
 *
 * I'd like for the functions in this file to be a facade sort of thing, but idk if that will happen. These functions
 * look a lot like simple passthroughs.
 *
 * Although, many of them do things to convert from C-style things to C++-style things, so that's cool I guess
 *
 * \author David
 */

#include "nova.h"
#include "view/nova_renderer.h"

#define TEXTURE_MANAGER nova::view::nova_renderer::instance->get_texture_manager()

NOVA_EXPORT void init_nova() {
    nova::view::nova_renderer::init_instance();
}

NOVA_EXPORT void add_texture(mc_atlas_texture & texture, int atlas_type, int texture_type) {
    TEXTURE_MANAGER.add_texture(
            texture,
            static_cast<nova::model::texture_manager::atlas_type>(atlas_type),
            static_cast<nova::model::texture_manager::texture_type >(texture_type)
    );
}

NOVA_EXPORT void reset_texture_manager() {
    TEXTURE_MANAGER.reset();
}

NOVA_EXPORT void add_texture_location(mc_texture_atlas_location location) {
    TEXTURE_MANAGER.add_texture_location(location);
}

NOVA_EXPORT int get_max_texture_size() {
    return TEXTURE_MANAGER.get_max_texture_size();
}

NOVA_EXPORT void update_renderer() {
    // nova_renderer::instance->update();
}

NOVA_EXPORT void send_render_command(mc_render_command * command) {
    // TODO: Pass down the render command
    // nova_renderer::instance->render_frame();
}

NOVA_EXPORT bool should_close() {
    return nova::view::nova_renderer::instance->should_end();
}

NOVA_EXPORT void send_change_gui_screen_command(mc_set_gui_screen_command * set_gui_screen) {
    nova::view::nova_renderer::instance->get_model().set_current_screen(&set_gui_screen->screen);
}
