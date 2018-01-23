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
#include "gl_uniform_buffer.h"

namespace nova {
    class gl_shader_program;

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
        /*!
         * \brief Creates all the uniform buffers that we need, and loads the binding points from the config/data file
         */
        uniform_buffer_store();

        void register_all_buffers_with_shader(const gl_shader_program &shader) noexcept;

        void update();

        /*
         * Inherited from iconfig_listener
         */
        virtual void on_config_change(nlohmann::json &new_config);

        virtual void on_config_loaded(nlohmann::json &config);

        gl_uniform_buffer<per_frame_uniforms>& get_per_frame_uniforms();

    private:
        per_frame_uniforms per_frame_uniform_variables;

        gl_uniform_buffer<per_frame_uniforms> per_frame_uniforms_buffer;

        void update_per_frame_uniforms(nlohmann::json &config);
    };
}

#endif //RENDERER_UBO_MANAGER_H
