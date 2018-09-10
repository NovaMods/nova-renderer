/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include <iostream>
#include <gtest/gtest.h>

#include "general_test_setup.hpp"

namespace nova {
    int main() {
        TEST_SETUP_LOGGER();

        char buff[FILENAME_MAX];
        getcwd(buff, FILENAME_MAX);
        NOVA_LOG(DEBUG) << "Running in " << buff << std::flush;

        nova::settings settings;
        auto renderer = nova::nova_renderer::initialize(settings);

        std::shared_ptr<nova::iwindow> window = renderer->get_engine()->get_window();

        NOVA_LOG(DEBUG) << window->should_close();

        while (!window->should_close()) {
            renderer->execute_frame();
            window->on_frame_end();
        }

        nova::nova_renderer::deinitialize();

        return 0;
    }
}

TEST(nova_renderer, end_to_end) {
    nova::main();
}