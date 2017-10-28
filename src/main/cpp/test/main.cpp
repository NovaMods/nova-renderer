#include "../render/nova_renderer.h"

bool is_format_supported(vk::PhysicalDevice& physical_device, vk::Format format, vk::FormatFeatureFlags features) {
    vk::FormatProperties props = physical_device.getFormatProperties(format);
    auto masked_features = props.linearTilingFeatures & features;
    return masked_features == features;
}

int main(int argc, char **argv) {
    nova::nova_renderer::init();

    auto& physical_device = nova::render_context::instance.physical_device;
    if(is_format_supported(physical_device, vk::Format::eR8G8B8A8Unorm, vk::FormatFeatureFlagBits::eTransferDstKHR)) {
        LOG(INFO) << "R8G8B8A8Unorm is supported as a transfer source";
    } else {
        LOG(ERROR) << "R8G8B8A8Unorm is not supported as a transfer source";
    }

    auto test_tex = nova::texture2D{};
    auto size = glm::u32vec2{4096, 2048};
    test_tex.set_data(nullptr, size, vk::Format::eR8G8B8A8Unorm);

    nova::nova_renderer::instance->render_frame();
}
