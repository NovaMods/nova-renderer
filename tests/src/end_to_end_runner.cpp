/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include "general_test_setup.hpp"
#undef TEST

#include <iostream>

#ifdef __linux__
#include <signal.h>
void sigsegv_handler(int signal);
void sigabrt_handler(int signal);
#include "../../src/util/linux_utils.hpp"
#endif

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
        settings.debug.enabled = true;
        settings.debug.renderdoc.enabled = false;
        settings.window.width = 640;
        settings.window.height = 480;
        const auto renderer = nova_renderer::initialize(settings);

        renderer->load_shaderpack(CMAKE_DEFINED_RESOURCES_PREFIX "shaderpacks/DefaultShaderpack");

        std::shared_ptr<iwindow> window = renderer->get_engine()->get_window();

        while(!window->should_close()) {
            renderer->execute_frame();
            window->on_frame_end();
        }

        nova_renderer::deinitialize();

        return 0;
    }
}


int main() {
#ifdef __linux__
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGABRT, sigabrt_handler);
#endif
    return nova::main();
}

#ifdef __linux__
void sigsegv_handler(int sig) {
    signal(sig, SIG_IGN);

    std::cerr << "!!!SIGSEGV!!!" << std::endl;
    nova_backtrace();

    _exit(1);
}

void sigabrt_handler(int sig) {
    signal(sig, SIG_IGN);

    std::cerr << "!!!SIGABRT!!!" << std::endl;
    nova_backtrace();

    _exit(1);
}
#endif