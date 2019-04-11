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
#include "loading/shaderpack/render_graph_builder.hpp"
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
                rhi = std::make_unique<rhi::d3d12_render_engine>(render_settings);
            } break;
#else
                NOVA_LOG(WARN) << "You selected the DX12 graphics API, but your system doesn't support it. Defaulting to Vulkan";
                [[fallthrough]];
#endif
            case graphics_api::vulkan: {
                MTR_SCOPE("Init", "InitVulkanRenderEngine");
                rhi = std::make_unique<rhi::vk_render_engine>(render_settings);
            } break;

            case graphics_api::gl2: {
                MTR_SCOPE("Init", "InitGL2RenderEngine");
                rhi = std::make_unique<rhi::gl2_render_engine>(render_settings);
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

            destroy_dynamic_resources();

            NOVA_LOG(DEBUG) << "Resources from old shaderpacks destroyed";
        }

        create_dynamic_textures(data.resources.textures);
        NOVA_LOG(DEBUG) << "Dynamic textures created";

        create_render_passes(data.passes, data.pipelines, data.materials);
        NOVA_LOG(DEBUG) << "Created render passes";

        shaderpack_loaded = true;

        NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name << " loaded successfully";
    }

    void nova_renderer::create_dynamic_textures(const std::vector<shaderpack::texture_create_info_t>& texture_create_infos) {
        for(const shaderpack::texture_create_info_t& create_info : texture_create_infos) {
            rhi::resource_t* new_texture = rhi->create_texture(create_info);
            dynamic_textures.emplace(create_info.name, new_texture);
        }
    }

    result<renderpass_t> nova_renderer::create_framebuffer_for_renderpass(const shaderpack::render_pass_create_info_t& create_info,
                                                                          rhi::renderpass_t* new_pass) {
        renderpass_t renderpass;
        renderpass.renderpass = new_pass;

        std::vector<rhi::image_t*> output_images;
        output_images.reserve(create_info.texture_outputs.size());

        glm::uvec2 framebuffer_size(0);

        std::vector<std::string> attachment_errors;
        attachment_errors.reserve(create_info.texture_outputs.size());

        for(const shaderpack::texture_attachment_info_t& attachment_info : create_info.texture_outputs) {
            if(attachment_info.name == "Backbuffer") {
                if(create_info.texture_outputs.size() == 1) {
                    renderpass.writes_to_backbuffer = true;
                    renderpass.framebuffer = nullptr; // Will be resolved when rendering

                } else {
                    return result<renderpass_t>(MAKE_ERROR(
                        "Pass {:s} writes to the backbuffer and {:d} other textures, but that's not allowed. If a pass writes to the backbuffer, it can't write to any other textures",
                        create_info.name,
                        create_info.texture_outputs.size() - 1));
                }

            } else {
                rhi::image_t* image = dynamic_textures.at(attachment_info.name);
                output_images.push_back(image);

                const shaderpack::texture_create_info_t& info = dynamic_texture_infos.at(attachment_info.name);
                const glm::uvec2 attachment_size = info.format.get_size_in_pixels(
                    {render_settings.window.width, render_settings.window.height});

                if(framebuffer_size.x > 0) {
                    if(attachment_size.x != framebuffer_size.x || attachment_size.y != framebuffer_size.y) {
                        attachment_errors.push_back(fmt::format(
                            fmt("Attachment {:s} has a size of {:d}x{:d}, but the framebuffer for pass {:s} has a size of {:d}x{:d} - these must match! All attachments of a single renderpass must have the same size"),
                            attachment_info.name,
                            attachment_size.x,
                            attachment_size.y,
                            create_info.name,
                            framebuffer_size.x,
                            framebuffer_size.y));
                    }

                } else {
                    framebuffer_size = attachment_size;
                }
            }
        }

        renderpass.framebuffer = rhi->create_framebuffer(new_pass, output_images, framebuffer_size);
        return result(renderpass);
    }

    void nova_renderer::create_render_passes(const std::vector<shaderpack::render_pass_create_info_t>& pass_create_infos,
                                             const std::vector<shaderpack::pipeline_create_info_t>& pipelines,
                                             const std::vector<shaderpack::material_data_t>& materials) {
        rhi->set_num_renderpasses(static_cast<uint32_t>(pass_create_infos.size()));

        for(const shaderpack::render_pass_create_info_t& create_info : pass_create_infos) {
            auto create_framebuffer = std::bind(&nova_renderer::create_framebuffer_for_renderpass, this, create_info, std::placeholders::_1);
            result<renderpass_t> renderpass_result = rhi->create_renderpass(create_info)
                                                         .flat_map(create_framebuffer);
        }
    }

    void nova_renderer::destroy_render_passes() {
        for(renderpass_t& renderpass : renderpasses) {
            rhi->destroy_renderpass(renderpass.renderpass);
            rhi->destroy_framebuffer(renderpass.framebuffer);

            for(pipeline_t& pipeline : renderpass.pipelines) {
                rhi->destroy_pipeline(pipeline.pipeline);

                for(material_pass_t& material_pass : pipeline.passes) {
                    // TODO: Destroy descriptors for material
                    // TODO: Have a way to save mesh data somewhere outside of the render graph, then process it cleanly here
                }
            }
        }

        renderpasses.clear();
    }

    void nova_renderer::destroy_dynamic_resources() {
        for(auto& [name, image] : dynamic_textures) {
            rhi->destroy_texture(image);
        }

        dynamic_textures.clear();

        // TODO: Also destroy dynamic buffers, when we have support for those
    }

    rhi::render_engine_t* nova_renderer::get_engine() const { return rhi.get(); }

    nova_renderer* nova_renderer::get_instance() { return instance.get(); }

    nova_renderer* nova_renderer::initialize(const nova_settings& settings) {
        return (instance = std::make_unique<nova_renderer>(settings)).get();
    }

    void nova_renderer::deinitialize() { instance.reset(); }
} // namespace nova::renderer
