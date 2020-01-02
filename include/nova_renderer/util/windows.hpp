#pragma once

#include "nova_renderer/util/platform.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef NOVA_WINDOWS
#include <windows.h>
#undef ERROR
#else
#error "Trying to include windows on non-windows build."
#endif
