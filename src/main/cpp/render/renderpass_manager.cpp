/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#include <vulkan/vulkan.hpp>
#include "renderpass_manager.h"
#include "vulkan/render_device.h"

namespace nova {
    renderpass_manager::renderpass_manager(std::shared_ptr<shaderpack> shaders) {
        // TODO: Inspect each shader, figure out which ones draw to which target, and build renderpasses based on that info
        // For now I'll just create the swapchain renderpass to make sure I hae the code

        create_final_renderpass();
    }

    void renderpass_manager::create_final_renderpass() {
        final_render_pass = renderpass_builder()
                .add_color_buffer()
                .build();

        LOG(INFO) << "Created final renderpass";
    }
}
