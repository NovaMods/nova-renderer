//
// Created by jannis on 30.08.18.
//

#include "vulkan_render_engine.hpp"
#include <set>
#include <vector>
#include "../../util/logger.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include "../../../3rdparty/SPIRV-Cross/spirv_glsl.hpp"
#include "../../loading/shaderpack/render_graph_builder.hpp"
#include "../../loading/shaderpack/shaderpack_loading.hpp"
#include "../../platform.hpp"
#include "../../util/utils.hpp"
#include "../dx12/win32_window.hpp"
#include "vulkan_type_converters.hpp"
#include "vulkan_utils.hpp"
#include "swapchain.hpp"

#ifdef NOVA_LINUX
#include <execinfo.h>
#include <cxxabi.h>

#endif

namespace nova {
    vulkan_render_engine::vulkan_render_engine(const nova_settings& settings, ttl::task_scheduler* task_scheduler) : render_engine(settings, task_scheduler) {
        NOVA_LOG(INFO) << "Initializing Vulkan rendering";

        const settings_options& options = settings.get_options();

        validate_mesh_options(options.vertex_memory_settings);

        const auto& version = options.vulkan.application_version;

        VkApplicationInfo application_info;
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pNext = nullptr;
        application_info.pApplicationName = options.vulkan.application_name.c_str();
        application_info.applicationVersion = VK_MAKE_VERSION(version.major, version.minor, version.patch);
        application_info.pEngineName = "Nova renderer 0.1";
        application_info.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &application_info;
#ifndef NDEBUG
        enabled_validation_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
        create_info.enabledLayerCount = static_cast<uint32_t>(enabled_validation_layer_names.size());
        create_info.ppEnabledLayerNames = enabled_validation_layer_names.data();

        std::vector<const char*> enabled_extension_names;
        enabled_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef NOVA_LINUX
        enabled_extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif NOVA_WINDOWS
        enabled_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
#error Unsupported Operating system
#endif

#ifndef NDEBUG
        enabled_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        enabled_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size());
        create_info.ppEnabledExtensionNames = enabled_extension_names.data();

        NOVA_THROW_IF_VK_ERROR(vkCreateInstance(&create_info, nullptr, &vk_instance), render_engine_initialization_exception);

        uint32_t num_extensions;
        vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, nullptr);
        gpu.available_extensions.resize(num_extensions);
        vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, gpu.available_extensions.data());

        std::stringstream ss;
        for(const VkExtensionProperties& props : gpu.available_extensions) {
            ss << "\t" << props.extensionName << " version " << props.specVersion << "\n";
        }

        NOVA_LOG(TRACE) << "Supported extensions:\n" << ss.str();

#ifndef NDEBUG
        vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT"));
        vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugReportCallbackEXT"));

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_create_info.pNext = nullptr;
        debug_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&debug_report_callback);
        debug_create_info.pUserData = this;

        NOVA_THROW_IF_VK_ERROR(vkCreateDebugUtilsMessengerEXT(vk_instance, &debug_create_info, nullptr, &debug_callback), render_engine_initialization_exception);
#endif
        // First we open the window. This doesn't depend on anything except the VkInstance/ This method also creates
        // the VkSurfaceKHR we can render to
        open_window(settings.get_options().window.width, settings.get_options().window.height);

        // Create the device. This depends on both the VkInstance and the VkSurfaceKHR: we need the VkSurfaceKHR to
        // make sure we find a device that can present to that surface
        create_device();

        create_per_thread_command_pools();

        // Create the swapchain. This depends on the VkInstance, VkPhysicalDevice, our pre-thread command pools, and
        // VkSurfaceKHR. This method also fills out a lot of the information in our vk_gpu_info
        create_swapchain();

        create_memory_allocator();
        mesh_memory = std::make_unique<compacting_block_allocator>(settings.get_options().vertex_memory_settings, vma_allocator, graphics_family_index, copy_family_index);

        create_global_sync_objects();
        create_per_thread_descriptor_pools();
        create_default_samplers();

#ifndef NDEBUG
        vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
        if(vkSetDebugUtilsObjectNameEXT == nullptr) {
            NOVA_LOG(ERROR) << "Could not load the debug name function";
        }
