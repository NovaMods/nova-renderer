#pragma once

#include "nova_renderer/util/platform.hpp"

#ifdef NOVA_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(NOVA_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

#include <vulkan/vulkan.h>

// Thank you, Windows, for being an idiot
#ifdef ERROR
#undef ERROR
#endif
#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif

// Thank you, X11, for proving that the Linux ecosystem has many of the same problems as Windows
#ifdef Always
#undef Always
#endif
#ifdef None
#undef None
#endif
