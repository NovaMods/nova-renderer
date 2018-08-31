/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#include "nova_renderer.hpp"
#include "loading/loading_utils.hpp"
#include "loading/shaderpack_loading.hpp"

#include <experimental/filesystem>

#include "platform.hpp"
namespace fs = std::experimental::filesystem;

namespace nova {
    template <typename RenderEngine>
    settings &nova_renderer<RenderEngine>::get_settings() {
        return render_settings;
    }

    template <typename RenderEngine>
    void nova_renderer<RenderEngine>::execute_frame() {}

    template <typename RenderEngine>
    void nova_renderer<RenderEngine>::load_shaderpack(const std::string &shaderpack_name) {
        const auto shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));
    }

    template<typename RenderEngine>
    RenderEngine *nova_renderer<RenderEngine>::get_engine() {
        return engine.get();
    }
}