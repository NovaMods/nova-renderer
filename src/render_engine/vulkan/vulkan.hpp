#pragma once

#include "nova_renderer/util/platform.hpp"

#ifdef NOVA_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(NOVA_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#ifdef Always
#undef Always
#endif
#ifdef None
#undef None
#endif
