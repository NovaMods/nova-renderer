/*!
 * \author ddubois 
 * \date 17-Oct-17.
 */

#include "device_memory_allocator.h"

namespace nova {
    device_memory_allocator device_memory_allocator::instance;

    device_memory_allocator &device_memory_allocator::get_instance() {
        return instance;
    }

    vk::Image device_memory_allocator::make_new_texture(vk::Device device, vk::ImageCreateInfo create_info) {
        return vk::Image();
    }
}
