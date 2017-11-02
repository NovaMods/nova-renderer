/*!
 * \brief Contains struct to represent .material files
 * \author ddubois 
 * \date 01-Nov-17.
 */

#ifndef RENDERER_MATERIALS_H
#define RENDERER_MATERIALS_H

#include <string>

namespace nova {
    enum class texture_filter {
        texel_aa,
        bilinear,
        point
    };

    enum class texture_wrap_mode {
        repeat,
        clamp
    };

    /*!
     * \brief Defines a sampler to use for a texture
     *
     * At the time of writing I'm not sure how this is correllated with a texture, but all well
     */
    struct sampler_state {
        int sampler_index;
        texture_filter filter;
        texture_wrap_mode wrap_mode;
    };

    /*!
     * \brief The kind of data in a vertex attribute
     */
    enum class vertex_field {
        position,
        color,
        main_uv,
        lightmap_uv,
        normal,
        tangent,
        virtual_texture_id,
        empty
    };

    /*!
     * \brief A texture definition in a material file
     *
     * This class simple describes where to get the texture data from.
     */
    class texture {
        /*!
         * \brief The name of the texture
         *
         * If the texture name starts with `atlas` then the texture is one of the atlases. Nova sticks all the textures
         * it can into the virtual texture atlas, so it doesn't really care what atlas you request.
         */
        std::string texture_name;
    };
}

#endif //RENDERER_MATERIALS_H
