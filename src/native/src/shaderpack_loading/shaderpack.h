/*!
 * \author David
 * \date 20-Jun-16.
 */

#ifndef RENDERER_SHADERPACK_H
#define RENDERER_SHADERPACK_H

#include <string>
#include <unordered_map>
#include <core/uniform_buffer_store.h>

#include "gl/objects/gl_shader_program.h"
#include "config/config.h"

/*!
 * \brief Represents a single shaderpack in all its glory
 */
class shaderpack : public iconfig_change_listener {
public:
    shaderpack();

    gl_shader_program & get_shader(std::string shader_name);

    /**
     * iconfig_change_listener methods
     */

    void on_config_change(nlohmann::json & new_config);

    void link_up_uniform_buffers(uniform_buffer_store &ubo_store);

private:
    const std::string SHADERPACK_FOLDER_NAME = "shaders";

    std::vector<std::string> default_shader_names;

    std::unordered_map<std::string, gl_shader_program> shaders;

    std::string name;

    void load_zip_shaderpack(std::string shaderpack_name);

    void load_folder_shaderpack(std::string shaderpack_name);

    void load_program(const std::string shader_path, const std::string shader_name);

    void load_shader(const std::string &shader_name, gl_shader_program & program, GLenum shader_type) const;

    bool try_loading_shader(const std::string &shader_name, gl_shader_program & program, GLenum shader_type,
                            const std::string extension) const;

    /*!
     * \brief Loads the shaderpack with the given name
     *
     * If the shaderpack name ends in ".zip", it's loaded as a zip file. If the shaderpack does not, then it's added as
     * a folder. DO NOT call this method, except with one of those two cases. It WILL break, and you'll tear your hair
     * out.
     */
    void load_shaderpack(const std::string &shaderpack_name);
};


#endif //RENDERER_SHADERPACK_H
