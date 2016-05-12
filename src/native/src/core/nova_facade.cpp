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

#define DLL_EXPORT

#include "nova.h"
#include "nova_renderer.h"

#define TEXTURE_MANAGER nova_renderer::instance->get_texture_manager()

NOVA_FUNC void init_nova() {
    nova_renderer::init_instance();
}

NOVA_FUNC void add_texture(mc_atlas_texture & texture, int atlas_type, int texture_type) {
    TEXTURE_MANAGER->add_texture(
            texture,
            static_cast<texture_manager::atlas_type>(atlas_type),
            static_cast<texture_manager::texture_type >(texture_type)
    );
}

NOVA_FUNC void reset_texture_manager() {
    TEXTURE_MANAGER->reset();
}

NOVA_FUNC void add_texture_location(mc_texture_atlas_location location) {
    TEXTURE_MANAGER->add_texture_location(location);
}

NOVA_FUNC int get_max_texture_size() {
    return TEXTURE_MANAGER->get_max_texture_size();
}

NOVA_FUNC void send_render_command(mc_render_command * cmd) {
    // TODO: Write this
}

NOVA_FUNC void do_test_render() {
    nova_renderer::instance->render_frame();
}

NOVA_FUNC bool should_close() {
    return nova_renderer::instance->should_end();
}

