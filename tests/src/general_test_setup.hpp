#pragma once

#include <fstream>
#include <iostream>

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/nova_settings.hpp"
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
