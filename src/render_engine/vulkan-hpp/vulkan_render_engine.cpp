/*!
 * \author ddubois
 * \date 03-Apr-19.
 */

#include "vulkan_render_engine.hpp"
#include "../../util/logger.hpp"
#include "vk_structs.hpp"
#include "vulkan_command_list.hpp"
#include "vulkan_utils.hpp"

#ifdef NOVA_LINUX
#define VK_USE_PLATFORM_XLIB_KHR // Use X11 for window creating on Linux... TODO: Wayland?
#define NOVA_VK_XLIB
#include "x11_window.hpp"
#include <vulkan/vulkan_xlib.h>
#include "../../util/linux_utils.hpp"

#elif defined(NOVA_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#define NOVA_USE_WIN32
#include "../../util/windows.hpp"
#include <set>
#include <vulkan/vulkan_win32.h>
#include "../../windowing/win32_window.hpp"
#endif

#ifdef ENABLE_VULKAN
namespace nova::renderer::rhi {

};

#define RADX_IMPLEMENTATION
#include "radx/radx_implement.inl"
#endif
