#pragma once

#ifdef _WIN32
#define SUPPORT_DX12    // TODO: Have the build script set the API defines
#define NOVA_WINDOWS
#endif

#if defined(linux) || defined(__linux) || defined(__linux__)
#define NOVA_LINUX
#endif
