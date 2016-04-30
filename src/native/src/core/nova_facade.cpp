/*!
 * \brief Provides implementations for all the functions that this library provides
 *
 * \author David
 */

#include "nova.h"
#include "nova_renderer.h"

#define TEXTURE_MANAGER nova_renderer::instance->get_texture_manager()

void init_nova() {
    nova_renderer::init_instance();
}

void add_texture(mc_texture texture) {
    TEXTURE_MANAGER->add_texture(texture);
}

void reset_texture_manager() {
    TEXTURE_MANAGER->reset();
}

void finalize_textures() {
    TEXTURE_MANAGER->finalize_textures();
}
