/*!
 * \author David
 * \date 05-Jul-16.
 */

#ifndef RENDERER_UBO_MANAGER_H
#define RENDERER_UBO_MANAGER_H

#include <string>
#include <unordered_map>
#include <json.hpp>
#include <gl/models/gl_shader_program.h>

#include "config/config.h"
#include "gl/models/gl_uniform_buffer.h"
#include "core/shaders/uniform_buffer_definitions.h"

class uniform_buffer_store : public iconfig_listener {
public:
    /*!
     * \brief Creates all the uniform buffers that we need, and loads the binding points from the config/data file
     */
    uniform_buffer_store();

    gl_uniform_buffer & operator[](std::string name);

    void register_all_buffers_with_shader(gl_shader_program& shader) const noexcept;

    /*
     * Inherited from iconfig_listener
     */
    virtual void on_config_change(nlohmann::json& new_config);

    virtual void on_config_loaded(nlohmann::json& config);
private:
    std::unordered_map<std::string, gl_uniform_buffer> buffers;

    camera_data cam_data;

    void upload_data();

    void set_bind_points(nlohmann::json& config);

    void create_ubos();
};


#endif //RENDERER_UBO_MANAGER_H
