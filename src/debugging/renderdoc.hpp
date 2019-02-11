/*!
 * \author ddubois
 * \date 24-Jan-19.
 */

#ifndef NOVA_RENDERER_RENDERDOC_HPP
#define NOVA_RENDERER_RENDERDOC_HPP

#include <string>

#include "renderdoc_app.h"

namespace nova {
    RENDERDOC_API_1_3_0 *load_renderdoc(const std::string &renderdoc_dll_path);
} // namespace nova

#endif // NOVA_RENDERER_RENDERDOC_HPP
