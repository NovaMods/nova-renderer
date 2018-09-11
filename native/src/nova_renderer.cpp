/*!
 * \author ddubois 
 * \date 03-Sep-18.
 */

#include <future>

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


    nova_renderer::nova_renderer(nova_settings &settings) : render_settings(settings) {
#if SUPPORT_DX12
        engine = std::make_unique<dx12_render_engine>(render_settings);

#elif SUPPORT_VULKAN
        engine = std::make_unique<vulkan_render_engine>(render_settings);
#endif

        // TODO: Get window size from config
        engine->open_window(200, 200);
        NOVA_LOG(DEBUG) << "Opened window";
    }

    nova_settings &nova_renderer::get_settings() {
        return render_settings;
    }

    void nova_renderer::execute_frame() {
        task_scheduler.Run(200, [](ftl::TaskScheduler* task_scheduler, void* arg) {
            reinterpret_cast<render_engine*>(arg)->render_frame();
        },
        engine.get());
    }

    void nova_renderer::load_shaderpack(const std::string &shaderpack_name) {
        struct load_shaderpack_args {
            const std::string& shaderpack_name;
            std::promise<shaderpack_data> output;
        };

        auto data = load_shaderpack_args{ shaderpack_name, std::promise<shaderpack_data>() };

        task_scheduler.Run(200, [](ftl::TaskScheduler* task_scheduler, void* arg) {
            auto* args = reinterpret_cast<load_shaderpack_args*>(arg);

            const auto shaderpack_data = load_shaderpack_data(fs::path(args->shaderpack_name), *task_scheduler);
            args->output.set_value(shaderpack_data);
        }, &data);
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

    ftl::TaskScheduler &nova_renderer::get_task_scheduler() {
        return task_scheduler;
    }
}
