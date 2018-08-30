/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "nova_renderer.hpp"
#include "loading/loading_utils.hpp"
#include "loading/shaderpack_loading.hpp"

#include <experimental/filesystem>

#include "platform.hpp"

#if SUPPORT_DX12
#include "render_engine/dx12/dx_12_render_engine.hpp"
#define RenderEngineType dx12_render_engine
#endif


namespace fs = std::experimental::filesystem;

namespace nova {
    std::unique_ptr<nova_renderer<RenderEngineType>> nova_renderer<RenderEngineType>::instance;

    nova_renderer<RenderEngineType> *nova_renderer<RenderEngineType>::initialize() {
        if(!instance) {
            instance = std::make_unique<nova_renderer<RenderEngineType>>();
        } else {
            throw already_initialized_exception("Nova has already been initialized");
        }
    }

    nova_renderer<RenderEngineType> *nova_renderer<RenderEngineType>::get_instance() {
        if(!instance) {
            throw uninitialized_exception("Nova has not been initialized yet");
        }

        return instance.get();
    }

    void nova_renderer<RenderEngineType>::deinitialize() {
        if(instance) {
            instance.reset(nullptr);
        }
    }

    settings &nova_renderer<RenderEngineType>::get_settings() {
        return render_settings;
    }

    void nova_renderer<RenderEngineType>::execute_frame() {

    }

    void nova_renderer<RenderEngineType>::load_shaderpack(const std::string &shaderpack_name) {
        const auto shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));
    }
}
