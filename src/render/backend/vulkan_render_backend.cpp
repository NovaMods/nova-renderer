#include "vulkan_render_backend.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace nova::renderer {
    static auto logger = spdlog::stdout_color_mt("VulkanBackend");

    VulkanBackend::VulkanBackend() { logger->info("Initialized Vulkan backend"); }

    void VulkanBackend::begin_frame() {
        frame_idx++;
        if(frame_idx == num_gpu_frames) {
            frame_idx = 0;
        }
    }

    vk::Instance VulkanBackend::get_instance() const { return instance; }
} // namespace nova::renderer
