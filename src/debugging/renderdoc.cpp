#include "renderdoc.hpp"

#include "../platform.hpp"
#include "../util/logger.hpp"

#if NOVA_WINDOWS
#include <windows.h>
#include "../util/windows_utils.hpp"

// Fucking hell
#ifdef ERROR
#undef ERROR
#endif
#elif NOVA_LINUX
#include <dlfcn.h>
#include "../util/linux_utils.hpp"
#endif

namespace nova {
    RENDERDOC_API_1_3_0* load_renderdoc(const std::string& renderdoc_dll_path) {
#if NOVA_WINDOWS
        HINSTANCE const renderdoc_dll = LoadLibrary(renderdoc_dll_path.c_str());
        if(!renderdoc_dll) {
            const std::string error = get_last_windows_error();
            NOVA_LOG(ERROR) << "Could not load RenderDoc. Error: " << error;

            return nullptr;
        }

        const auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(renderdoc_dll, "RENDERDOC_GetAPI"));
        if(!get_api) {
            const std::string error = get_last_windows_error();
            NOVA_LOG(ERROR) << "Could not find the RenderDoc API loading function. Error: " << error;

            return nullptr;
        }

#elif NOVA_LINUX
        void *renderdoc_so = dlopen(renderdoc_dll_path.c_str(), RTLD_NOW);
        if(!renderdoc_so) {
            // Try to load system-wide version of renderdoc
            renderdoc_so = dlopen("librenderdoc.so", RTLD_NOW);
            if(!renderdoc_so) {
                NOVA_LOG(ERROR) << "Could not load RenderdDoc. Error: " << dlerror();
                return nullptr;
            }
        }

        const auto get_api = (pRENDERDOC_GetAPI) dlsym(renderdoc_so, "RENDERDOC_GetAPI");
        if(!get_api) {
            NOVA_LOG(ERROR) << "Could not find the RenderDoc API loading function. Error: " << dlerror();
            return nullptr;
        }
#endif

        RENDERDOC_API_1_3_0* api;
        const uint32_t ret = get_api(eRENDERDOC_API_Version_1_3_0, reinterpret_cast<void**>(&api));
        if(ret != 1) {
            NOVA_LOG(ERROR) << "Could not load RenderDoc API";

            return nullptr;
        }
        return api;
    }
}
