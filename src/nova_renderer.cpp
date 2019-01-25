/*!
 * \author ddubois
 * \date 03-Sep-18.
 */

#include <future>

#include "nova_renderer.hpp"

#include "util/logger.hpp"
#include "glslang/MachineIndependent/Initialize.h"
#include "loading/shaderpack/shaderpack_loading.hpp"

#if _WIN32
#include "render_engine/dx12/dx12_render_engine.hpp"
#endif 

#include "render_engine/vulkan/vulkan_render_engine.hpp"
#include "debugging/renderdoc.hpp"

namespace nova {
    nova_renderer *nova_renderer::instance;

    nova_renderer::nova_renderer(const settings_options &settings) : 
		render_settings(settings), task_scheduler(1, ttl::empty_queue_behavior::YIELD) {

        if(settings.debug.renderdoc.enable) {
#if _WIN32
			render_doc = load_renderdoc(settings.debug.renderdoc.renderdoc_dll_path);
#endif

            if(render_doc) {
				render_doc->SetCaptureFilePathTemplate(settings.debug.renderdoc.capture_path.c_str());

				RENDERDOC_InputButton captureKey = eRENDERDOC_Key_F12;
				render_doc->SetCaptureKeys(&captureKey, 1);

				render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowFullscreen, true);
				render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, true);
				render_doc->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, true);
				render_doc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, true);
            }
        }

        switch(settings.api) {
        case graphics_api::dx12:
            #if _WIN32
            engine = std::make_unique<dx12_render_engine>(render_settings, &task_scheduler);
            break;
            #endif
        case graphics_api::vulkan:
            engine = std::make_unique<vulkan_render_engine>(render_settings, &task_scheduler);
        }
    }

	nova_settings &nova_renderer::get_settings() {
        return render_settings;
    }

    void nova_renderer::execute_frame() const {
		engine->render_frame();
    }

    void nova_renderer::load_shaderpack(const std::string &shaderpack_name) const {
        glslang::InitializeProcess();

        const shaderpack_data shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));

        engine->set_shaderpack(shaderpack_data);
		NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name << " loaded successfully";
    }

    render_engine *nova_renderer::get_engine() const {
        return engine.get();
    }

    nova_renderer *nova_renderer::get_instance() {
        return instance;
    }

    void nova_renderer::deinitialize() {
        delete instance;
    }

	ttl::task_scheduler &nova_renderer::get_task_scheduler() {
        return task_scheduler;
    }
}  // namespace nova
