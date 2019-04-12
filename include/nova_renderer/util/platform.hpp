/*!
 * \brief Figures out what APIs are supported on the target platform
 */

#pragma once

/*
 * Right now this is boring - but this will eventually determine the rendering API based on the platform we're compiling
 * for
 *
 * The promise of cross-platform rendering APIs is nice, but in a few short months of Vulkan I've already run into all
 * kinds of problems. Khronos' promises ring empty
 */

#ifdef _WIN32
#define SUPPORT_DX12
#define NOVA_WINDOWS
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
#define NOVA_LINUX
#endif
