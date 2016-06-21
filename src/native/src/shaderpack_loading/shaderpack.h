/*!
 * \author David
 * \date 20-Jun-16.
 */

#ifndef RENDERER_SHADERPACK_H
#define RENDERER_SHADERPACK_H

#include <string>
#include <unordered_map>

#include "../interfaces/ishader.h"
#include "../gl/objects/gl_shader_program.h"

/*!
 * \brief Represents a single shaderpack in all its glory
 */
class shaderpack {
public:
    /*!
     * \brief Loads the shaderpack with the given name
     *
     * If the shaderpack name ends in ".zip", it's loaded as a zip file. If the shaderpack does not, then it's added as
     * a folder. DO NOT call this method, except with one of those two cases. It WILL break, and you'll tear your hair
     * out.
     *
     * \param shaderpack_name The name of the shaderpack to load
     */
    shaderpack(std::string shaderpack_name);
private:
    const std::string SHADERPACK_FOLDER_NAME = "shaders";

    std::vector<const std::string> default_shader_names = {
            "gui"
    };

    std::unordered_map<std::string, ishader *> shaders;
    // shaderpack_config config;

    void load_zip_shaderpack(std::string shaderpack_name);

    void load_folder_shaderpack(std::string shaderpack_name);

    void load_program(const std::string shader_path, const std::string shader_name);

    void load_shader(const std::string &shader_name, gl_shader_program *program, GLenum shader_type) const;

    bool try_loading_shader(const std::string &shader_name, gl_shader_program *program, GLenum shader_type,
                            const std::string extension) const;
};


#endif //RENDERER_SHADERPACK_H
