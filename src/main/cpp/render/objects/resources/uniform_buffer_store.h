/*!
 * \author David
 * \date 05-Jul-16.
 */

#ifndef RENDERER_UBO_MANAGER_H
#define RENDERER_UBO_MANAGER_H

#include <string>
#include <unordered_map>
#include <json.hpp>
#include "../../../data_loading/settings.h"
#include "uniform_buffer_definitions.h"
#include "uniform_buffer.h"

namespace nova {
    /*!
     * \brief Holds all the uniform buffers that Nova needs to use
     *
     * The Uniform Buffer Store is kinda nice because it registers itself as a configuration change listener,
     * meaning that it will receive updates whenever a new configuration is available. These updates are (currently)
     * uploaded by the "update" stage of the renderer
     *
     * Ideally, all transfers of data from the CPU to GPU will happen in a separate thread, and the render thread
     * will do nothing except dispatch rendering commands
     */
    class uniform_buffer_store : public iconfig_listener {
    public:
        uniform_buffer_store(std::shared_ptr<render_context> context);

		/*
		 * Buffer access
		 */

		std::shared_ptr<auto_buffer> get_per_model_buffer();

		void add_buffer(uniform_buffer& new_buffer);

		const bool is_buffer_known(std::string buffer_name) const;

		uniform_buffer& get_buffer(std::string buffer_name);

        /*
         * Inherited from iconfig_listener
         */
        virtual void on_config_change(nlohmann::json &new_config) override;

        virtual void on_config_loaded(nlohmann::json &config) override;

    private:
		std::unordered_map<std::string, uniform_buffer> buffers;

		std::shared_ptr<auto_buffer> per_model_resources_buffer;
    };
}

#endif //RENDERER_UBO_MANAGER_H
