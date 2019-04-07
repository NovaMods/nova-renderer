/*!
 * \author ddubois
 * \date 03-Sep-18.
 */

#include <array>
#include <future>

#include <nova_renderer/nova_renderer.hpp>

#include "glslang/MachineIndependent/Initialize.h"
#include "loading/shaderpack/shaderpack_loading.hpp"

#if defined(NOVA_WINDOWS)
#include "render_engine/dx12/d3d12_render_engine.hpp"
#endif

#include "debugging/renderdoc.hpp"
#include "render_engine/vulkan/vulkan_render_engine.hpp"

#include <minitrace/minitrace.h>
#include "render_engine/gl2/gl2_render_engine.hpp"
#include "util/logger.hpp"

namespace nova::renderer {
    std::unique_ptr<nova_renderer> nova_renderer::instance;

    nova_renderer::nova_renderer(nova_settings settings) : render_settings(settings) {

        mtr_init("trace.json");

        MTR_META_PROCESS_NAME("NovaRenderer");
        MTR_META_THREAD_NAME("Main");

        MTR_SCOPE("Init", "nova_renderer::nova_renderer");

        if(settings.debug.renderdoc.enabled) {
            MTR_SCOPE("Init", "LoadRenderdoc");
            auto rd_load_result = load_renderdoc(settings.debug.renderdoc.renderdoc_dll_path);

            rd_load_result
                .map([&](RENDERDOC_API_1_3_0* api) {
                    render_doc = api;

                    render_doc->SetCaptureFilePathTemplate(settings.debug.renderdoc.capture_path.c_str());

                    RENDERDOC_InputButton capture_key[] = {eRENDERDOC_Key_F12, eRENDERDOC_Key_PrtScrn};
                    render_doc->SetCaptureKeys(capture_key, 2);

                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowFullscreen, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_VerifyMapWrites, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1U);
                    render_doc->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1U);

                    NOVA_LOG(INFO) << "Loaded RenderDoc successfully";

                    return 0;
                })
                .on_error([](const nova_error& error) { NOVA_LOG(ERROR) << error.to_string(); });
        }

        switch(settings.api) {
            case graphics_api::dx12:
#if defined(NOVA_WINDOWS)
            {
                MTR_SCOPE("Init", "InitDirect3D12RenderEngine");
                engine = std::make_unique<rhi::d3d12_render_engine>(render_settings);
            } break;
#else
                NOVA_LOG(WARN) << "You selected the DX12 graphics API, but your system doesn't support it. Defaulting to Vulkan";
                [[fallthrough]];
#endif
            case graphics_api::vulkan: {
                MTR_SCOPE("Init", "InitVulkanRenderEngine");
                engine = std::make_unique<rhi::vk_render_engine>(render_settings);
            } break;

            case graphics_api::gl2: {
                MTR_SCOPE("Init", "InitGL2RenderEngine");
                engine = std::make_unique<rhi::gl2_render_engine>(render_settings);
            } break;
        }
    }

    nova_renderer::~nova_renderer() { mtr_shutdown(); }

    nova_settings& nova_renderer::get_settings() { return render_settings; }

    void nova_renderer::execute_frame() const {
        mtr_flush();

        MTR_SCOPE("RenderLoop", "execute_frame");
    }

    void nova_renderer::load_shaderpack(const std::string& shaderpack_name) {
        MTR_SCOPE("ShaderpackLoading", "load_shaderpack");
        glslang::InitializeProcess();

        const shaderpack::shaderpack_data_t data = shaderpack::load_shaderpack_data(fs::path(shaderpack_name));

        if(shaderpack_loaded) {
            destroy_render_passes();
            destroy_graphics_pipelines();
            materials.clear();
            material_passes_by_pipeline.clear();
            destroy_dynamic_resources();

            NOVA_LOG(DEBUG) << "Resources from old shaderpacks destroyed";
        }

        create_textures(data.resources.textures);
        NOVA_LOG(DEBUG) << "Dynamic textures created";
        for(const material_data& mat_data : data.materials) {
            materials[mat_data.name] = mat_data;

            for(const material_pass& mat : mat_data.passes) {
                material_passes_by_pipeline[mat.pipeline].push_back(mat);
            }
        }
        NOVA_LOG(DEBUG) << "Materials saved";

        create_render_passes(data.passes);
        NOVA_LOG(DEBUG) << "Created render passes";
        create_graphics_pipelines(data.pipelines);
        NOVA_LOG(DEBUG) << "Created pipelines";

        create_material_descriptor_sets();
        NOVA_LOG(TRACE) << "Material descriptor sets created";

        generate_barriers_for_dynamic_resources();

        shaderpack_loaded = true;

        NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name << " loaded successfully";
    } 

    void nova_renderer::destroy_render_passes() {
        for(renderpass_t* renderpass : renderpasses) {
            engine->destroy_renderpass(renderpass->renderpass);
        }

        renderpasses.clear();
    }

    rhi::render_engine_t* nova_renderer::get_engine() const { return engine.get(); }

    nova_renderer* nova_renderer::get_instance() { return instance.get(); }

    nova_renderer* nova_renderer::initialize(const nova_settings& settings) {
        return (instance = std::make_unique<nova_renderer>(settings)).get();
    }

    void nova_renderer::deinitialize() { instance.reset(); }
} // namespace nova::renderer
