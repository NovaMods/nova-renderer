#pragma once

#ifdef _WIN32
#define NOVA_WINDOWS 1
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
#define NOVA_LINUX 1
#endif
