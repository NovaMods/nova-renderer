/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include <iostream>

#include "../../src/nova_renderer.hpp"

#include "../../src/platform.hpp"

#include "../../src/util/logger.hpp"
#include "../../src/render_engine/vulkan/vulkan_render_engine.hpp"

int main(int num_args, const char** args) {
    // Add default logging handlers

    auto& log = nova::logger::instance;
    log.add_log_handler(nova::log_level::TRACE,     [] (auto msg) {std::cout << "TRACE: " << msg << "\n";});
    log.add_log_handler(nova::log_level::DEBUG,     [] (auto msg) {std::cout << "DEBUG: " << msg << "\n";});
    log.add_log_handler(nova::log_level::INFO,      [] (auto msg) {std::cout << "INFO: " << msg << "\n";});
    log.add_log_handler(nova::log_level::WARN,      [] (auto msg) {std::cerr << "WARN: " << msg << "\n";});
    log.add_log_handler(nova::log_level::ERROR,     [] (auto msg) {std::cerr << "ERROR: " << msg << "\n";});
    log.add_log_handler(nova::log_level::FATAL,     [] (auto msg) {std::cerr << "FATAL: " << msg << "\n";});
    log.add_log_handler(nova::log_level::MAX_LEVEL, [] (auto msg) {std::cerr << "MAX_LEVEL: " << msg << "\n";});

#ifdef __linux__
    auto renderer = nova::nova_renderer<nova::vulkan_render_engine>::initialize();
#elif defined(_WIN32)
    auto renderer = nova::nova_renderer<nova::dx12_render_engine>::initialize();
#else
#error Unsupported Platform
#endif

    system("PAUSE");
    return 0;
}
