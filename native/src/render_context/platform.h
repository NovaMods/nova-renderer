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
 * kinda of problems. Khronos' promises ring empty
 */

#define SUPPORT_VULKAN

#endif //NOVA_RENDERER_PLATFORM_H
