#include "vulkan_render_backend.hpp"

#include <stdexcept>

#include <Tracy.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "VkBootstrap.h"

namespace nova::renderer {
    static auto logger = spdlog::stdout_color_mt("VulkanBackend");

#pragma region Options
    constexpr bool ENABLE_DEBUG_LAYER = true;

    constexpr bool ENABLE_GPU_BASED_VALIDATION = false;

    constexpr bool BREAK_ON_VALIDATION_ERRORS = true;
#pragma endregion

    VulkanBackend::VulkanBackend(HWND window_handle) {
        ZoneScoped;

        auto builder = vkb::InstanceBuilder()
                           .set_app_name("Minecraft")
                           .set_engine_name("Nova")
                           .set_app_version(1, 16, 0)
                           .set_engine_version(0, 10, 0)
                           .require_api_version(1, 2, 0)
                           .enable_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        if(ENABLE_DEBUG_LAYER) {
            builder.request_validation_layers()
                .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT)
                .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
                .set_debug_callback(&VulkanBackend::debug_report_callback);
        }

        if(ENABLE_GPU_BASED_VALIDATION) {
            builder.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
        }

        auto inst_ret = builder.build();
        if(!inst_ret) {
            const auto msg = fmt::format("Failed to create Vulkan instance. Error: {}", inst_ret.error().message());
            throw std::runtime_error(msg);
        }
        auto vkb_inst = inst_ret.value();

        instance = vkb_inst.instance;

        create_surface(window_handle);

        auto physical_features = vk::PhysicalDeviceFeatures{};
        physical_features.fullDrawIndexUint32 = true;
        physical_features.multiDrawIndirect = true;
        #ifndef NDEBUG
        physical_features.robustBufferAccess = true;
        #endif

        auto selector = vkb::PhysicalDeviceSelector{vkb_inst}
                            .set_surface(surface)
                            .set_minimum_version(1, 2)
                            .require_dedicated_transfer_queue();
        auto phys_ret = selector.select();
        if(!phys_ret) {
            const auto msg = fmt::format("Failed to select Vulkan Physical Device. Error: {}", phys_ret.error().message());
            throw std::runtime_error(msg);
        }

        vkb::DeviceBuilder device_builder{phys_ret.value()};
        // automatically propagate needed data from instance & physical device
        auto dev_ret = device_builder.build();
        if(!dev_ret) {
            const auto msg = fmt::format("Failed to create Vulkan device. Error: {}", dev_ret.error().message());
            throw std::runtime_error(msg);
        }
        vkb::Device vkb_device = dev_ret.value();

        // Get the VkDevice handle used in the rest of a vulkan application
        device = vkb_device.device;

        // Get the graphics queue with a helper function
        auto graphics_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
        if(!graphics_queue_ret) {
            const auto msg = fmt::format("Failed to get graphics queue. Error: {}", graphics_queue_ret.error().message());
            throw std::runtime_error(msg);
        }

        VkQueue graphics_queue = graphics_queue_ret.value();        

        create_per_thread_command_pools();

        create_standard_pipeline_layout();

        logger->info("Initialized Vulkan backend");
    }

    VulkanBackend::~VulkanBackend() {}

    void VulkanBackend::begin_frame() {
        frame_idx++;
        if(frame_idx == num_gpu_frames) {
            frame_idx = 0;
        }
    }

    vk::Instance VulkanBackend::get_instance() const { return instance; }

#pragma region Init
    void VulkanBackend::create_surface(HWND window_handle) {
        ZoneScoped;
        VkWin32SurfaceCreateInfoKHR win32_surface_create = {};
        win32_surface_create.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32_surface_create.hwnd = window_handle;

        vkCreateWin32SurfaceKHR(instance, &win32_surface_create, nullptr, &surface);
    }
#pragma endregion

#pragma region Debug
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanBackend::debug_report_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                                        const VkDebugUtilsMessageTypeFlagsEXT message_types,
                                                                        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                        void* render_device) {
        std::string type = "General";
        if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0U) {
            type = "Validation";
        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0U) {
            type = "Performance";
        }

        std::string queue_list;
        if(callback_data->queueLabelCount != 0) {
            queue_list.append(" Queues: ");
            for(uint32_t i = 0; i < callback_data->queueLabelCount; i++) {
                queue_list.append(callback_data->pQueueLabels[i].pLabelName);
                if(i != callback_data->queueLabelCount - 1) {
                    queue_list.append(", ");
                }
            }
        }

        std::string command_buffer_list;
        if(callback_data->cmdBufLabelCount != 0) {
            command_buffer_list.append("Command Buffers: ");
            for(uint32_t i = 0; i < callback_data->cmdBufLabelCount; i++) {
                command_buffer_list.append(callback_data->pCmdBufLabels[i].pLabelName);
                if(i != callback_data->cmdBufLabelCount - 1) {
                    command_buffer_list.append(", ");
                }
            }
        }

        std::string object_list;
        if(callback_data->objectCount != 0) {
            object_list.append("Objects: ");
            for(uint32_t i = 0; i < callback_data->objectCount; i++) {
                object_list.append(vk::to_string(static_cast<vk::ObjectType>(callback_data->pObjects[i].objectType)));
                if(callback_data->pObjects[i].pObjectName != nullptr) {
                    object_list.append(fmt::format(" \"{}\"", callback_data->pObjects[i].pObjectName));
                }
                object_list.append(fmt::format(" ({})", callback_data->pObjects[i].objectHandle));
                if(i != callback_data->objectCount - 1) {
                    object_list.append(", ");
                }
            }
        }

        std::string vk_message;
        if(callback_data->pMessage != nullptr) {
            vk_message.append(callback_data->pMessage);
        }

        const auto msg = fmt::format("[{}] {} {} {} {}", type, queue_list, command_buffer_list, object_list, vk_message);

        if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            logger->error("{}", msg);
#ifdef NOVA_LINUX
            nova_backtrace();
#endif

            if(BREAK_ON_VALIDATION_ERRORS) {
                DebugBreak();
            }

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
            // Warnings may hint at unexpected / non-spec API usage
            logger->warn("{}", msg);

        } else if(((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) &&
                  ((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U)) { // No validation info!
            // Informal messages that may become handy during debugging
            logger->info("{}", msg);

        } else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
            // Diagnostic info from the Vulkan loader and layers
            // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
            logger->debug("{}", msg);

        } else if((message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) == 0U) { // No validation info!
            // Catch-all to be super sure
            logger->info("{}", msg);
        }

        return VK_FALSE;
    }

    void VulkanBackend::enable_debug_output() {
        // Uses the Vulkan C bindings cause idk I don't wanna figure this bs out

        ZoneScoped;
        vk_create_debug_utils_messenger_ext = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        vk_destroy_debug_report_callback_ext = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
        debug_create_info.pNext = nullptr;
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(&debug_report_callback);
        debug_create_info.pUserData = this;

        const auto result = vk_create_debug_utils_messenger_ext(instance, &debug_create_info, nullptr, &debug_callback);
        if(result != VK_SUCCESS) {
            logger->error("Could not register debug callback");
        }
    }
#pragma endregion
} // namespace nova::renderer
