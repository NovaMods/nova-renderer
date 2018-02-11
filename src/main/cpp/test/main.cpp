#include "../render/nova_renderer.h"

#include <easylogging++.h>
#include "../render/objects/textures/texture2D.h"

bool is_format_supported(vk::PhysicalDevice& physical_device, vk::Format format, vk::FormatFeatureFlags features) {
    vk::FormatProperties props = physical_device.getFormatProperties(format);
    auto masked_features = props.linearTilingFeatures & features;
    return masked_features == features;
}

int main(int argc, char **argv) {
    nova::nova_renderer::init();
    LOG(INFO) << "Inited Nova in the test program";

    nova::nova_renderer::instance->render_frame();

    nova::nova_renderer::deinit();
}
