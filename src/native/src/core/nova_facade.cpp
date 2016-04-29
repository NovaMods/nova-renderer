/*!
 * \brief Provides implementations for all the functions that this library provides
 *
 * \author David
 */

#include <iostream>

#define DLL_EXPORT
#include "nova.h"

void init_nova() {
    nova_renderer::init_instance();
}

void add_texture(mc_texture texture) {
    nova_renderer::get_instance()->add_texture(texture);
}
