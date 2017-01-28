/*!
 * \author David
 * \date 29-Apr-16.
 */

#ifndef RENDERER_TEXTURE_RECEIVER_H
#define RENDERER_TEXTURE_RECEIVER_H

#include <string>
#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../../../mc_interface/mc_objects.h"
#include "texture2D.h"
#include "../../../utils/smart_enum.h"

namespace nova {
    /*!
     * \brief Holds all the textures that the Nova Renderer can deal with
     *
     * This class does a few things. I'm going to walk you through a couple usage scenarios because it's way easier for
     * me to explain things that way
     *
     * \par Loading a resource pack:
     * When MC loads a resource pack, the Java Nova code should reset the texture manager. That clears out all existing
     * textures, freeing up the VRAM and RAM they used. Next, the Nova Renderer loops through all the textures it cares
     * about, which is super gross because I have to hardcode the values it cares about but I don't know a better way to
     * do is yet, and gets each texture from the resource pack. It sends each texture to the texture manager by way of
     * nova_renderer#add_texture(mc_atlas_texture). Once all textures have been loaded, the Nova Renderer calls
     * nova_renderer#finalize_textures, which tells the texture manager (this thing) to stitch as many textures as
     * possible into a texture atlas and generate a mapping from texture place in the atlas to texture name, such that
     * someone can call texture_manager#get_texture_location(std::string) and get back a texture_location struct, which
     * has the GL ID of the requested texture, the minimum UV coordinates that refer to that texture, and the maximum
     * UV coordinates that refer to that texture. This is useful mostly when building chunk geometry, so I can assign
     * the right UV coordinates to each triangle.
     *
     * \par Rendering the world:
     * This class won't perform a lot of actions while rendering the world. Mostly I'll just be like "I need the terrain
     * texture" or "I really need the entity texture". I'm going to be using texture atlases as much as possible.
     * Anyway, I'll ask the texture manager for a certain texture atlas, and the texture manager will give it back to
     * me. Then, I can bind that texture and render my pants off.
     */
    class texture_manager {
    public:
        /*!
         * \brief Identifies which atlas a texture is
         *
         * | 0 | GUI |
| 1 | assets/minecraft/textures/gui/options_background.png |
| 2 | Font |
| 3 | Terrain Color |
| 4 | Terrain Normalmap |
| 5 | Terrain Data |
| 6 | Entities Color |
| 7 | Entities Normalmap |
| 8 | Entities Data |
| 9 | Items |
| 10 | World Data |
| 11 | Particles |
| 12 | Weather |
| 13 | Sky |
| 14 | assets/minecraft/textures/environment/end_sky.png |
| 15 | assets/minecraft/textures/environment/clouds.png |
         */
        SMART_ENUM(
                texture_type,
                gui,
                options_background,
                font,
                terrain_color,
                terrain_normalmap,
                terrain_data,
                entities_color,
                entities_normalmap,
                entities_data,
                items,
                world_data,
                particles,
                weather,
                sky,
                end_sky,
                clouds
        )

        /*!
         * \brief Tells you the min/max UV coordinates of a texture in an atlas
         *
         * The name of the texture in the atlas is used as a key in a hash map
         *
         * The exact atlas is not identified here. That's because I expect the caller to know what kind of texture they
         * have. if you're making the terrain, you know you need the terrain texture.
         */
        struct texture_location {
            glm::ivec2 min;     //!< The minimum UV coordinate of the requested texture in its atlas
            glm::ivec2 max;     //!< The maximum UV coordinate of the requested texture in its atlas
        };

        /*!
         * \brief Initializes the texture_manager. Doesn't do anything special.
         *
         * Note to self: Don't put any GL calls in the constructor. It's called before an OpenGL context is available
         */
        texture_manager();

        /*!
         * \brief De-allocates everything ths texture_manager uses
         *
         * The destructor calls #reset to de-allocate all OpenGL textures. All other memory should get cleaned up when it
         * does out of scope
         */
        ~texture_manager();

        /*!
         * \brief De-allocates all OpenGL textures and clears all data, making way for a new resource pack's textures
         */
        void reset();

        /*!
         * \brief Adds a texture to this resource manager
         *
         * The texture is not put into an atlas immediately. Rather, it is held in a staging area until #finalize_textures
         * is called. Then it's put into an atlas
         *
         * \param new_texture The new texture
         * \param data_type The type of texture we're dealing with
         */
        void add_texture(mc_atlas_texture &new_texture, texture_type data_type);

        /*!
         * \brief Adds the given texture location to the list of texture locations
         *
         * \param location The location to add
         */
        void add_texture_location(mc_texture_atlas_location &location);

        /*!
         * \brief Retrieves the texture location for a texture with a specific name
         *
         * \param texture_name The MC resource location of the texture to get the location of. This name should be the
         * exact MC name of the texture
         * \return The location of the requested texture
         */
        const texture_location &get_texture_location(const std::string &texture_name);

        /*!
         * \brief Returns a pointer to the specified atlas
         *
         * \param atlas The type of atlas to get
         * \param type The type of data that should be in the atlas
         * \return A pointer to the atlas texture
         */
        texture2D &get_texture_atlas(texture_type type);

        /*!
         * \brief Returns the maximum texture size supported by OpenGL on the current platform
         *
         * This size is used primarily as an upper bound for the size of the texture atlases Nova uses. Nova uses OpenGL
         * 4.3, which allows for a greater texture size than the OpenGL 2.1 that the Shaders Mod uses. This allows for
         * bigger texture atlases, which in turn means I don't have to bind textures as much.
         */
        int get_max_texture_size();

    private:
        std::map<texture_type, texture2D> atlases;

        /*!
         * \brief A map from the name of a texture according to Minecraft and the UV coordinates it takes up in its
         * texture atlas
         */
        std::map<std::string, texture_location> locations;

        int max_texture_size = -1;
    };
}

#endif //RENDERER_TEXTURE_RECEIVER_H
