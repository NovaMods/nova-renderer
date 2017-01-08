/*!
 * \brief
 *
 * \author ddubois 
 * \date 14-Nov-16.
 */

#ifndef RENDERER_MESH_BUILDER_H
#define RENDERER_MESH_BUILDER_H

#include <unordered_map>
#include <mutex>
#include "../../../mc_interface/mc_objects.h"
#include "../mesh_definition.h"

namespace nova {
    /*!
     * \brief Builds meshes from various mesh definitions, and holds those meshes until the render thread can scoop
     * them up
     */
    class mesh_builder {
    public:

        /*!
         * \brief Builds some geometry for the GUI from the provided GUI data structure
         */
        void build_geometry(mc_gui_screen &screen);

        /*!
         * \brief Builds some geometry for the chunk described by the given data structure
         */
        void build_geometry(mc_chunk &chunk);

        /*!
         * \brief Builds geometry for the provided entity definition
         */
        void build_geometry(mc_entity &entity);

        mesh_definition &get_gui_mesh();

    private:
        mesh_definition gui_mesh;
        mc_gui_screen cur_gui_screen;
    };

    /*!
     * \brief Compares two mc_gui_screen objects, determining if they represent the same visual data
     *
     * I'l like to have had this function in the same header file as the mc_gui_screen struct. However, I need
     * mc_gui_screen to be a C struct so I can properly assign to it from Java. The compiler yelled at me about "You
     * can't compare structs" so I couldn't use the == operator and here we are.
     */
    bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2);

    /*!
     * \brief Determines whether or not the two given buttons are the same
     *
     * Two buttons are the same if they have the same position, size, and pressed status. If they can be drawn using
     * the exact same geometry and texture, then they are the same.
     *
     * \param button1 The first button to compare
     * \param button2 The second button to compare
     *
     * \return True if the buttons are the same, false otherwise
     */
    bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2);

    /*!
     * \brief Compares two C strings, taking special care to not crash if one or both of the strings is null
     *
     * \param text1 The first string to compare
     * \param text2  The second string to compare
     * \return True is the strings are different, false otherwise
     */
    bool are_different_strings(const char *text1, const char *text2);
}


#endif //RENDERER_MESH_BUILDER_H