#endif
    }

    vulkan_render_engine::~vulkan_render_engine() {
        vkDeviceWaitIdle(device);
    }

    void vulkan_render_engine::transition_dynamic_textures() {
        NOVA_LOG(TRACE) << "Transitioning dynamic textures to color attachment layouts";
        std::vector<VkImageMemoryBarrier> color_barriers;
        color_barriers.reserve(textures.size());

        std::vector<VkImageMemoryBarrier> depth_barriers;
        depth_barriers.reserve(textures.size());

        for(const auto& [texture_name, texture] : textures) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = texture.image;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            if(texture.format == VK_FORMAT_D24_UNORM_S8_UINT || texture.format == VK_FORMAT_D32_SFLOAT) {
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                depth_barriers.push_back(barrier);

            } else {
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                color_barriers.push_back(barrier);
            }
        }

        const VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        vkAllocateCommandBuffers(device, &alloc_info, &cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmds, &begin_info);

        vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, color_barriers.size(), color_barriers.data());

        vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, depth_barriers.size(), depth_barriers.data());

        vkEndCommandBuffer(cmds);

        VkFence transition_done_fence;

        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        vkCreateFence(device, &fence_create_info, nullptr, &transition_done_fence);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        vkQueueSubmit(graphics_queue, 1, &submit_info, transition_done_fence);
        vkWaitForFences(device, 1, &transition_done_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkFreeCommandBuffers(device, command_pool, 1, &cmds);

        dynamic_textures_need_to_transition = false;
    }

    void vulkan_render_engine::create_swapchain() {
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu.phys_device, surface, &gpu.surface_capabilities), render_engine_initialization_exception);

        uint32_t num_surface_formats;
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, nullptr), render_engine_initialization_exception);
        gpu.surface_formats.resize(num_surface_formats);
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu.phys_device, surface, &num_surface_formats, gpu.surface_formats.data()), render_engine_initialization_exception);

        uint32_t num_surface_present_modes;
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_formats, nullptr), render_engine_initialization_exception);
        gpu.present_modes.resize(num_surface_formats);
        NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(gpu.phys_device, surface, &num_surface_formats, gpu.present_modes.data()), render_engine_initialization_exception);

        swapchain = std::make_unique<swapchain_manager>(3, *this, window->get_window_size());
    }

    void vulkan_render_engine::open_window(uint32_t width, uint32_t height) {
#ifdef NOVA_LINUX
        window = std::make_shared<x11_window>(width, height);

        VkXlibSurfaceCreateInfoKHR x_surface_create_info;
        x_surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        x_surface_create_info.pNext = nullptr;
        x_surface_create_info.flags = 0;
        x_surface_create_info.dpy = window->get_display();
        x_surface_create_info.window = window->get_x11_window();

        NOVA_THROW_IF_VK_ERROR(vkCreateXlibSurfaceKHR(vk_instance, &x_surface_create_info, nullptr, &surface), render_engine_initialization_exception);
#elif NOVA_WINDOWS
        window = std::make_shared<win32_window>(width, height);

        VkWin32SurfaceCreateInfoKHR win32_surface_create = {};
        win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create.hwnd = window->get_window_handle();

        NOVA_THROW_IF_VK_ERROR(vkCreateWin32SurfaceKHR(vk_instance, &win32_surface_create, nullptr, &surface), render_engine_initialization_exception);

#else
#error Unsuported window system
#endif
    }

    void vulkan_render_engine::validate_mesh_options(const settings_options::block_allocator_settings& options) const {
        if(options.buffer_part_size % sizeof(full_vertex) != 0) {
            throw std::runtime_error("vertex_memory_settings.buffer_part_size must be a multiple of sizeof(full_vertex) (which equals " + std::to_string(sizeof(full_vertex)) + ")");
        }
        if(options.new_buffer_size % options.buffer_part_size != 0) {
            throw std::runtime_error("vertex_memory_settings.new_buffer_size must be a multiple of vertex_memory_settings.buffer_part_size (which equals " + std::to_string(options.buffer_part_size) + ")");
        }
        if(options.max_total_allocation % options.new_buffer_size != 0) {
            throw std::runtime_error("vertex_memory_settings.max_total_allocation must be a multiple of vertex_memory_settings.new_buffer_size (which equals " + std::to_string(options.max_total_allocation) + ")");
        }
    }

    void vulkan_render_engine::create_device() {
        uint32_t device_count;
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr), render_engine_initialization_exception);
        auto* physical_devices = new VkPhysicalDevice[device_count];
        NOVA_THROW_IF_VK_ERROR(vkEnumeratePhysicalDevices(vk_instance, &device_count, physical_devices), render_engine_initialization_exception);

        uint32_t graphics_family_idx = 0xFFFFFFFF;
        uint32_t compute_family_idx = 0xFFFFFFFF;
        uint32_t copy_family_idx = 0xFFFFFFFF;

        for(uint32_t device_idx = 0; device_idx < device_count; device_idx++) {
            graphics_family_idx = 0xFFFFFFFF;
            VkPhysicalDevice current_device = physical_devices[device_idx];
            vkGetPhysicalDeviceProperties(current_device, &gpu.props);

            if(gpu.props.vendorID == 0x8086 && device_count - 1 > device_idx) { // Intel GPU... they are not powerful and we have more available, so skip it
                continue;
            }

            if(!does_device_support_extensions(current_device)) {
                continue;
            }

            uint32_t queue_family_count;
            vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_family_count, nullptr);
            gpu.queue_family_props.resize(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(current_device, &queue_family_count, gpu.queue_family_props.data());

            for(uint32_t queue_idx = 0; queue_idx < queue_family_count; queue_idx++) {
                const VkQueueFamilyProperties current_properties = gpu.queue_family_props[queue_idx];
                if(current_properties.queueCount < 1) {
                    continue;
                }

                VkBool32 supports_present = VK_FALSE;
                NOVA_THROW_IF_VK_ERROR(vkGetPhysicalDeviceSurfaceSupportKHR(current_device, queue_idx, surface, &supports_present), render_engine_initialization_exception);
                const VkQueueFlags supports_graphics = current_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                if(supports_graphics && supports_present == VK_TRUE && graphics_family_idx == 0xFFFFFFFF) {
                    graphics_family_idx = queue_idx;
                }

                const VkQueueFlags supports_compute = current_properties.queueFlags & VK_QUEUE_COMPUTE_BIT;
                if(supports_compute && compute_family_idx == 0xFFFFFFFF) {
                    compute_family_idx = queue_idx;
                }

                const VkQueueFlags supports_copy = current_properties.queueFlags & VK_QUEUE_TRANSFER_BIT;
                if(supports_copy && copy_family_idx == 0xFFFFFFFF) {
                    copy_family_idx = queue_idx;
                }
            }

            if(graphics_family_idx != 0xFFFFFFFF) {
                NOVA_LOG(INFO) << "Selected GPU " << gpu.props.deviceName;
                gpu.phys_device = current_device;
                break;
            }
        }

        if(!gpu.phys_device) {
            throw render_engine_initialization_exception("Failed to find good GPU");
        }

        vkGetPhysicalDeviceFeatures(gpu.phys_device, &gpu.supported_features);

        const float priority = 1.0;

        VkDeviceQueueCreateInfo graphics_queue_create_info{};
        graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphics_queue_create_info.pNext = nullptr;
        graphics_queue_create_info.flags = 0;
        graphics_queue_create_info.queueCount = 1;
        graphics_queue_create_info.queueFamilyIndex = graphics_family_idx;
        graphics_queue_create_info.pQueuePriorities = &priority;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {graphics_queue_create_info};

        VkPhysicalDeviceFeatures physical_device_features{};
        physical_device_features.geometryShader = VK_TRUE;
        physical_device_features.tessellationShader = VK_TRUE;
        physical_device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pNext = nullptr;
        device_create_info.flags = 0;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.pEnabledFeatures = &physical_device_features;
        device_create_info.enabledExtensionCount = 1;
        const char* swapchain_extension = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        device_create_info.ppEnabledExtensionNames = &swapchain_extension;
        device_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_validation_layer_names.size());
        if(!enabled_validation_layer_names.empty()) {
            device_create_info.ppEnabledLayerNames = enabled_validation_layer_names.data();
        }

        NOVA_THROW_IF_VK_ERROR(vkCreateDevice(gpu.phys_device, &device_create_info, nullptr, &device), render_engine_initialization_exception);

        graphics_family_index = graphics_family_idx;
        vkGetDeviceQueue(device, graphics_family_idx, 0, &graphics_queue);
        compute_family_index = compute_family_idx;
        vkGetDeviceQueue(device, compute_family_idx, 0, &compute_queue);
        copy_family_index = copy_family_idx;
        vkGetDeviceQueue(device, copy_family_idx, 0, &copy_queue);

        delete[] physical_devices;
    }

    bool vulkan_render_engine::does_device_support_extensions(VkPhysicalDevice device) {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available.data());

        std::set<std::string> required = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        for(const auto& extension : available) {
            required.erase(extension.extensionName);
        }

        return required.empty();
    }

    void vulkan_render_engine::create_memory_allocator() {
        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.physicalDevice = gpu.phys_device;
        allocator_create_info.device = device;

        NOVA_THROW_IF_VK_ERROR(vmaCreateAllocator(&allocator_create_info, &vma_allocator), render_engine_initialization_exception);
    }

    void vulkan_render_engine::create_global_sync_objects() {
        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        frame_fences.resize(max_frames_in_queue);
        image_available_semaphores.resize(max_frames_in_queue);
        render_finished_semaphores_by_frame.resize(max_frames_in_queue);

        for(uint32_t i = 0; i < max_frames_in_queue; i++) {
            NOVA_THROW_IF_VK_ERROR(vkCreateFence(device, &fence_info, nullptr, &frame_fences[i]), render_engine_initialization_exception);
            NOVA_THROW_IF_VK_ERROR(vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]), render_engine_initialization_exception);
        }
        NOVA_THROW_IF_VK_ERROR(vkCreateFence(device, &fence_info, nullptr, &mesh_rendering_done), render_engine_initialization_exception);
        NOVA_THROW_IF_VK_ERROR(vkCreateFence(device, &fence_info, nullptr, &upload_to_megamesh_buffer_done), render_engine_initialization_exception);
    }

    void vulkan_render_engine::set_shaderpack(const shaderpack_data& data) {
        NOVA_LOG(DEBUG) << "Vulkan render engine loading new shaderpack";
        if(shaderpack_loaded) {
            destroy_render_passes();
            destroy_graphics_pipelines();
            materials.clear();
            material_passes_by_pipeline.clear();
            destroy_dynamic_resources();

            NOVA_LOG(DEBUG) << "Resources from old shaderpacks destroyed";
        }

        create_textures(data.resources.textures);
        NOVA_LOG(DEBUG) << "Dynamic textures created";
        for(const material_data& mat_data : data.materials) {
            materials[mat_data.name] = mat_data;

            for(const material_pass& mat : mat_data.passes) {
                material_passes_by_pipeline[mat.pipeline].push_back(mat);
            }
        }
        NOVA_LOG(DEBUG) << "Materials saved";

        create_render_passes(data.passes);
        NOVA_LOG(DEBUG) << "Created render passes";
        create_graphics_pipelines(data.pipelines);
        NOVA_LOG(DEBUG) << "Created pipelines";

        create_material_descriptor_sets();
        NOVA_LOG(TRACE) << "Material descriptor sets created";

        generate_barriers_for_dynamic_resources();

        shaderpack_loaded = true;
    }

    VkCommandPool vulkan_render_engine::get_command_buffer_pool_for_current_thread(uint32_t queue_index) {
        const uint32_t cur_thread_idx = scheduler->get_current_thread_idx();
        return command_pools_by_thread_idx.at(cur_thread_idx).at(queue_index);
    }

    VkDescriptorPool vulkan_render_engine::get_descriptor_pool_for_current_thread() {
        const uint32_t cur_thread_idx = scheduler->get_current_thread_idx();
        return descriptor_pools_by_thread_idx.at(cur_thread_idx);
    }

    void vulkan_render_engine::create_default_samplers() {
        VkSamplerCreateInfo point_sampler_create = {};
        point_sampler_create.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        point_sampler_create.magFilter = VK_FILTER_NEAREST;
        point_sampler_create.minFilter = VK_FILTER_NEAREST;

        NOVA_THROW_IF_VK_ERROR(vkCreateSampler(device, &point_sampler_create, nullptr, &point_sampler), render_engine_initialization_exception);
    }

    vk_buffer vulkan_render_engine::get_or_allocate_mesh_staging_buffer(const uint32_t needed_size) {
        std::lock_guard l(mesh_staging_buffers_mutex);

        if(!available_mesh_staging_buffers.empty()) {
            // Try to find a buffer that's big enough
            uint32_t potential_staging_buffer_idx = std::numeric_limits<uint32_t>::max();

            for(uint32_t i = 0; i < available_mesh_staging_buffers.size(); i++) {
                if(available_mesh_staging_buffers[i].alloc_info.size >= needed_size && available_mesh_staging_buffers[i].alloc_info.size > available_mesh_staging_buffers[potential_staging_buffer_idx].alloc_info.size) {
                    potential_staging_buffer_idx = i;
                }
            }

            if(potential_staging_buffer_idx < available_mesh_staging_buffers.size()) {
                const vk_buffer staging_buffer = available_mesh_staging_buffers[potential_staging_buffer_idx];
                available_mesh_staging_buffers.erase(available_mesh_staging_buffers.begin() + potential_staging_buffer_idx);
                return staging_buffer;
            }
        }

        vk_buffer new_buffer = {};

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_create_info.size = needed_size;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &copy_family_index;

        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator, &buffer_create_info, &allocation_create_info, &new_buffer.buffer, &new_buffer.allocation, &new_buffer.alloc_info), render_engine_rendering_exception);

        return new_buffer;
    }

    void vulkan_render_engine::free_mesh_staging_buffer(const vk_buffer& buffer) {
        std::lock_guard l(mesh_staging_buffers_mutex);

        available_mesh_staging_buffers.push_back(buffer);
    }

    std::future<uint32_t> vulkan_render_engine::add_mesh(const mesh_data& input_mesh) {
        return scheduler->add_task(
            [&](ttl::task_scheduler*, const mesh_data* input_mesh, std::mutex* mesh_upload_queue_mutex, std::queue<mesh_staging_buffer_upload_command>* mesh_upload_queue) {
                const uint32_t vertex_size = input_mesh->vertex_data.size() * sizeof(full_vertex);
                const uint32_t index_size = input_mesh->indices.size() * sizeof(uint32_t);

                // TODO: Make the extra memory allocation configurable
                const uint32_t total_memory_needed = std::round((vertex_size + index_size) * 1.1); // Extra size so chunks can grow

                vk_buffer staging_buffer = get_or_allocate_mesh_staging_buffer(total_memory_needed);
                std::memcpy(staging_buffer.alloc_info.pMappedData, &input_mesh->vertex_data[0], vertex_size);
                std::memcpy(reinterpret_cast<uint8_t*>(staging_buffer.alloc_info.pMappedData) + vertex_size, &input_mesh->indices[0], index_size);

                const uint32_t mesh_id = next_mesh_id.fetch_add(1);

                std::lock_guard l(*mesh_upload_queue_mutex);
                mesh_upload_queue->push(mesh_staging_buffer_upload_command{staging_buffer, mesh_id, vertex_size, vertex_size + index_size});

                return mesh_id;
            },
            &input_mesh,
            &mesh_upload_queue_mutex,
            &mesh_upload_queue);
    }

    void vulkan_render_engine::delete_mesh(uint32_t mesh_id) {
        const vk_mesh mesh = meshes.at(mesh_id);
        meshes.erase(mesh_id);

        mesh_memory->free(mesh.memory);
    }

    bool vk_resource_binding::operator==(const vk_resource_binding& other) const {
        return other.set == set && other.binding == binding && other.descriptorCount == descriptorCount && other.descriptorType == descriptorType;
    }

    bool vk_resource_binding::operator!=(const vk_resource_binding& other) const {
        return !(*this == other);
    }

    void vulkan_render_engine::create_render_passes(const std::vector<render_pass_data>& passes) {
        NOVA_LOG(DEBUG) << "Flattening frame graph...";

        std::unordered_map<std::string, render_pass_data> regular_render_passes;
        regular_render_passes.reserve(passes.size());
        render_passes.reserve(passes.size());
        for(const render_pass_data& pass_data : passes) {
            render_passes[pass_data.name].data = pass_data;
            VkFenceCreateInfo fence_info = {};
            fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

            vkCreateFence(device, &fence_info, nullptr, &render_passes[pass_data.name].fence);
            regular_render_passes[pass_data.name] = pass_data;
        }

        render_passes_by_order = order_passes(regular_render_passes);

        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();

        for(const auto& [pass_name, pass] : render_passes) {
            bool is_forward = pass_name == "Forward";
            VkSubpassDescription subpass_description;
            subpass_description.flags = 0;
            subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass_description.inputAttachmentCount = 0;
            subpass_description.pInputAttachments = nullptr;
            subpass_description.preserveAttachmentCount = 0;
            subpass_description.pPreserveAttachments = nullptr;
            subpass_description.pResolveAttachments = nullptr;
            subpass_description.pDepthStencilAttachment = nullptr;

            VkSubpassDependency image_available_dependency;
            image_available_dependency.dependencyFlags = 0;
            image_available_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            image_available_dependency.dstSubpass = 0;
            image_available_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            image_available_dependency.srcAccessMask = 0;
            image_available_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            image_available_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo render_pass_create_info;
            render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            render_pass_create_info.pNext = nullptr;
            render_pass_create_info.flags = 0;
            render_pass_create_info.subpassCount = 1;
            render_pass_create_info.pSubpasses = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies = &image_available_dependency;

            std::vector<VkAttachmentReference> attachment_references;
            std::vector<VkAttachmentDescription> attachments;
            std::vector<VkImageView> framebuffer_attachments;
            uint32_t framebuffer_width = 0;
            uint32_t framebuffer_height = 0;

            std::vector<vk_texture*> textures_in_framebuffer;
            textures_in_framebuffer.reserve(pass.data.texture_outputs.size() + (pass.data.depth_texture ? 1 : 0));
            bool writes_to_backbuffer = false;
            // Collect framebuffer size information from color output attachments
            for(const texture_attachment& attachment : pass.data.texture_outputs) {
                if(attachment.name == "Backbuffer") {
                    // Handle backbuffer
                    // Backbuffer framebuffers are handled by themselves in their own special snowflake way so we just need to skip everything
                    writes_to_backbuffer = true;

                    VkAttachmentDescription desc = {};
                    desc.flags = 0;
                    desc.format = swapchain->get_swapchain_format();
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
                    desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                    desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

                    attachments.push_back(desc);

                    VkAttachmentReference ref = {};
                    ref.layout = VK_IMAGE_LAYOUT_GENERAL;
                    ref.attachment = attachments.size() - 1;
                    attachment_references.push_back(ref);

                    framebuffer_width = swapchain_extent.width;
                    framebuffer_height = swapchain_extent.height;

                    render_passes[pass_name].writes_to_backbuffer = true;

                    break;

                } else {
                    const vk_texture& tex = textures.at(attachment.name);
                    // the textures array _should_ _never_ change from this point onward. If it does, this pointer will probably point to undefined data
                    textures_in_framebuffer.push_back(&textures.at(attachment.name));

                    framebuffer_attachments.push_back(tex.image_view);

                    const glm::uvec2 attachment_size = tex.data.format.get_size_in_pixels({swapchain_extent.width, swapchain_extent.height});

                    if(framebuffer_width == 0) {
                        framebuffer_width = attachment_size.x;

                    } else if(attachment_size.x != framebuffer_width) {
                        NOVA_LOG(ERROR) << "Texture " << attachment.name << " used by renderpass " << pass_name << " has a width of " << attachment_size.x << ", but the framebuffer has a width of " << framebuffer_width
                                        << ". This is illegal, all input textures of a single renderpass must be the same size";
                    }

                    if(framebuffer_height == 0) {
                        framebuffer_height = attachment_size.y;

                    } else if(attachment_size.y != framebuffer_height) {
                        NOVA_LOG(ERROR) << "Texture " << attachment.name << " used by renderpass " << pass_name << " has a height of " << attachment_size.y << ", but the framebuffer has a height of " << framebuffer_height
                                        << ". This is illegal, all input textures of a single renderpass must be the same size";
                    }

                    NOVA_LOG(TRACE) << "Adding image view " << textures.at(attachment.name).image_view << " from image " << attachment.name << " to framebuffer for renderpass " << pass.data.name;

                    VkAttachmentDescription desc = {};
                    desc.flags = 0;
                    desc.format = tex.format;
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
                    desc.loadOp = attachment.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                    desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

                    attachments.push_back(desc);

                    VkAttachmentReference ref = {};
                    ref.layout = VK_IMAGE_LAYOUT_GENERAL;
                    ref.attachment = attachments.size() - 1;
                    attachment_references.push_back(ref);
                }
            }

            VkAttachmentReference depth_reference = {};
            // Collect framebuffer size information from the depth attachment
            if(pass.data.depth_texture) {

                const vk_texture& tex = textures.at(pass.data.depth_texture->name);
                framebuffer_attachments.push_back(tex.image_view);

                textures_in_framebuffer.push_back(&textures.at(pass.data.depth_texture->name));

                const glm::uvec2 attachment_size = tex.data.format.get_size_in_pixels({swapchain_extent.width, swapchain_extent.height});

                if(framebuffer_width == 0) {
                    framebuffer_width = attachment_size.x;

                } else if(attachment_size.x != framebuffer_width) {
                    NOVA_LOG(ERROR) << "Texture " << pass.data.depth_texture->name << " used by renderpass " << pass_name << " has a width of " << attachment_size.x << ", but the framebuffer has a width of " << framebuffer_width
                                    << ". This is illegal, all input textures of a single renderpass must be the same size";
                }

                if(framebuffer_height == 0) {
                    framebuffer_height = attachment_size.y;

                } else if(attachment_size.y != framebuffer_height) {
                    NOVA_LOG(ERROR) << "Texture " << pass.data.depth_texture->name << " used by renderpass " << pass_name << " has a height of " << attachment_size.y << ", but the framebuffer has a height of " << framebuffer_height
                                    << ". This is illegal, all input textures of a single renderpass must be the same size";
                }

                VkAttachmentDescription desc = {};
                desc.flags = 0;
                desc.format = tex.format;
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = pass.data.depth_texture->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

                attachments.push_back(desc);

                depth_reference.layout = VK_IMAGE_LAYOUT_GENERAL;
                depth_reference.attachment = attachments.size() - 1;
                subpass_description.pDepthStencilAttachment = &depth_reference;
            }

            if(framebuffer_width == 0) {
                NOVA_LOG(ERROR) << "Framebuffer width for pass " << pass.data.name
                                << " is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero width";
            }

            if(framebuffer_height == 0) {
                NOVA_LOG(ERROR) << "Framebuffer height for pass " << pass.data.name
                                << " is 0. This is illegal! Make sure that there is at least one attachment for this render pass, and ensure that all attachments used by this pass have a non-zero height";
            }

            if(framebuffer_attachments.size() > gpu.props.limits.maxColorAttachments) {
                NOVA_LOG(ERROR) << "Framebuffer for pass " << pass.data.name << " has " << framebuffer_attachments.size() << " color attachments, but your GPU only supports " << gpu.props.limits.maxColorAttachments
                                << ". Please reduce the number of attachments that this pass uses, possibly by changing some of your input attachments to bound textures";
            }

            subpass_description.colorAttachmentCount = attachment_references.size();
            subpass_description.pColorAttachments = attachment_references.data();

            render_pass_create_info.attachmentCount = attachments.size();
            render_pass_create_info.pAttachments = attachments.data();

            NOVA_THROW_IF_VK_ERROR(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_passes[pass_name].pass), render_engine_initialization_exception);

            if(writes_to_backbuffer) {
                if(pass.data.texture_outputs.size() > 1) {
                    NOVA_LOG(ERROR) << "Pass " << pass.data.name << " writes to the backbuffer, and other textures. Passes that write to the backbuffer are not allowed to write to any other textures";
                }

                render_passes[pass_name].framebuffer.framebuffer = nullptr;

            } else {
                VkFramebufferCreateInfo framebuffer_create_info = {};
                framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebuffer_create_info.renderPass = render_passes[pass_name].pass;
                framebuffer_create_info.attachmentCount = static_cast<uint32_t>(framebuffer_attachments.size());
                framebuffer_create_info.pAttachments = framebuffer_attachments.data();
                framebuffer_create_info.width = framebuffer_width;
                framebuffer_create_info.height = framebuffer_height;
                framebuffer_create_info.layers = 1;

                std::stringstream ss;
                for(const VkImageView& attachment : framebuffer_attachments) {
                    ss << attachment << ", ";
                }

                NOVA_LOG(TRACE) << "Creating framebuffer with size (" << framebuffer_width << ", " << framebuffer_height << "), and attachments " << ss.str();

                VkFramebuffer framebuffer;
                NOVA_THROW_IF_VK_ERROR(vkCreateFramebuffer(device, &framebuffer_create_info, nullptr, &framebuffer), render_engine_initialization_exception);
                render_passes[pass_name].framebuffer.framebuffer = framebuffer;
                render_passes[pass_name].framebuffer.images = std::move(textures_in_framebuffer);
            }

            render_passes[pass_name].render_area = {{0, 0}, {framebuffer_width, framebuffer_height}};

            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(render_passes[pass_name].pass);
            object_name.pObjectName = pass_name.c_str();
            NOVA_THROW_IF_VK_ERROR(vkSetDebugUtilsObjectNameEXT(device, &object_name), render_engine_initialization_exception);
        }
    }

    void vulkan_render_engine::create_graphics_pipelines(const std::vector<pipeline_data>& pipelines) {
        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();

        for(const pipeline_data& data : pipelines) {
            NOVA_LOG(TRACE) << "Creating a VkPipeline for pipeline " << data.name;
            vk_pipeline nova_pipeline;
            nova_pipeline.data = data;

            std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
            std::unordered_map<VkShaderStageFlags, VkShaderModule> shader_modules;

            NOVA_LOG(TRACE) << "Compiling vertex module";
            shader_modules[VK_SHADER_STAGE_VERTEX_BIT] = create_shader_module(data.vertex_shader.source);
            get_shader_module_descriptors(data.vertex_shader.source, nova_pipeline.bindings);

            if(data.geometry_shader) {
                NOVA_LOG(TRACE) << "Compiling geometry module";
                shader_modules[VK_SHADER_STAGE_GEOMETRY_BIT] = create_shader_module(data.geometry_shader->source);
                get_shader_module_descriptors(data.geometry_shader->source, nova_pipeline.bindings);
            }

            if(data.tessellation_control_shader) {
                NOVA_LOG(TRACE) << "Compiling tessellation_control module";
                shader_modules[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = create_shader_module(data.tessellation_control_shader->source);
                get_shader_module_descriptors(data.tessellation_control_shader->source, nova_pipeline.bindings);
            }

            if(data.tessellation_evaluation_shader) {
                NOVA_LOG(TRACE) << "Compiling tessellation_evaluation module";
                shader_modules[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = create_shader_module(data.tessellation_evaluation_shader->source);
                get_shader_module_descriptors(data.tessellation_evaluation_shader->source, nova_pipeline.bindings);
            }

            if(data.fragment_shader) {
                NOVA_LOG(TRACE) << "Compiling fragment module";
                shader_modules[VK_SHADER_STAGE_FRAGMENT_BIT] = create_shader_module(data.fragment_shader->source);
                get_shader_module_descriptors(data.fragment_shader->source, nova_pipeline.bindings);
            }

            nova_pipeline.layouts = create_descriptor_set_layouts(nova_pipeline.bindings);

            VkPipelineLayoutCreateInfo pipeline_layout_create_info;
            pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipeline_layout_create_info.pNext = nullptr;
            pipeline_layout_create_info.flags = 0;
            pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(nova_pipeline.layouts.size());
            pipeline_layout_create_info.pSetLayouts = nova_pipeline.layouts.data();
            pipeline_layout_create_info.pushConstantRangeCount = 0;
            pipeline_layout_create_info.pPushConstantRanges = nullptr;

            VkPipelineLayout layout;
            NOVA_THROW_IF_VK_ERROR(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &layout), render_engine_initialization_exception);
            nova_pipeline.layout = layout;

            for(const auto& [stage, shader_module] : shader_modules) {
                VkPipelineShaderStageCreateInfo shader_stage_create_info;
                shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_stage_create_info.pNext = nullptr;
                shader_stage_create_info.flags = 0;
                shader_stage_create_info.stage = static_cast<VkShaderStageFlagBits>(stage);
                shader_stage_create_info.module = shader_module;
                shader_stage_create_info.pName = "main";
                shader_stage_create_info.pSpecializationInfo = nullptr;

                shader_stages.push_back(shader_stage_create_info);
            }

            const std::vector<VkVertexInputBindingDescription>& vertex_binding_descriptions = vulkan::get_vertex_input_binding_descriptions();
            const std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions = vulkan::get_vertex_input_attribute_descriptions();

            VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
            vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertex_input_state_create_info.pNext = nullptr;
            vertex_input_state_create_info.flags = 0;
            vertex_input_state_create_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_binding_descriptions.size());
            vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
            vertex_input_state_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
            vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

            VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
            input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly_create_info.pNext = nullptr;
            input_assembly_create_info.flags = 0;
            input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
            switch(data.primitive_mode) {
                case primitive_topology_enum::Triangles:
                    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                    break;
                case primitive_topology_enum::Lines:
                    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                    break;
            }

            VkViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = static_cast<float>(swapchain_extent.width);
            viewport.height = static_cast<float>(swapchain_extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor;
            scissor.offset = {0, 0};
            scissor.extent = swapchain_extent;

            VkPipelineViewportStateCreateInfo viewport_state_create_info;
            viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state_create_info.pNext = nullptr;
            viewport_state_create_info.flags = 0;
            viewport_state_create_info.viewportCount = 1;
            viewport_state_create_info.pViewports = &viewport;
            viewport_state_create_info.scissorCount = 1;
            viewport_state_create_info.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer_create_info;
            rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer_create_info.pNext = nullptr;
            rasterizer_create_info.flags = 0;
            rasterizer_create_info.depthClampEnable = VK_FALSE;
            rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer_create_info.lineWidth = 1.0f;
            rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer_create_info.depthBiasEnable = VK_TRUE;
            rasterizer_create_info.depthClampEnable = VK_FALSE;
            rasterizer_create_info.depthBiasConstantFactor = data.depth_bias;
            rasterizer_create_info.depthBiasClamp = 0.0f;
            rasterizer_create_info.depthBiasSlopeFactor = data.slope_scaled_depth_bias;

            VkPipelineMultisampleStateCreateInfo multisample_create_info;
            multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisample_create_info.pNext = nullptr;
            multisample_create_info.flags = 0;
            multisample_create_info.sampleShadingEnable = VK_FALSE;
            multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisample_create_info.minSampleShading = 1.0f;
            multisample_create_info.pSampleMask = nullptr;
            multisample_create_info.alphaToCoverageEnable = VK_FALSE;
            multisample_create_info.alphaToOneEnable = VK_FALSE;

            VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
            depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil_create_info.depthTestEnable = std::find(data.states.begin(), data.states.end(), state_enum::DisableDepthTest) == data.states.end();
            depth_stencil_create_info.depthWriteEnable = std::find(data.states.begin(), data.states.end(), state_enum::DisableDepthWrite) == data.states.end();
            depth_stencil_create_info.depthCompareOp = vulkan::type_converters::to_compare_op(data.depth_func);
            depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
            depth_stencil_create_info.stencilTestEnable = std::find(data.states.begin(), data.states.end(), state_enum::EnableStencilTest) != data.states.end();
            if(data.front_face) {
                depth_stencil_create_info.front.failOp = vulkan::type_converters::to_stencil_op(data.front_face->fail_op);
                depth_stencil_create_info.front.passOp = vulkan::type_converters::to_stencil_op(data.front_face->pass_op);
                depth_stencil_create_info.front.depthFailOp = vulkan::type_converters::to_stencil_op(data.front_face->depth_fail_op);
                depth_stencil_create_info.front.compareOp = vulkan::type_converters::to_compare_op(data.front_face->compare_op);
                depth_stencil_create_info.front.compareMask = data.front_face->compare_mask;
                depth_stencil_create_info.front.writeMask = data.front_face->write_mask;
            }
            if(data.back_face) {
                depth_stencil_create_info.back.failOp = vulkan::type_converters::to_stencil_op(data.back_face->fail_op);
                depth_stencil_create_info.back.passOp = vulkan::type_converters::to_stencil_op(data.back_face->pass_op);
                depth_stencil_create_info.back.depthFailOp = vulkan::type_converters::to_stencil_op(data.back_face->depth_fail_op);
                depth_stencil_create_info.back.compareOp = vulkan::type_converters::to_compare_op(data.back_face->compare_op);
                depth_stencil_create_info.back.compareMask = data.back_face->compare_mask;
                depth_stencil_create_info.back.writeMask = data.back_face->write_mask;
            }

            VkPipelineColorBlendAttachmentState color_blend_attachment;
            color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            color_blend_attachment.blendEnable = VK_TRUE;
            color_blend_attachment.srcColorBlendFactor = vulkan::type_converters::to_blend_factor(data.source_blend_factor);
            color_blend_attachment.dstColorBlendFactor = vulkan::type_converters::to_blend_factor(data.destination_blend_factor);
            color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
            color_blend_attachment.srcAlphaBlendFactor = vulkan::type_converters::to_blend_factor(data.alpha_src);
            color_blend_attachment.dstAlphaBlendFactor = vulkan::type_converters::to_blend_factor(data.alpha_dst);
            color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

            VkPipelineColorBlendStateCreateInfo color_blend_create_info;
            color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blend_create_info.pNext = nullptr;
            color_blend_create_info.flags = 0;
            color_blend_create_info.logicOpEnable = VK_FALSE;
            color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_create_info.attachmentCount = 1;
            color_blend_create_info.pAttachments = &color_blend_attachment;
            color_blend_create_info.blendConstants[0] = 0.0f;
            color_blend_create_info.blendConstants[1] = 0.0f;
            color_blend_create_info.blendConstants[2] = 0.0f;
            color_blend_create_info.blendConstants[3] = 0.0f;

            VkGraphicsPipelineCreateInfo pipeline_create_info = {};
            pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline_create_info.pNext = nullptr;
            pipeline_create_info.flags = 0;
            pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
            pipeline_create_info.pStages = shader_stages.data();
            pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
            pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
            pipeline_create_info.pViewportState = &viewport_state_create_info;
            pipeline_create_info.pRasterizationState = &rasterizer_create_info;
            pipeline_create_info.pMultisampleState = &multisample_create_info;
            pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
            pipeline_create_info.pColorBlendState = &color_blend_create_info;
            pipeline_create_info.pDynamicState = nullptr;
            pipeline_create_info.layout = layout;
            pipeline_create_info.renderPass = render_passes.at(data.pass).pass;
            pipeline_create_info.subpass = 0;
            pipeline_create_info.basePipelineIndex = -1;

            NOVA_THROW_IF_VK_ERROR(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &nova_pipeline.pipeline), render_engine_initialization_exception);

            pipelines_by_renderpass[data.pass].push_back(nova_pipeline);
        }
    }

    VkShaderModule vulkan_render_engine::create_shader_module(const std::vector<uint32_t>& spirv) const {
        VkShaderModuleCreateInfo shader_module_create_info;
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.flags = 0;
        shader_module_create_info.pCode = spirv.data();
        shader_module_create_info.codeSize = spirv.size() * 4;

        VkShaderModule module;
        NOVA_THROW_IF_VK_ERROR(vkCreateShaderModule(device, &shader_module_create_info, nullptr, &module), render_engine_initialization_exception);

        return module;
    }

    void vulkan_render_engine::upload_new_mesh_parts() {
        if(mesh_upload_queue.empty()) {
            // Early out yay
            return;
        }

        VkCommandBuffer mesh_upload_cmds;

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandBufferCount = 1;
        alloc_info.commandPool = get_command_buffer_pool_for_current_thread(copy_family_index);
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        vkAllocateCommandBuffers(device, &alloc_info, &mesh_upload_cmds);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(mesh_upload_cmds, &begin_info);

        // Ensure that all reads from this buffer have finished. I don't care about writes because the only
        // way two dudes would be writing to the same region of a megamesh at the same time was if there was a
        // horrible problem

        mesh_memory->add_barriers_before_data_upload(mesh_upload_cmds);

        mesh_upload_queue_mutex.lock();
        meshes_mutex.lock();

        std::vector<vk_buffer> freed_buffers;
        freed_buffers.reserve(mesh_upload_queue.size() * 2);
        while(!mesh_upload_queue.empty()) {
            const mesh_staging_buffer_upload_command cmd = mesh_upload_queue.front();
            mesh_upload_queue.pop();

            compacting_block_allocator::allocation_info* mem = mesh_memory->allocate(cmd.staging_buffer.alloc_info.size);

            VkBufferCopy copy = {};
            copy.size = cmd.staging_buffer.alloc_info.size;
            copy.srcOffset = 0;
            copy.dstOffset = mem->offset;
            vkCmdCopyBuffer(mesh_upload_cmds, cmd.staging_buffer.buffer, mem->block->get_buffer(), 1, &copy);

            VkDrawIndexedIndirectCommand mesh_draw_command = {};
            mesh_draw_command.indexCount = (cmd.model_matrix_offset - cmd.indices_offset) / sizeof(uint32_t);
            mesh_draw_command.instanceCount = 1;
            mesh_draw_command.firstIndex = 0;
            mesh_draw_command.vertexOffset = static_cast<uint32_t>(mem->offset);
            mesh_draw_command.firstInstance = 0;

            meshes[cmd.mesh_id] = {mem, cmd.indices_offset, cmd.model_matrix_offset, mesh_draw_command};

            freed_buffers.insert(freed_buffers.end(), cmd.staging_buffer);
        }

        mesh_memory->add_barriers_after_data_upload(mesh_upload_cmds);

        mesh_upload_queue_mutex.unlock();
        meshes_mutex.unlock();

        vkEndCommandBuffer(mesh_upload_cmds);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &mesh_upload_cmds;

        // Be super duper sure that mesh rendering is done
        for(const auto& [pass_name, pass] : render_passes) {
            vkWaitForFences(device, 1, &pass.fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        }
        vkQueueSubmit(copy_queue, 1, &submit_info, upload_to_megamesh_buffer_done);

        vkWaitForFences(device, 1, &upload_to_megamesh_buffer_done, VK_TRUE, std::numeric_limits<uint64_t>::max());

        vkResetFences(device, 1, &upload_to_megamesh_buffer_done);

        // Once the upload is done, return all the staging buffers to the pool
        std::lock_guard l(mesh_staging_buffers_mutex);
        available_mesh_staging_buffers.insert(available_mesh_staging_buffers.end(), freed_buffers.begin(), freed_buffers.end());
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::string type = "General";
        if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            type = "Validation";

        } else if(messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            type = "Performance";
        }

        std::stringstream ss;
        ss << "[" << type << "]";
        if(pCallbackData->queueLabelCount != 0) {
            ss << " Queues: ";
            for(uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
                ss << pCallbackData->pQueueLabels[i].pLabelName;
                if(i != pCallbackData->queueLabelCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(pCallbackData->cmdBufLabelCount != 0) {
            ss << " Command Buffers: ";
            for(uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
                ss << pCallbackData->pCmdBufLabels[i].pLabelName;
                if(i != pCallbackData->cmdBufLabelCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(pCallbackData->objectCount != 0) {
            ss << " Objects: ";
            for(uint32_t i = 0; i < pCallbackData->objectCount; i++) {
                ss << vulkan::to_string(pCallbackData->pObjects[i].objectType);
                if(pCallbackData->pObjects[i].pObjectName != nullptr) {
                    ss << pCallbackData->pObjects[i].pObjectName;
                }
                ss << " (" << pCallbackData->pObjects[i].objectHandle << ") ";
                if(i != pCallbackData->objectCount - 1) {
                    ss << ", ";
                }
            }
        }

        if(pCallbackData->pMessage != nullptr) {
            ss << pCallbackData->pMessage;
        }

        const std::string msg = ss.str();

        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            NOVA_LOG(ERROR) << "[" << type << "] " << msg;

        } else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            // Warnings may hint at unexpected / non-spec API usage
            NOVA_LOG(WARN) << "[" << type << "] " << msg;

        } else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            // Informal messages that may become handy during debugging
            NOVA_LOG(INFO) << "[" << type << "] " << msg;

        } else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            NOVA_LOG(DEBUG) << "[" << type << "] " << msg;

        } else {
            // Catch-all to be super sure
            NOVA_LOG(INFO) << "[" << type << "]" << msg;
        }

#ifdef NOVA_LINUX
        if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            nova_backtrace();
        }
#endif
        return VK_FALSE;
    }

    std::shared_ptr<iwindow> vulkan_render_engine::get_window() const {
        return window;
    }

    void vulkan_render_engine::render_frame() {
        reset_render_finished_semaphores();

        current_semaphore_idx = 0;
        vkWaitForFences(device, 1, &frame_fences.at(current_frame), VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(device, 1, &frame_fences.at(current_frame));

        swapchain->acquire_next_swapchain_image(image_available_semaphores.at(current_frame));

        // Record command buffers
        // We can't upload a new shaderpack in the middle of a frame!
        // Future iterations of this code will likely be more clever, so that "load shaderpack" gets scheduled for the beginning of the frame
        shaderpack_loading_mutex.lock();

        if(dynamic_textures_need_to_transition) {
            transition_dynamic_textures();
        }

        for(const std::string& renderpass_name : render_passes_by_order) {
            execute_renderpass(&renderpass_name);
        }
        shaderpack_loading_mutex.unlock();

        // Records and submits a command buffer that barriers until reading vertex data from the megamesh buffer has
        // finished, uploads new mesh parts, then barriers until transfers to the megamesh vertex buffer are finished
        upload_new_mesh_parts();

        swapchain->present_current_image(render_finished_semaphores_by_frame.at(current_frame));
        current_frame = (current_frame + 1) % max_frames_in_queue;
    }

    std::pair<std::vector<VkAttachmentDescription>, std::vector<VkAttachmentReference>> vulkan_render_engine::to_vk_attachment_info(std::vector<std::string>& attachment_names) {
        std::vector<VkAttachmentDescription> attachment_descriptions;
        attachment_descriptions.reserve(attachment_names.size());

        std::vector<VkAttachmentReference> attachment_references;
        attachment_references.reserve(attachment_names.size());

        for(const std::string& name : attachment_names) {
            const vk_texture& tex = textures.at(name);

            VkAttachmentDescription color_attachment;
            color_attachment.flags = 0;
            color_attachment.format = to_vk_format(tex.data.format.pixel_format);
            color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
            color_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
            attachment_descriptions.push_back(color_attachment);

            VkAttachmentReference color_attachment_reference;
            color_attachment_reference.attachment = static_cast<uint32_t>(attachment_references.size());
            color_attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;
            attachment_references.push_back(color_attachment_reference);
        }

        return {attachment_descriptions, attachment_references};
    }

    void vulkan_render_engine::destroy_render_passes() {
        for(const auto& [pass_name, pass] : render_passes) {
            vkDestroyRenderPass(device, pass.pass, nullptr);
        }

        render_passes.clear();
        render_passes_by_order.clear();
    }

    void vulkan_render_engine::destroy_graphics_pipelines() {
        for(const auto& [renderpass_name, pipelines] : pipelines_by_renderpass) {
            for(const auto& pipeline : pipelines) {
                vkDestroyPipeline(device, pipeline.pipeline, nullptr);
            }
        }

        pipelines_by_renderpass.clear();
    }

    void vulkan_render_engine::destroy_dynamic_resources() {
        for(auto itr = std::begin(textures); itr != std::end(textures);) {
            const vk_texture& tex = itr->second;
            if(tex.is_dynamic) {
                vkDestroyImageView(device, tex.image_view, nullptr);
                vmaDestroyImage(vma_allocator, tex.image, tex.allocation);

                itr = textures.erase(itr);

            } else {
                ++itr;
            }
        }

        for(auto itr = std::begin(buffers); itr != std::end(buffers);) {
            const vk_buffer& buf = itr->second;
            if(buf.is_dynamic) {
                vmaDestroyBuffer(vma_allocator, buf.buffer, buf.allocation);

                itr = buffers.erase(itr);

            } else {
                ++itr;
            }
        }
    }

    void vulkan_render_engine::execute_renderpass(const std::string* renderpass_name) {
        const vk_render_pass& renderpass = render_passes.at(*renderpass_name);
        vkResetFences(device, 1, &renderpass.fence);

        const VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmds;
        NOVA_THROW_IF_VK_ERROR(vkAllocateCommandBuffers(device, &alloc_info, &cmds), buffer_allocate_failed);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        const std::vector<vk_pipeline> pipelines = pipelines_by_renderpass.at(*renderpass_name);

        std::vector<VkCommandBuffer> secondary_command_buffers(pipelines.size());

        ttl::condition_counter pipelines_rendering_counter;
        uint32_t i = 0;
        for(const vk_pipeline& pipe : pipelines) {
            render_pipeline(&pipe, &secondary_command_buffers[i], renderpass);
            i++;
        }

        vkBeginCommandBuffer(cmds, &begin_info);

#pragma region Texture attachment layout transition
        if(!renderpass.read_texture_barriers.empty()) {
            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, renderpass.read_texture_barriers.size(), renderpass.read_texture_barriers.data());
        }

        if(!renderpass.write_texture_barriers.empty()) {
            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, renderpass.write_texture_barriers.size(), renderpass.write_texture_barriers.data());
        }

        if(renderpass.writes_to_backbuffer) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = swapchain->get_current_image();
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // Each swapchain image **will** be rendered to before it is presented
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            vkCmdPipelineBarrier(cmds, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        // TODO: Any barriers for aliased textures if we're at an aliased boundary
        // TODO: Something about the depth buffer
#pragma endregion

        VkClearValue clear_value = {};
        clear_value.color = {0, 0, 0, 0};

        const VkClearValue clear_values[] = {clear_value, clear_value};

        VkRenderPassBeginInfo rp_begin_info = {};
        rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin_info.renderPass = renderpass.pass;
        rp_begin_info.framebuffer = renderpass.framebuffer.framebuffer;
        rp_begin_info.renderArea = renderpass.render_area;
        rp_begin_info.clearValueCount = 2;
        rp_begin_info.pClearValues = clear_values;

        if(rp_begin_info.framebuffer == nullptr) {
            rp_begin_info.framebuffer = swapchain->get_current_framebuffer();
        }

        NOVA_LOG(TRACE) << "Starting renderpass " << *renderpass_name << " with framebuffer " << rp_begin_info.framebuffer;

        vkCmdBeginRenderPass(cmds, &rp_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vkCmdExecuteCommands(cmds, static_cast<uint32_t>(secondary_command_buffers.size()), secondary_command_buffers.data());

        vkCmdEndRenderPass(cmds);

        vkEndCommandBuffer(cmds);

        // If we write to the backbuffer, we need to signal the full-frame fence. If we do not, we can signal the individual renderpass's fence
        if(renderpass.writes_to_backbuffer) {
            submit_to_queue(cmds, graphics_queue, frame_fences.at(current_frame), {image_available_semaphores.at(current_frame)});

        } else {
            submit_to_queue(cmds, graphics_queue, renderpass.fence, {});
        }
    }

    void vulkan_render_engine::render_pipeline(const vk_pipeline* pipeline, VkCommandBuffer* cmds, const vk_render_pass& renderpass) {
        // This function is intended to be run inside a separate fiber than its caller, so it needs to get the
        // command pool for its thread, since command pools need to be externally synchronized
        const VkCommandPool command_pool = get_command_buffer_pool_for_current_thread(graphics_family_index);

        VkCommandBufferAllocateInfo cmds_info = {};
        cmds_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmds_info.commandPool = command_pool;
        cmds_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cmds_info.commandBufferCount = 1;

        vkAllocateCommandBuffers(device, &cmds_info, cmds);

        VkCommandBufferInheritanceInfo cmds_inheritance_info = {};
        cmds_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        cmds_inheritance_info.framebuffer = renderpass.framebuffer.framebuffer;
        cmds_inheritance_info.renderPass = renderpass.pass;

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        begin_info.pInheritanceInfo = &cmds_inheritance_info;

        // Start command buffer, start renderpass, and bind pipeline
        vkBeginCommandBuffer(*cmds, &begin_info);

        vkCmdBindPipeline(*cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        // Record all the draws for a material
        // I'll worry about depth sorting later
        const std::vector<material_pass> materials = material_passes_by_pipeline.at(pipeline->data.name);
        for(const material_pass& pass : materials) {
            bind_material_resources(pass, *pipeline, *cmds);

            draw_all_for_material(pass, *cmds);
        }

        vkEndCommandBuffer(*cmds);
    }

    void vulkan_render_engine::bind_material_resources(const material_pass& mat_pass, const vk_pipeline& pipeline, VkCommandBuffer cmds) {
        vkCmdBindDescriptorSets(cmds, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &mat_pass.descriptor_sets.at(0), 0, nullptr);
    }

    void vulkan_render_engine::draw_all_for_material(const material_pass& pass, VkCommandBuffer cmds) {
        // Version 1: Put indirect draw commands into a buffer right here, send that data to the GPU, and render that
        // Version 2: Let the host application tell us which render objects are visible and which are not, and incorporate that information
        // Version 3: Send data about what is and isn't visible to the GPU and construct the indirect draw commands buffer in a compute shader
        // Version 2: Incorporate occlusion queries so we know what with all certainty what is and isn't visible

        // At the current time I'm making version 1

        // TODO: _Anything_ smarter

        if(renderables_by_material.find(pass.name) == renderables_by_material.end()) {
            // Nothing to render? Don't render it!

            // smh

            return;
        }

        const std::unordered_map<VkBuffer, std::vector<render_object>>& renderables_by_buffer = renderables_by_material.at(pass.name);

        for(const auto& [buffer, renderables] : renderables_by_buffer) {
            VkBufferCreateInfo buffer_create_info = {};
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.size = sizeof(VkDrawIndexedIndirectCommand) * renderables.size();
            buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_create_info.queueFamilyIndexCount = 1;
            buffer_create_info.pQueueFamilyIndices = &graphics_family_index;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

            VkBuffer indirect_draw_commands_buffer;
            VmaAllocation allocation;
            VmaAllocationInfo alloc_info;

            NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(vma_allocator, &buffer_create_info, &alloc_create_info, &indirect_draw_commands_buffer, &allocation, &alloc_info), buffer_allocate_failed);

            // Version 1: write commands for all things to the indirect draw buffer
            VkDrawIndexedIndirectCommand* indirect_commands = reinterpret_cast<VkDrawIndexedIndirectCommand*>(alloc_info.pMappedData);

            for(uint32_t i = 0; i < renderables.size(); i++) {
                const render_object& cur_obj = renderables.at(i);
                indirect_commands[i] = cur_obj.mesh->draw_cmd;
            }

            vkCmdBindVertexBuffers(cmds, 0, 1, &buffer, nullptr);
            vkCmdBindIndexBuffer(cmds, buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexedIndirect(cmds, indirect_draw_commands_buffer, 0, renderables.size(), 0);
        }
    }

    void vulkan_render_engine::submit_to_queue(VkCommandBuffer cmds, VkQueue queue, VkFence cmd_buffer_done_fence, const std::vector<VkSemaphore>& wait_semaphores) {
        std::lock_guard l(render_done_sync_mutex);
        std::vector<VkSemaphore>& render_finished_semaphores = render_finished_semaphores_by_frame.at(current_frame);

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        while(render_finished_semaphores.size() <= current_semaphore_idx) {
            VkSemaphore semaphore;

            NOVA_THROW_IF_VK_ERROR(vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore), render_engine_initialization_exception);

            render_finished_semaphores.push_back(semaphore);
        }

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmds;

        const bool one_null_semaphore = wait_semaphores.size() == 1 && wait_semaphores.at(0) == VK_NULL_HANDLE;
        if(!one_null_semaphore) {
            submit_info.waitSemaphoreCount = wait_semaphores.size();
            submit_info.pWaitSemaphores = wait_semaphores.data();
        }
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_finished_semaphores.at(current_semaphore_idx);
        NOVA_THROW_IF_VK_ERROR(vkQueueSubmit(queue, 1, &submit_info, cmd_buffer_done_fence), render_engine_rendering_exception);

        current_semaphore_idx++;
    }

    VkFormat vulkan_render_engine::to_vk_format(const pixel_format_enum format) {
        switch(format) {
            case pixel_format_enum::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case pixel_format_enum::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case pixel_format_enum::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case pixel_format_enum::Depth:
                return VK_FORMAT_D32_SFLOAT;

            case pixel_format_enum::DepthStencil:
                return VK_FORMAT_D24_UNORM_S8_UINT;
        }

        return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    }

    void vulkan_render_engine::reset_render_finished_semaphores() {
        render_finished_semaphores_by_frame[current_frame].clear();
    }

    void vulkan_render_engine::create_per_thread_command_pools() {
        const uint32_t num_threads = scheduler->get_num_threads();
        command_pools_by_thread_idx.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
            command_pools_by_thread_idx.push_back(make_new_command_pools());
        }
    }

    void vulkan_render_engine::create_per_thread_descriptor_pools() {
        const uint32_t num_threads = scheduler->get_num_threads();
        descriptor_pools_by_thread_idx.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
            descriptor_pools_by_thread_idx.push_back(make_new_descriptor_pool());
        }
    }

    std::unordered_map<uint32_t, VkCommandPool> vulkan_render_engine::make_new_command_pools() const {
        std::vector<uint32_t> queue_indices;
        queue_indices.push_back(graphics_family_index);
        queue_indices.push_back(copy_family_index);
        queue_indices.push_back(compute_family_index);

        std::unordered_map<uint32_t, VkCommandPool> pools_by_queue;
        pools_by_queue.reserve(3);

        for(const uint32_t queue_index : queue_indices) {
            VkCommandPoolCreateInfo command_pool_create_info;
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            command_pool_create_info.queueFamilyIndex = queue_index;

            VkCommandPool command_pool;
            NOVA_THROW_IF_VK_ERROR(vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool), render_engine_initialization_exception);
            pools_by_queue[queue_index] = command_pool;
        }

        return pools_by_queue;
    }

    VkDescriptorPool vulkan_render_engine::make_new_descriptor_pool() const {
        std::vector<VkDescriptorPoolSize> pool_sizes;
        pool_sizes.emplace_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 5}); // Virtual textures greatly reduces the number of total textures
        pool_sizes.emplace_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 5});
        pool_sizes.emplace_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5000});

        VkDescriptorPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.maxSets = 5000;
        pool_create_info.poolSizeCount = pool_sizes.size();
        pool_create_info.pPoolSizes = pool_sizes.data();

        VkDescriptorPool pool;
        NOVA_THROW_IF_VK_ERROR(vkCreateDescriptorPool(device, &pool_create_info, nullptr, &pool), descriptor_pool_creation_failed);

        return pool;
    }

    void vulkan_render_engine::create_textures(const std::vector<texture_resource_data>& texture_datas) {
        const VkExtent2D swapchain_extent = swapchain->get_swapchain_extent();
        const glm::uvec2 swapchain_extent_glm = {swapchain_extent.width, swapchain_extent.height};
        for(const texture_resource_data& texture_data : texture_datas) {
            vk_texture texture;
            texture.is_dynamic = true;
            texture.data = texture_data;
            texture.format = to_vk_format(texture_data.format.pixel_format);
            texture.layout = VK_IMAGE_LAYOUT_UNDEFINED;

            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = texture.format;
            const glm::uvec2 texture_size = texture_data.format.get_size_in_pixels(swapchain_extent_glm);
            image_create_info.extent.width = texture_size.x;
            image_create_info.extent.height = texture_size.y;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            if(texture.format == VK_FORMAT_D24_UNORM_S8_UINT || texture.format == VK_FORMAT_D32_SFLOAT) {
                image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            } else {
                image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            image_create_info.queueFamilyIndexCount = 1;
            image_create_info.pQueueFamilyIndices = &graphics_family_index;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            alloc_create_info.preferredFlags = 0;
            alloc_create_info.memoryTypeBits = 0;
            alloc_create_info.pool = VK_NULL_HANDLE;
            alloc_create_info.pUserData = nullptr;

            vmaCreateImage(vma_allocator, &image_create_info, &alloc_create_info, &texture.image, &texture.allocation, &texture.vma_info);

            VkImageViewCreateInfo image_view_create_info = {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = texture.image;
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = image_create_info.format;
            if(texture.format == VK_FORMAT_D24_UNORM_S8_UINT || texture.format == VK_FORMAT_D32_SFLOAT) {
                image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                texture.is_depth_tex = true;

            } else {
                image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;

            vkCreateImageView(device, &image_view_create_info, nullptr, &texture.image_view);

            VkDebugUtilsObjectNameInfoEXT object_name = {};
            object_name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            object_name.objectType = VK_OBJECT_TYPE_IMAGE;
            object_name.objectHandle = reinterpret_cast<uint64_t>(texture.image);
            object_name.pObjectName = texture_data.name.c_str();
            NOVA_THROW_IF_VK_ERROR(vkSetDebugUtilsObjectNameEXT(device, &object_name), render_engine_initialization_exception);
            NOVA_LOG(INFO) << "Set object " << texture.image << " to have name " << texture_data.name;

            textures[texture_data.name] = texture;
        }

        dynamic_textures_need_to_transition = true;
    }

    void vulkan_render_engine::add_resource_to_bindings(std::unordered_map<std::string, vk_resource_binding>& bindings, const spirv_cross::CompilerGLSL& shader_compiler, const spirv_cross::Resource& resource, const VkDescriptorType type) {
        const uint32_t set = shader_compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const uint32_t binding = shader_compiler.get_decoration(resource.id, spv::DecorationBinding);

        vk_resource_binding new_binding = {};
        new_binding.set = set;
        new_binding.binding = binding;
        new_binding.descriptorType = type;
        new_binding.descriptorCount = 1;

        if(bindings.find(resource.name) == bindings.end()) {
            // Totally new binding!
            bindings[resource.name] = new_binding;

        } else {
            // Existing binding. Is it the same as our binding?
            const vk_resource_binding& existing_binding = bindings.at(resource.name);
            if(existing_binding != new_binding) {
                // They have two different bindings with the same name. Not allowed
                NOVA_LOG(ERROR) << "You have two different uniforms named " << resource.name << " in different shader stages. This is not allowed. Use unique names";
            }
        }
    }

    void vulkan_render_engine::get_shader_module_descriptors(const std::vector<uint32_t>& spirv, std::unordered_map<std::string, vk_resource_binding>& bindings) {
        const spirv_cross::CompilerGLSL shader_compiler(spirv);
        const spirv_cross::ShaderResources resources = shader_compiler.get_shader_resources();

        for(const spirv_cross::Resource& resource : resources.sampled_images) {
            add_resource_to_bindings(bindings, shader_compiler, resource, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }

        for(const spirv_cross::Resource& resource : resources.uniform_buffers) {
            add_resource_to_bindings(bindings, shader_compiler, resource, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        }
    }

    std::vector<VkDescriptorSetLayout> vulkan_render_engine::create_descriptor_set_layouts(std::unordered_map<std::string, vk_resource_binding> all_bindings) const {
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> bindings_by_set;

        for(const auto& named_binding : all_bindings) {
            const vk_resource_binding& binding = named_binding.second;
            VkDescriptorSetLayoutBinding new_binding = {};
            new_binding.binding = binding.binding;
            new_binding.descriptorCount = binding.descriptorCount;
            new_binding.descriptorType = binding.descriptorType;
            new_binding.pImmutableSamplers = binding.pImmutableSamplers;
            new_binding.stageFlags = VK_SHADER_STAGE_ALL;

            bindings_by_set[binding.set].push_back(new_binding);
        }

        std::vector<VkDescriptorSetLayoutCreateInfo> dsl_create_infos = {};
        dsl_create_infos.reserve(bindings_by_set.size());
        for(uint32_t i = 0; i < bindings_by_set.size(); i++) {
            if(bindings_by_set.find(i) == bindings_by_set.end()) {
                NOVA_LOG(ERROR) << "Could not get information for descriptor set " << i << "; most likely you skipped"
                                << " a descriptor set in your shader. Ensure that all shaders for this pipeline together don't have"
                                << " any gaps in the descriptor sets they declare";
                throw shader_layout_creation_failed("Descriptor set " + std::to_string(i) + " not present");
            }

            const std::vector<VkDescriptorSetLayoutBinding>& bindings = bindings_by_set.at(i);

            VkDescriptorSetLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = bindings.size();
            create_info.pBindings = bindings.data();

            dsl_create_infos.push_back(create_info);
        }

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.resize(dsl_create_infos.size());
        for(uint32_t i = 0; i < dsl_create_infos.size(); i++) {
            vkCreateDescriptorSetLayout(device, &dsl_create_infos[i], nullptr, &layouts[i]);
        }

        return layouts;
    }

    void vulkan_render_engine::create_material_descriptor_sets() {
        for(const auto& [renderpass_name, pipelines] : pipelines_by_renderpass) {
            for(const auto& pipeline : pipelines) {
                std::vector<material_pass>& material_passes = material_passes_by_pipeline.at(pipeline.data.name);
                for(material_pass& mat_pass : material_passes) {
                    if(pipeline.layouts.empty()) {
                        // If there's no layouts, we're done
                        NOVA_LOG(TRACE) << "No layouts for pipeline " << pipeline.data.name << ", which material pass " << mat_pass.name << " of material " << mat_pass.material_name << " uses";
                        continue;
                    }

                    NOVA_LOG(TRACE) << "Creating descriptor sets for pipeline " << pipeline.data.name;

                    auto layouts = std::vector<VkDescriptorSetLayout>{};
                    layouts.reserve(pipeline.layouts.size());

                    // CLion might tell you to simplify this into a foreach loop... DO NOT! The layouts need to be added in set
                    // order, not map order which is what you'll get if you use a foreach - AND IT'S WRONG
                    for(uint32_t i = 0; i < pipeline.layouts.size(); i++) {
                        layouts.push_back(pipeline.layouts.at(static_cast<int32_t>(i)));
                    }

                    VkDescriptorSetAllocateInfo alloc_info = {};
                    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                    alloc_info.descriptorPool = get_descriptor_pool_for_current_thread();
                    alloc_info.descriptorSetCount = layouts.size();
                    alloc_info.pSetLayouts = layouts.data();

                    mat_pass.descriptor_sets.resize(layouts.size());
                    NOVA_THROW_IF_VK_ERROR(vkAllocateDescriptorSets(device, &alloc_info, mat_pass.descriptor_sets.data()), shaderpack_loading_error);

                    update_material_descriptor_sets(mat_pass, pipeline.bindings);
                }
            }
        }
    }

    std::vector<VkImageMemoryBarrier> vulkan_render_engine::make_attachment_to_shader_read_only_barriers(const std::unordered_set<std::string>& textures) {
        std::vector<VkImageMemoryBarrier> barriers;
        barriers.reserve(textures.size());

        for(const std::string& tex_name : textures) {
            const vk_texture& tex = this->textures.at(tex_name);

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = tex.image;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            if(tex.is_depth_tex) {
                barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            } else {
                barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            barriers.push_back(barrier);
        }

        return barriers;
    }

    enum class barrier_necessity { maybe, yes, no };

    void vulkan_render_engine::create_barriers_for_renderpass(vk_render_pass& pass) {
        /*
         * For each renderpass:
         * - Walk backwards through previous renderpasses
         * - If we find one that writes to a resource we read from before we find one that reads from a resource we read from, add a barrier
         * - If we find one that reads from a resource we write to before we find one that writes to a resource we write to, add a barrier
         */

        std::unordered_map<std::string, barrier_necessity> read_texture_barrier_necessity;
        read_texture_barrier_necessity.reserve(pass.data.texture_inputs.size());
        for(const std::string& tex_name : pass.data.texture_inputs) {
            read_texture_barrier_necessity[tex_name] = barrier_necessity::maybe;
        }

        std::unordered_map<std::string, barrier_necessity> write_texture_barrier_necessity;
        write_texture_barrier_necessity.reserve(pass.data.texture_outputs.size());
        for(const texture_attachment& attach : pass.data.texture_outputs) {
            write_texture_barrier_necessity[attach.name] = barrier_necessity::maybe;
        }

        barrier_necessity depth_buffer_barrier_necessity = barrier_necessity::maybe;

        // Find where we are in the list of passes
        auto itr = render_passes_by_order.rbegin();
        while(itr != render_passes_by_order.rend()) {
            if(*itr == pass.data.name) {
                break;
            }

            ++itr;
        }

        ++itr;

        // Walk backwards from where we are, checking if any of the textures the previous pass writes to are textures we need to barrier
        while(itr != render_passes_by_order.rend()) {
            const vk_render_pass& previous_pass = render_passes.at(*itr);
            // If the previous pass reads from a texture that we read from, it (or a even earlier pass) will have the barrier
            const std::vector<std::string> read_textures = previous_pass.data.texture_inputs;

            for(const std::string& prev_read_texture : read_textures) {
                // If the previous pass reads from a texture that we read from, we don't need to barrier it
                if(read_texture_barrier_necessity.find(prev_read_texture) != read_texture_barrier_necessity.end()) {
                    if(read_texture_barrier_necessity.at(prev_read_texture) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Do not need a barrier for read texture " << prev_read_texture << " before renderpass " << pass.data.name << " because pass " << previous_pass.data.name << " reads from it as well";
                        read_texture_barrier_necessity[prev_read_texture] = barrier_necessity::no;
                    }
                }

                // If the previous pass reads from a texture that we write to, we need a barrier
                if(write_texture_barrier_necessity.find(prev_read_texture) != write_texture_barrier_necessity.end()) {
                    if(write_texture_barrier_necessity.at(prev_read_texture) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Need a barrier for write texture " << prev_read_texture << " before renderpass " << pass.data.name << " because pass " << previous_pass.data.name << " reads from it";
                        write_texture_barrier_necessity[prev_read_texture] = barrier_necessity::no;
                    }
                }
            }

            for(const texture_attachment& prev_write_tex : previous_pass.data.texture_outputs) {
                // If the previous pass write to a texture that we read from, we need a barrier
                if(read_texture_barrier_necessity.find(prev_write_tex.name) != read_texture_barrier_necessity.end()) {
                    if(read_texture_barrier_necessity.at(prev_write_tex.name) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Need a barrier for read texture " << prev_write_tex.name << " before renderpass " << pass.data.name << " because pass " << previous_pass.data.name << " writes to is";
                        read_texture_barrier_necessity[prev_write_tex.name] = barrier_necessity::yes;
                    }
                }

                // If the previous pass write to a texture that we write to, we don't need to barrier it
                if(write_texture_barrier_necessity.find(prev_write_tex.name) != write_texture_barrier_necessity.end()) {
                    if(write_texture_barrier_necessity.at(prev_write_tex.name) == barrier_necessity::maybe) {
                        NOVA_LOG(TRACE) << "Do not need a barrier for write texture " << prev_write_tex.name << " before renderpass " << pass.data.name << " because pass " << previous_pass.data.name << " writes to it as well";
                        write_texture_barrier_necessity[prev_write_tex.name] = barrier_necessity::yes;
                    }
                }
            }

            ++itr;
        }

        for(const auto& [tex_name, necessity] : read_texture_barrier_necessity) {
            const vk_texture& tex = textures.at(tex_name);

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = tex.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            pass.read_texture_barriers.push_back(barrier);
        }

        for(const auto& [tex_name, necessity] : write_texture_barrier_necessity) {
            if(tex_name == "Backbuffer") {
                continue;
            }

            const VkImage image = textures.at(tex_name).image;

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.srcQueueFamilyIndex = graphics_family_index;
            barrier.dstQueueFamilyIndex = graphics_family_index;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            pass.write_texture_barriers.push_back(barrier);
        }
    }

    void vulkan_render_engine::generate_barriers_for_dynamic_resources() {
        for(auto& [name, pass] : render_passes) {
            create_barriers_for_renderpass(pass);
        }
    }

    void vulkan_render_engine::update_material_descriptor_sets(const material_pass& mat, const std::unordered_map<std::string, vk_resource_binding>& name_to_descriptor) {
        // for each resource:
        //  - Get its set and binding from the pipeline
        //  - Update its descriptor set
        NOVA_LOG(TRACE) << "Updating descriptors for material " << mat.material_name;

        std::vector<VkWriteDescriptorSet> writes;

        // We create VkDescriptorImageInfo objects in a different scope, so were they to live there forever they'd get destructed before we can use them
        // Instead we have them in a std::vector so they get deallocated _after_ being used
        std::vector<VkDescriptorImageInfo> image_infos(mat.bindings.size());

        std::vector<VkDescriptorBufferInfo> buffer_infos(mat.bindings.size());

        for(const auto& [renderpass_name, pipelines] : pipelines_by_renderpass) {
            bool should_break = false;

            for(const vk_pipeline& pipeline : pipelines) {
                if(pipeline.data.name == mat.pipeline) {
                    for(const auto& [descriptor_name, resource_name] : mat.bindings) {
                        if(pipeline.bindings.find(descriptor_name) == pipeline.bindings.end()) {
                            NOVA_LOG(DEBUG) << "Material pass " << mat.name << " in material " << mat.material_name << " wants to bind " << resource_name << " to descriptor set " << descriptor_name << ", but it doesn't exist in pipeline "
                                            << pipeline.data.name << ", which this material pass uses";
                        }
                    }

                    should_break = true;
                    break;
                }

                if(should_break) {
                    break;
                }
            }
        }

        for(const auto& [descriptor_name, resource_name] : mat.bindings) {
            const auto& descriptor_info = name_to_descriptor.at(descriptor_name);
            const auto descriptor_set = mat.descriptor_sets[descriptor_info.set];
            bool is_known = true;

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptor_set;
            write.dstBinding = descriptor_info.binding;
            write.descriptorCount = 1;
            write.dstArrayElement = 0;

            if(textures.find(resource_name) != textures.end()) {
                const vk_texture& texture = textures.at(resource_name);
                write_texture_to_descriptor(texture, write, image_infos);

            } else if(buffers.find(resource_name) != buffers.end()) {
                const vk_buffer& buffer = buffers.at(resource_name);
                write_buffer_to_descriptor(buffer, write, buffer_infos);

            } else {
                is_known = false;
                NOVA_LOG(WARN) << "Resource " << resource_name << " is not known to Nova. I hope you aren't using it cause it doesn't exist";
            }

            if(is_known) {
                writes.push_back(write);
            }
        }

        vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
    }

    void vulkan_render_engine::write_texture_to_descriptor(const vk_texture& texture, VkWriteDescriptorSet& write, std::vector<VkDescriptorImageInfo>& image_infos) const {
        VkDescriptorImageInfo image_info = {};
        image_info.imageView = texture.image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_info.sampler = point_sampler;

        image_infos.push_back(image_info);

        write.pImageInfo = &image_infos.at(image_infos.size() - 1);
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    void vulkan_render_engine::write_buffer_to_descriptor(const vk_buffer& buffer, VkWriteDescriptorSet& write, std::vector<VkDescriptorBufferInfo>& buffer_infos) {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.buffer;
        buffer_info.offset = 0;
        buffer_info.range = buffer.alloc_info.size;

        buffer_infos.push_back(buffer_info);

        write.pBufferInfo = &buffer_infos[buffer_infos.size() - 1];
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
} // namespace nova
