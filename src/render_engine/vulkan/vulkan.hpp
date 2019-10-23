#pragma once

#include "nova_renderer/util/platform.hpp"

#include <vulkan/vulkan.h>

// I really don't know how Khronos/anyone else gets vulkan.h to work. Doing this manually feels dirty, and not in a
// good way, but it works 
#ifdef NOVA_LINUX
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

#elif defined(NOVA_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

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
