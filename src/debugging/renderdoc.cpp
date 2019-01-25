#include "renderdoc.hpp"

#include "../util/logger.hpp"
#include "../util/windows_utils.hpp"

#if _WIN32
#include <windows.h>

// Fucking hell
#ifdef ERROR
#undef ERROR
#endif
#endif

namespace nova {
    RENDERDOC_API_1_3_0* load_renderdoc(const std::string& renderdoc_dll_path) {
#if _WIN32
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

#elif __linux__
		void* renderdoc_so = dlopen(renderdoc_dll_path.c_str());
        // TODO
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
