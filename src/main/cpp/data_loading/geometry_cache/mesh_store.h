/*!
 * \brief
 *
 * \author ddubois 
 * \date 27-Sep-16.
 */

#ifndef RENDERER_GEOMETRY_CACHE_H
#define RENDERER_GEOMETRY_CACHE_H

#include <vector>
#include <mutex>
#include <functional>
#include <unordered_map>
#include "../../render/objects/render_object.h"
#include "builders/mesh_builder.h"
#include "../../render/objects/shaders/geometry_filter.h"
#include "../../render/objects/shaders/shaderpack.h"

namespace nova {
    /*!
         * \brief Provides access to the meshes that Nova will want to deal with
         *
         * The primary way it does this is by allowing the user to specify
         */
    class mesh_store {
    public:
        mesh_store();

        void add_gui_geometry(mc_gui_screen& screen);

        /*!
         * \brief Sets the shaderpack reference to the given shaderpack
         *
         * This object needs to keep a reference to the current shaderpack so that it can apply all the right filters
         * when a new piece of geometry is loaded, determining which shader should render the bit of geometry
         *
         * \param new_shaderpack The new shaderpack to use for retrieving filters from
         */
        void set_shaderpack(shaderpack& new_shaderpack);

        /*!
         * \brief Retrieves the list of meshes that the shader with the provided name should render
         *
         * \param shader_name The name of the shader to get meshes for
         * \return All the meshes that should be rendered with the given name
         */
        std::vector<render_object *> get_meshes_for_shader(std::string shader_name);

    private:
        std::unordered_map<std::string, std::vector<render_object*>> renderables_grouped_by_shader;

        mc_gui_screen cur_gui_screen;
        shaderpack* shaders;

        /*!
         * \brief Puts the provided render object into all the proper lists, so that it can be rendered by the right
         * shader
         *
         * \param object The render_object to sort
         */
        void sort_render_object(render_object &object);

        /*!
         * \brief Removes all the render_objects from the lists of render_objects that match the given fitler funciton
         *
         * The idea here is that when things like the GUI screen change, or when a chunk changes, old geometry will need
         * to be removed. This should accomplish that.
         *
         * \param filter The function to use to decide which (if any) objects to remove
         */
        void remove_render_objects(std::function<bool(render_object &)> fitler);

        /*!
         * \brief Determines if a given obejct matches a given geometry filter
         *
         * \param object The object to check the matching of
         * \param filter The filter to check the object agains
         * \return True if the object matches, false if not
         */
        bool matches_filter(render_object &object, geometry_filter &filter);
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
};

#endif //RENDERER_GEOMETRY_CACHE_H
