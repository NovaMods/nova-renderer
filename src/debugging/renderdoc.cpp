#include "renderdoc.hpp"

#include "../platform.hpp"
#include "../util/logger.hpp"
#include "../util/utils.hpp"

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
    result<RENDERDOC_API_1_3_0*> load_renderdoc(const std::string& renderdoc_dll_path) {
#if defined(NOVA_WINDOWS)
        using HINSTANCE = HINSTANCE__* const;
        HINSTANCE renderdoc_dll = LoadLibrary(renderdoc_dll_path.c_str());
        if(!renderdoc_dll) {
            const std::string error = get_last_windows_error();
            return result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc. Error: {:s}", error));
        }

        NOVA_LOG(TRACE) << "Loaded RenderDoc DLL from " << renderdoc_dll_path;

        const auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(renderdoc_dll, "RENDERDOC_GetAPI"));
        if(!get_api) {
            const std::string error = get_last_windows_error();
            return result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc DLL. Error: {:s}", error));
        }

#elif defined(NOVA_LINUX)
        void* renderdoc_so = dlopen(renderdoc_dll_path.c_str(), RTLD_NOW);
        if(renderdoc_so == nullptr) {
            // Try to load system-wide version of renderdoc
            renderdoc_so = dlopen("librenderdoc.so", RTLD_NOW);
            if(renderdoc_so == nullptr) {
                return result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc DLL. Error: {:s}", dlerror()));
            }
        }

        const auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(renderdoc_so, "RENDERDOC_GetAPI"));
        if(get_api == nullptr) {
            return result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not find the RenderDoc API loading function. Error: {:s}", dlerror()));
        }
#endif

        RENDERDOC_API_1_3_0* api;
        const int32_t ret = get_api(eRENDERDOC_API_Version_1_3_0, reinterpret_cast<void**>(&api));
        if(ret != 1) {
            NOVA_LOG(ERROR) << "Could not load RenderDoc API";

            return result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc API. Error code {:d}", ret));
        }

        NOVA_LOG(TRACE) << "Loaded RenderDoc 1.3 API";
        return result(api);
    }
} // namespace nova::renderer
