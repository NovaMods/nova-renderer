#include "renderdoc.hpp"

#include "../platform.hpp"
#include "../util/logger.hpp"

#if defined(NOVA_WINDOWS)
#include "../util/windows.hpp"
#include "../util/windows_utils.hpp"

// Fucking hell
#ifdef ERROR
#undef ERROR
#endif
#elif defined(NOVA_LINUX)
#include <dlfcn.h>
#include "../util/linux_utils.hpp"
#endif

namespace nova::renderer {
    RENDERDOC_API_1_3_0* load_renderdoc(const std::string& renderdoc_dll_path) {
#if defined(NOVA_WINDOWS)
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

#elif defined(NOVA_LINUX)
        void* renderdoc_so = dlopen(renderdoc_dll_path.c_str(), RTLD_NOW);
        if(renderdoc_so == nullptr) {
            // Try to load system-wide version of renderdoc
            renderdoc_so = dlopen("librenderdoc.so", RTLD_NOW);
            if(renderdoc_so == nullptr) {
                NOVA_LOG(ERROR) << "Could not load RenderdDoc. Error: " << dlerror();
                return nullptr;
            }
        }

        const auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(renderdoc_so, "RENDERDOC_GetAPI"));
        if(get_api == nullptr) {
            NOVA_LOG(ERROR) << "Could not find the RenderDoc API loading function. Error: " << dlerror();
            return nullptr;
        }
#endif

        RENDERDOC_API_1_3_0* api;
        const int32_t ret = get_api(eRENDERDOC_API_Version_1_3_0, reinterpret_cast<void**>(&api));
        if(ret != 1) {
            NOVA_LOG(ERROR) << "Could not load RenderDoc API";

            return nullptr;
        }
        return api;
    }
} // namespace nova::renderer
