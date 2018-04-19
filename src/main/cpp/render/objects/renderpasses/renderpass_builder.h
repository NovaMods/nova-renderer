/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#ifndef RENDERER_RENDER_PASS_MANAGER_H
#define RENDERER_RENDER_PASS_MANAGER_H

#include <glm/glm.hpp>
#include "../../vulkan/render_context.h"
#include "../../../data_loading/loaders/shader_loading.h"
#include "../resources/texture_manager.h"

namespace nova {
    struct pass_vulkan_information {
        vk::RenderPass renderpass;
        vk::Framebuffer frameBuffer;
        vk::Extent2D framebuffer_size;
        uint32_t num_attachments;
        std::vector<std::string> texture_outputs;
        std::experimental::optional<std::string> depth_output;
    };

    std::unordered_map<std::string, pass_vulkan_information> make_passes(const shaderpack_data& data, texture_manager& textures,
                                                                         std::shared_ptr<render_context> context);
}

#endif //RENDERER_RENDER_PASS_MANAGER_H
