//
// Created by jannis on 18.09.18.
//

#pragma once

#include <vulkan/vulkan.h>

#include "nova_renderer/shaderpack_data.hpp"

#ifdef Always // RIP X11 and its macros
#undef Always
#endif

#ifdef ENABLE_VULKAN
namespace nova::renderer {
} // namespace nova::renderer
#endif
