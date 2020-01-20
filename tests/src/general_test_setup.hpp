#pragma once

#include <fstream>
#include <iostream>

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/util/logger.hpp"
#include "nova_renderer/util/platform.hpp"

#include "../../src/filesystem/zip_folder_accessor.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>

#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#ifndef CMAKE_DEFINED_RESOURCES_PREFIX
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CMAKE_DEFINED_RESOURCES_PREFIX ""
#endif

#ifndef TEST_SETUP_LOGGER // Tests are weird... this is done to avoid some linking errors
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_SETUP_LOGGER                                                                                                                  \
    [] {                                                                                                                                   \
        auto error_log = std::make_shared<std::ofstream>();                                                                                \
        error_log->open("test_error_log.log");                                                                                             \
        auto test_log = std::make_shared<std::ofstream>("test_log.log");                                                                   \
        auto& log = nova::renderer::Logger::instance;                                                                                      \
        log.add_log_handler(nova::renderer::TRACE, [test_log](auto msg) {                                                                  \
            std::cout << "TRACE: " << msg.c_str() << std::endl;                                                                            \
            *test_log << "TRACE: " << msg.c_str() << std::endl;                                                                            \
        });                                                                                                                                \
        log.add_log_handler(nova::renderer::DEBUG, [test_log](auto msg) {                                                                  \
            std::cout << "DEBUG: " << msg.c_str() << std::endl;                                                                            \
            *test_log << "DEBUG: " << msg.c_str() << std::endl;                                                                            \
        });                                                                                                                                \
        log.add_log_handler(nova::renderer::INFO, [test_log](auto msg) {                                                                   \
            std::cout << "INFO: " << msg.c_str() << std::endl;                                                                             \
            *test_log << "INFO: " << msg.c_str() << std::endl;                                                                             \
        });                                                                                                                                \
        log.add_log_handler(nova::renderer::WARN, [test_log](auto msg) {                                                                   \
            std::cerr << "WARN: " << msg.c_str() << std::endl;                                                                             \
            *test_log << "WARN: " << msg.c_str() << std::endl;                                                                             \
        });                                                                                                                                \
        log.add_log_handler(nova::renderer::ERROR, [test_log, error_log](auto msg) {                                                       \
            std::cerr << "ERROR: " << msg.c_str() << std::endl;                                                                            \
            *error_log << "ERROR: " << msg.c_str() << std::endl << std::flush;                                                             \
            *test_log << "ERROR: " << msg.c_str() << std::endl;                                                                            \
        });                                                                                                                                \
        log.add_log_handler(nova::renderer::FATAL, [test_log, error_log](auto msg) {                                                       \
            std::cerr << "FATAL: " << msg.c_str() << std::endl;                                                                            \
            *error_log << "FATAL: " << msg.c_str() << std::endl << std::flush;                                                             \
            *test_log << "FATAL: " << msg.c_str() << std::endl;                                                                            \
        });                                                                                                                                \
        log.add_log_handler(nova::renderer::MAX_LEVEL, [test_log, error_log](auto msg) {                                                   \
            std::cerr << "MAX_LEVEL: " << msg.c_str() << std::endl;                                                                        \
            *error_log << "MAX_LEVEL: " << msg.c_str() << std::endl << std::flush;                                                         \
            *test_log << "MAX_LEVEL: " << msg.c_str() << std::endl;                                                                        \
        });                                                                                                                                \
    }
#endif

#ifndef TEST_CONFIGURE_RUNTIME
#ifdef _WIN32
#define TEST_CONFIGURE_RUNTIME                                                                                                             \
    [] {                                                                                                                                   \
        SetErrorMode(SEM_NOGPFAULTERRORBOX);                                                                                               \
        _set_abort_behavior(0, _WRITE_ABORT_MSG);                                                                                          \
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);                                                                                  \
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);                                                                                 \
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);                                                                                \
    }
#else
#define TEST_CONFIGURE_RUNTIME()
#endif
#endif