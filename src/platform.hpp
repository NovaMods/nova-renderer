/*!
 * \brief Figures out what APIs are supported on the target platform
 *
 * \author ddubois
 * \date 17-Aug-18.
 */

#ifndef NOVA_RENDERER_PLATFORM_H
#define NOVA_RENDERER_PLATFORM_H

/*
 * Right now this is boring - but this will eventually determine the rendering API based on the platform we're compiling
 * for
 *
 * The promise of cross-platform rendering APIs is nice, but in a few short months of Vulkan I've already run into all
 * kinds of problems. Khronos' promises ring empty
 */

#ifdef _WIN32
#define SUPPORT_DX12 1
#define NOVA_WINDOWS 1
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
#define NOVA_LINUX 1
#endif

#endif // NOVA_RENDERER_PLATFORM_H
