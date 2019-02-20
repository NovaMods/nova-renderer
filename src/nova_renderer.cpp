/*!
 * \author ddubois
 * \date 03-Sep-18.
 */

#include <future>

#include "nova_renderer.hpp"

#include "glslang/MachineIndependent/Initialize.h"
#include "loading/shaderpack/shaderpack_loading.hpp"
#include "util/logger.hpp"

#if defined(NOVA_WINDOWS)
#include "render_engine/dx12/dx12_render_engine.hpp"
#endif

#include "debugging/renderdoc.hpp"
#include "render_engine/vulkan/vulkan_render_engine.hpp"

#include "minitrace.h"

namespace nova::renderer {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    nova_renderer::nova_renderer(const settings_options& settings)
        : render_settings(settings), task_scheduler(1, ttl::empty_queue_behavior::YIELD) {

        mtr_init("trace.json");

        MTR_META_PROCESS_NAME("NovaRenderer");
        MTR_META_THREAD_NAME("Main");

        MTR_SCOPE("Init", "nova_renderer::nova_renderer");

        if(settings.debug.renderdoc.enabled) {
            MTR_SCOPE("Init", "LoadRenderdoc");
            render_doc = load_renderdoc(settings.debug.renderdoc.renderdoc_dll_path);

            if(render_doc != nullptr) {
                render_doc->SetCaptureFilePathTemplate(settings.debug.renderdoc.capture_path.c_str());

                RENDERDOC_InputButton captureKey = eRENDERDOC_Key_PrtScrn;
                render_doc->SetCaptureKeys(&captureKey, 1);

                render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowFullscreen, 1U);
                render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, 1U);
                render_doc->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, 1U);
                render_doc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1U);
                render_doc->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1U);
            }
        }

        switch(settings.api) {
            case graphics_api::dx12:
#if defined(NOVA_WINDOWS)
            {
                MTR_SCOPE("Init", "InitDirectX12RenderEngine");
                engine = std::make_unique<dx12_render_engine>(render_settings, &task_scheduler);
            } break;
#endif
            case graphics_api::vulkan:
                MTR_SCOPE("Init", "InitVulkanRenderEngine");
                engine = std::make_unique<vulkan_render_engine>(render_settings, &task_scheduler);
        }
    }

    nova_renderer::~nova_renderer() { mtr_shutdown(); }

    nova_settings& nova_renderer::get_settings() { return render_settings; }

    void nova_renderer::execute_frame() const {
        MTR_SCOPE("RenderLoop", "execute_frame");
        engine->render_frame();

        mtr_flush();
    }

    void nova_renderer::load_shaderpack(const std::string& shaderpack_name) const {
        MTR_SCOPE("ShaderpackLoading", "load_shaderpack");
        glslang::InitializeProcess();

        const shaderpack_data shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));

        engine->set_shaderpack(shaderpack_data);
        NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name << " loaded successfully";
    }

    render_engine* nova_renderer::get_engine() const { return engine.get(); }

    nova_renderer* nova_renderer::get_instance() { return instance.get(); }

    nova_renderer* nova_renderer::initialize(const settings_options& settings) {
        return (instance = std::make_unique<nova_renderer>(settings)).get();
    }

    void nova_renderer::deinitialize() { instance = nullptr; }

    ttl::task_scheduler& nova_renderer::get_task_scheduler() { return task_scheduler; }
} // namespace nova
