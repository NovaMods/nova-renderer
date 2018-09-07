/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include <iostream>
#include <gtest/gtest.h>

#include "../../src/nova_renderer.hpp"
#include "../../src/platform.hpp"
#include "general_test_setup.hpp"

#ifdef _WIN32
#include <direct.h>
#include "../../src/render_engine/dx12/dx12_render_engine.hpp"
#define getcwd _getcwd
#else
#include <unistd.h>
#include "../../src/render_engine/vulkan/vulkan_render_engine.hpp"
#endif

#include "../../src/util/logger.hpp"
#include "../../src/loading/zip_folder_accessor.hpp"
#include "../../src/settings/settings.hpp"

namespace nova {
    int main() {
        TEST_SETUP_LOGGER();

        char buff[FILENAME_MAX];
        getcwd(buff, FILENAME_MAX);
        NOVA_LOG(DEBUG) << "Running in " << buff << std::flush;

        nova::settings settings;
        auto renderer = nova::nova_renderer::initialize(settings);

        std::shared_ptr<nova::iwindow> window = renderer->get_engine()->get_window();

        while (!window->should_close()) {
            // renderer->execute_frame();
            window->on_frame_end();
        }

        nova::nova_renderer::deinitialize();

        return 0;
    }
}

TEST(nova_renderer, end_to_end) {
    nova::main();
}