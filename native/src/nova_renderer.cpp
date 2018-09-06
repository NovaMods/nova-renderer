/*!
 * \author ddubois 
 * \date 03-Sep-18.
 */

#include "nova_renderer.hpp"

#include "platform.hpp"
#include "util/logger.hpp"

#if SUPPORT_DX12
#include "render_engine/dx12/dx12_render_engine.hpp"
#elif SUPPORT_VULKAN
#include "render_engine/vulkan/vulkan_render_engine.hpp"
#endif

namespace nova {
    nova_renderer* nova_renderer::instance;


    nova_renderer::nova_renderer() : render_settings("config/config.toml") {
#if SUPPORT_DX12
        engine = std::make_unique<dx12_render_engine>(render_settings);

#elif SUPPORT_VULKAN
        engine = std::make_unique<vulkan_render_engine>(render_settings);
#endif

        // TODO: Get window size from config
        engine->open_window(200, 200);
        frame_index = engine->get_current_swapchain_index();


    }

    settings &nova_renderer::get_settings() {
        return render_settings;
    }

    void nova_renderer::execute_frame() {
        // Transition the swapchain image from being presentable to being writable from a shader
        std::unique_ptr<command_buffer_base> buffer = engine->allocate_command_buffer(command_buffer_type::GENERIC);
        auto* swapchain_image_command_buffer = dynamic_cast<graphics_command_buffer_base *>(buffer.get());

        swapchain_image_command_buffer->reset();

        image_barrier_data swapchain_image_to_shader_writable = {};
        swapchain_image_to_shader_writable.resource_to_barrier = engine->get_current_swapchain_image(frame_index);
        swapchain_image_to_shader_writable.initial_layout = image_layout::PRESENT;
        swapchain_image_to_shader_writable.final_layout = image_layout::RENDER_TARGET;

        std::vector<image_barrier_data> to_shader_barriers;
        to_shader_barriers.push_back(swapchain_image_to_shader_writable);
        swapchain_image_command_buffer->resource_barrier(stage_flags::COLOR_ATTACHMENT_WRITE, stage_flags::COLOR_ATTACHMENT_WRITE,
                                                         {}, {}, to_shader_barriers);

        std::shared_ptr<iframebuffer> backbuffer_framebuffer = engine->get_current_swapchain_framebuffer(frame_index);
        swapchain_image_command_buffer->set_render_target(backbuffer_framebuffer.get());

        glm::vec4 clear_color(0, 0.2f, 0.4f, 1.0f);
        swapchain_image_command_buffer->clear_render_target(backbuffer_framebuffer.get(), clear_color);

        image_barrier_data swapchain_image_to_presentable = {};
        swapchain_image_to_presentable.resource_to_barrier = engine->get_current_swapchain_image(frame_index);
        swapchain_image_to_presentable.initial_layout = image_layout::RENDER_TARGET;
        swapchain_image_to_presentable.final_layout = image_layout::PRESENT;

        std::vector<image_barrier_data> to_presentable_barriers;
        to_presentable_barriers.push_back(swapchain_image_to_presentable);
        swapchain_image_command_buffer->resource_barrier(stage_flags::COLOR_ATTACHMENT_WRITE, stage_flags::COLOR_ATTACHMENT_WRITE,
                                                         {}, {}, to_presentable_barriers);

        swapchain_image_command_buffer->end_recording();

        std::vector<command_buffer_base*> command_buffers;
        command_buffers.push_back(swapchain_image_command_buffer);
        engine->execute_command_buffers(command_buffers);

        engine->present_swapchain_image();

        swapchain_image_command_buffer->wait_until_completion();

        engine->free_command_buffer(std::move(buffer));

        frame_index++;
        if(frame_index >= FRAME_BUFFER_COUNT) {
            frame_index = 0;
        }
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
        delete instance;
    }

    enki::TaskScheduler &nova_renderer::get_task_scheduler() {
        return task_scheduler;
    }
}
