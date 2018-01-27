#include "../render/nova_renderer.h"

#include <easylogging++.h>
#include "../render/objects/textures/texture2D.h"

bool is_format_supported(vk::PhysicalDevice& physical_device, vk::Format format, vk::FormatFeatureFlags features) {
    vk::FormatProperties props = physical_device.getFormatProperties(format);
    auto masked_features = props.linearTilingFeatures & features;
    return masked_features == features;
}

int main(int argc, char **argv) {
    // Hackyboye - allocated a bunch of memory, then free it, so we know there's space
    nova::nova_renderer::init();

    /*
    auto test_tex = nova::texture2D{};
    auto size = vk::Extent2D{4096, 2048};
    test_tex.set_data(nullptr, size, vk::Format::eR8G8B8A8Unorm);
     */

    nova::nova_renderer::instance->render_frame();
}
