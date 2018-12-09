/*!
 * \brief Main class for Nova. This class exists as a singleton so it's always available
 *
 * \author ddubois
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_NOVA_RENDERER_H
#define NOVA_RENDERER_NOVA_RENDERER_H

#include <string>
#include <memory>
#include <ftl/atomic_counter.h>

#include "settings/nova_settings.hpp"
#include "render_engine/render_engine.hpp"

namespace nova {
    NOVA_EXCEPTION(already_initialized_exception);
    NOVA_EXCEPTION(uninitialized_exception);

    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
     */
    class nova_renderer {
    public:
        /*!
         * \brief Initializes the Nova Renderer
         */
        explicit nova_renderer(const settings_options &settings);

		~nova_renderer();

        /*!
         * \brief Loads the shaderpack with the given name
         *
         * This method will first try to load from the `shaderpacks/` folder (mimicking Optifine shaders). If the
         * shaderpack isn't found there, it'll try to load it from the `resourcepacks/` directory (mimicking Bedrock
         * shaders). If the shader can't be found at either place, a `nova::resource_not_found` exception will be thrown
         *
         * \param shaderpack_name The name of the shaderpack to load
         */
        void load_shaderpack(const std::string &shaderpack_name);

        /*!
         * \brief Executes a single frame
         */
        void execute_frame();

        nova_settings &get_settings();

        render_engine *get_engine();

        ftl::TaskScheduler &get_task_scheduler();

        static nova_renderer *initialize(const settings_options &settings) {
            return (instance = new nova_renderer(settings));
        }

        static nova_renderer *get_instance();

        static void deinitialize();

    private:
        nova_settings render_settings;
        std::unique_ptr<render_engine> engine;

        ftl::TaskScheduler task_scheduler;
		ftl::AtomicCounter frame_counter;

        static nova_renderer *instance;

		/*!
		 * \brief A thread for FTL's TaskScheduler::Run function to run in
		 * 
		 * FTL was not designed for Nova.
		 * 
		 * I need ftl::TaskScheduler::Run to be called when Nova initializes, and I need it to exit when Nova de-
		 * initializes, so that there's an active ftl::TaskScheduler throughout all of Nova's lifetime
		 * 
		 * Trust me, I need my ftl::TaskScheduler to do this
		 * 
		 * I can't simply call ftl::TaskScheduler::Run in nova_renderer::nova_renderer - ftl::TaskScheduler::Run cleans up 
		 * its thread pool
		 * 
		 * Thus, I call `ftl::TaskScheduler::Run` in a separate thread (this std::thread), passing in a lambda that waits for
		 * is_nova_running_counter to be equal to 0. It's set to 1 in nova_renderer's constructor, then set to 0 in 
		 * nova_renderer's destructor. In this way I ensure that there's a valid task scheduler for the entirety of Nova
		 */
		std::unique_ptr<std::thread> hack_to_make_ftl_usable_for_nova;
		ftl::AtomicCounter is_nova_running_counter;
    };
}  // namespace nova

#endif  // NOVA_RENDERER_NOVA_RENDERER_H
