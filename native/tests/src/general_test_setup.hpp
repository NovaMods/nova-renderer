//
// Created by jannis on 07.09.18.
//

#ifndef NOVA_RENDERER_GENERAL_TEST_SETUP_HPP
#define NOVA_RENDERER_GENERAL_TEST_SETUP_HPP

#include <iostream>
#include "../../src/util/logger.hpp"

#ifndef TEST_SETUP_LOGGER // Tests are weird... this is done to avoid some linking errors
#define TEST_SETUP_LOGGER \
[]{ \
    auto &log = nova::logger::instance; \
    log.add_log_handler(nova::log_level::TRACE, [](auto msg) { std::cout << "TRACE: " << msg << std::endl; }); \
    log.add_log_handler(nova::log_level::DEBUG, [](auto msg) { std::cout << "DEBUG: " << msg << std::endl;}); \
    log.add_log_handler(nova::log_level::INFO, [](auto msg) { std::cout << "INFO: " << msg << std::endl; }); \
    log.add_log_handler(nova::log_level::WARN, [](auto msg) { std::cerr << "WARN: " << msg << std::endl; }); \
    log.add_log_handler(nova::log_level::ERROR, [](auto msg) { std::cerr << "ERROR: " << msg << std::endl; }); \
    log.add_log_handler(nova::log_level::FATAL, [](auto msg) { std::cerr << "FATAL: " << msg << std::endl; }); \
    log.add_log_handler(nova::log_level::MAX_LEVEL, [](auto msg) { std::cerr << "MAX_LEVEL: " << msg << std::endl; }); \
}

#include "../../src/settings/settings.hpp"
#include "../../src/loading/zip_folder_accessor.hpp"
#include "../../src/nova_renderer.hpp"
#include "../../src/platform.hpp"
#ifdef _WIN32
#include <direct.h>
#include "../../src/render_engine/dx12/dx12_render_engine.hpp"
#define getcwd _getcwd
#else
#include <unistd.h>
#include "../../src/render_engine/vulkan/vulkan_render_engine.hpp"
#endif

#endif

#endif //NOVA_RENDERER_GENERAL_TEST_SETUP_HPP
