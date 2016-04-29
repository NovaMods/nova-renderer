/*!
 * \brief Holds all the objects that we need to get from MC
 *
 * \author David
 * \date 27-Apr-16.
 */

#ifndef RENDERER_MC_OBJECTS_H
#define RENDERER_MC_OBJECTS_H

/*!
 * \brief Holds the information that comes from MC textures
 */
struct mc_texture {
    int width;
    int height;
    int num_components;
    unsigned char * texture_data;
    const char * name;
};

#endif //RENDERER_MC_OBJECTS_H
