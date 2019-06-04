//
// Created by jannis on 18.09.18.
//

#pragma once

#include <nova_renderer/shaderpack_data.hpp>
#include <vulkan/vulkan.h>

#ifdef Always // RIP X11 and its macros
#undef Always
#endif

#ifdef LEGACY_VULKAN
namespace nova::renderer {
} // namespace nova::renderer
#endif
