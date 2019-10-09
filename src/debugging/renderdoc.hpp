/*!
 * \author ddubois
 * \date 24-Jan-19.
 */

#ifndef NOVA_RENDERER_RENDERDOC_HPP
#define NOVA_RENDERER_RENDERDOC_HPP

#include "nova_renderer/renderdoc_app.h"
#include "nova_renderer/util/result.hpp"

namespace nova::renderer {
    Result<RENDERDOC_API_1_3_0*> load_renderdoc(const std::string& renderdoc_dll_path);
} // namespace nova::renderer

#endif // NOVA_RENDERER_RENDERDOC_HPP
