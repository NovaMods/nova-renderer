#include "renderdoc.hpp"

#include <rx/core/log.h>

#include "nova_renderer/util/platform.hpp"
#include "nova_renderer/util/utils.hpp"

#if defined(NOVA_WINDOWS)
#include "nova_renderer/util/windows.hpp"

#include "../util/windows_utils.hpp"

#elif defined(NOVA_LINUX)
#include <dlfcn.h>

#include "../util/linux_utils.hpp"
#endif

namespace nova::renderer {
    RX_LOG("RenderDoc", logger);

    ntl::Result<RENDERDOC_API_1_3_0*> load_renderdoc(const rx::string& renderdoc_dll_path) {
#if defined(NOVA_WINDOWS)
        using Hinstance = HINSTANCE__* const;
        Hinstance renderdoc_dll = LoadLibrary(renderdoc_dll_path.data());
        if(!renderdoc_dll) {
            const rx::string error = get_last_windows_error();
            return ntl::Result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc. Error: %s", error));
        }

        logger->verbose("Loaded RenderDoc DLL from %s", renderdoc_dll_path);

        const auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(renderdoc_dll, "RENDERDOC_GetAPI"));
        if(!get_api) {
            const rx::string error = get_last_windows_error();
            return ntl::Result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc DLL. Error: %s", error));
        }

#elif defined(NOVA_LINUX)
        void* renderdoc_so = dlopen(renderdoc_dll_path.data(), RTLD_NOW);
        if(renderdoc_so == nullptr) {
            // Try to load system-wide version of renderdoc
            renderdoc_so = dlopen("librenderdoc.so", RTLD_NOW);
            if(renderdoc_so == nullptr) {
                return ntl::Result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc DLL. Error: %s", dlerror()));
            }
        }

        const auto get_api = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(renderdoc_so, "RENDERDOC_GetAPI"));
        if(get_api == nullptr) {
            return ntl::Result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not find the RenderDoc API loading function. Error: %s", dlerror()));
        }
#endif

        RENDERDOC_API_1_3_0* api;
        const auto ret = get_api(eRENDERDOC_API_Version_1_3_0, reinterpret_cast<void**>(&api));
        if(ret != 1) {
            logger->error("Could not load RenderDoc API");

            return ntl::Result<RENDERDOC_API_1_3_0*>(MAKE_ERROR("Could not load RenderDoc API. Error code %d", ret));
        }

        logger->verbose("Loaded RenderDoc 1.3 API");
        return ntl::Result(api);
    }
} // namespace nova::renderer
