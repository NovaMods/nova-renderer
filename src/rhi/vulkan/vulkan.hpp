#pragma once

#include "nova_renderer/util/platform.hpp"

// vulkan.h is a C header, so it does C things, and my C++ linter is like "wati no be more C++" but I ain't about to
// rewrite vulkan.h
#pragma warning(push, 0)
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
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif
#pragma warning(pop)

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
#ifdef Bool
#undef Bool
#endif
#ifdef Status
#undef Status
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif