/*!
 * \author ddubois
 * \date 03-Sep-18.
 */

#include <future>

#include "nova_renderer.hpp"

#include "platform.hpp"
#include "util/logger.hpp"
#include "glslang/MachineIndependent/Initialize.h"
#include "loading/shaderpack/shaderpack_loading.hpp"

#if _WIN32
#include "render_engine/dx12/dx12_render_engine.hpp"
#endif 

#include "render_engine/vulkan/vulkan_render_engine.hpp"

namespace nova {
    nova_renderer *nova_renderer::instance;

    nova_renderer::nova_renderer(const settings_options &settings) : 
		render_settings(settings), task_scheduler(16, ttl::empty_queue_behavior::YIELD) {

        switch(settings.api) {
        case graphics_api::dx12:
            #if _WIN32
            engine = std::make_unique<dx12_render_engine>(render_settings, &task_scheduler);
            break;
            #endif
        case graphics_api::vulkan:
            engine = std::make_unique<vulkan_render_engine>(render_settings, &task_scheduler);
        }

        NOVA_LOG(DEBUG) << "Opened window";
    }

	nova_settings &nova_renderer::get_settings() {
        return render_settings;
    }

    void nova_renderer::execute_frame() {
		/*frame_done_future.wait();
		frame_done_future.get();
		try {
			frame_done_future = task_scheduler.add_task([](ttl::task_scheduler* task_scheduler, render_engine* engine) { engine->render_frame(); }, engine.get());

		} catch(const std::exception& e) {
			NOVA_LOG(ERROR) << "Could not execute frame: " << e.what();
		}*/

		engine->render_frame();
    }

    void nova_renderer::load_shaderpack(const std::string &shaderpack_name) const {
        glslang::InitializeProcess();

        const shaderpack_data shaderpack_data = load_shaderpack_data(fs::path(shaderpack_name));

        engine->set_shaderpack(shaderpack_data);
		NOVA_LOG(INFO) << "Shaderpack " << shaderpack_name << " loaded successfully";
    }

    render_engine *nova_renderer::get_engine() const {
        return engine.get();
    }

    nova_renderer *nova_renderer::get_instance() {
        return instance;
    }

    void nova_renderer::deinitialize() {
        delete instance;
    }

	ttl::task_scheduler &nova_renderer::get_task_scheduler() {
        return task_scheduler;
    }
}  // namespace nova
