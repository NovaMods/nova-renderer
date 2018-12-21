/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include "general_test_setup.hpp"
#undef TEST

#include <iostream>
#include <gtest/gtest.h>

namespace nova {
    int main() {
        TEST_SETUP_LOGGER();

        char buff[FILENAME_MAX];
        getcwd(buff, FILENAME_MAX);
        NOVA_LOG(DEBUG) << "Running in " << buff << std::flush;
        NOVA_LOG(DEBUG) << "Predefined resources at: " << CMAKE_DEFINED_RESOURCES_PREFIX;

        settings_options settings;
        settings.api = graphics_api::vulkan;
        settings.vulkan.application_name = "Nova Renderer test";
        settings.vulkan.application_version = { 0, 8, 0 };
        settings.window.width = 640;
        settings.window.height = 480;
        auto renderer = nova_renderer::initialize(settings);

        renderer->load_shaderpack(CMAKE_DEFINED_RESOURCES_PREFIX "shaderpacks/DefaultShaderpack");

        std::shared_ptr<iwindow> window = renderer->get_engine()->get_window();

        while (!window->should_close()) {
            renderer->execute_frame();
            window->on_frame_end();
        }

        nova_renderer::deinitialize();

        return 0;
    }
}


int main() {
    return nova::main();
}