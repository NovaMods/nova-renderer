/*!
 * \author ddubois 
 * \date 03-Sep-18.
 */

#include "nova_renderer.hpp"

#include "platform.hpp"

#if SUPPORT_DX12
#include "render_engine/dx12/dx12_render_engine.hpp"
#elif SUPPORT_VULKAN
#include "render_engine/vulkan/vulkan_render_engine.hpp"
#endif

namespace nova {
    nova_renderer* nova_renderer::instance;


    nova_renderer::nova_renderer()
#if SUPPORT_DX12
    : render_settings(settings_options{"directx-12"}) {
        engine = std::make_unique<dx12_render_engine>(render_settings);

#elif SUPPORT_VULKAN
    : render_settings(settings_options{"vulkan-1.1"}) {
        engine = std::make_unique<vulkan_render_engine>(render_settings);
#endif
    }

    settings &nova_renderer::get_settings() {
        return render_settings;
    }

    void nova_renderer::execute_frame() {
        // Transition the swapchain image from being presentable to being writable from a shader
        std::unique_ptr<command_buffer_base> buffer = engine->allocate_command_buffer(command_buffer_type::GENERIC);
        auto* swapchain_image_command_buffer = dynamic_cast<graphics_command_buffer_base *>(buffer.get());

        swapchain_image_command_buffer->reset();

        resource_barrier_data swapchain_image_to_shader_writable = {};
        swapchain_image_to_shader_writable.resource_to_barrier = engine->get_current_swapchain_image();
        swapchain_image_to_shader_writable.initial_layout = resource_layout::PRESENT;
        swapchain_image_to_shader_writable.final_layout = resource_layout::RENDER_TARGET;

        std::vector<resource_barrier_data> to_shader_barriers;
        to_shader_barriers.push_back(swapchain_image_to_shader_writable);
        swapchain_image_command_buffer->resource_barrier(to_shader_barriers);

        std::shared_ptr<iframebuffer> backbuffer_framebuffer = engine->get_current_swapchain_framebuffer();
        swapchain_image_command_buffer->set_render_target(backbuffer_framebuffer.get());

        glm::vec4 clear_color(0, 0.2f, 0.4f, 1.0f);
        swapchain_image_command_buffer->clear_render_target(backbuffer_framebuffer.get(), clear_color);

        resource_barrier_data swapchain_image_to_presentable = {};
        swapchain_image_to_presentable.resource_to_barrier = engine->get_current_swapchain_image();
        swapchain_image_to_presentable.initial_layout = resource_layout::RENDER_TARGET;
        swapchain_image_to_presentable.final_layout = resource_layout::PRESENT;

        std::vector<resource_barrier_data> to_presentable_barriers;
        to_presentable_barriers.push_back(swapchain_image_to_presentable);
        swapchain_image_command_buffer->resource_barrier(to_presentable_barriers);

        swapchain_image_command_buffer->end_recording();

        std::vector<command_buffer_base*> command_buffers;
        command_buffers.push_back(swapchain_image_command_buffer);
        engine->execute_command_buffers(command_buffers);

        engine->present_swapchain_image();

        engine->free_command_buffer(std::move(buffer));
    }

    void nova_renderer::load_shaderpack(const std::string &shaderpack_name) {
        const auto shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));
    }

    render_engine* nova_renderer::get_engine() {
        return engine.get();
    }

    nova_renderer* nova_renderer::get_instance() {
        return instance;
    }

    void nova_renderer::deinitialize() {

    }
}
