/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#ifndef RENDERER_RENDER_PASS_MANAGER_H
#define RENDERER_RENDER_PASS_MANAGER_H

#include <glm/glm.hpp>
#include "../../vulkan/render_context.h"
#include "../../../data_loading/loaders/shader_loading.h"
#include "../textures/texture_manager.h"

namespace nova {
    std::unordered_map<std::string, std::pair<vk::RenderPass, vk::Framebuffer>> make_passes(const shaderpack_data& data,
                                                                                            std::shared_ptr<texture_manager> textures,
                                                                                            std::shared_ptr<render_context> context);
}

#endif //RENDERER_RENDER_PASS_MANAGER_H
