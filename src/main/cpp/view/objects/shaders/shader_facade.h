/*!
 * \brief
 *
 * \author ddubois 
 * \date 20-Sep-16.
 */

#ifndef RENDERER_SHADER_INTERFACE_H
#define RENDERER_SHADER_INTERFACE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <initializer_list>
#include <utils/export.h>
#include <mutex>
#include <optional.hpp>

#include "view/objects/render_object.h"
#include "gl_shader_program.h"

namespace nova {
    namespace view {
        /*!
         * \brief Holds a bunch of information to filter geometry
         *
         * At first I used lambdas. Lambdas are very powerful: they can do anything. Anything!
         *
         * The lambdas were super hard to debug, though. I stored references to them in some kinda of structure, and
         * those references were really bad at pointing back to the line of code where the lambda was actually
         * constructed. I couldn't see what was actually happening. this would be fine, except that it wasn't and I
         * kept getting crashes that were super hard to debug.
         *
         * Now there's a simple data structure. Data is much easier to debug. No more stupid crashes for me!
         *
         * (i hope)
         *
         * There's a precidence for these filters:
         *  - If one of the optional values isn't set, then it's ignored.
         *  - Block, Entity, Sky, Particle, Geometry Type, and Name filters are applied first. They're all OR'd together
         *  - Transparent, Cutout, Damaged, and Emissive are applied next, OR'd together
         *
         * This means you can have a filter which matches transparent blocks and entities, or all cutout blocks of a
         * certain geometry type, but you can't have a filter that matches cutout blocks and emissive particles. If you
         * need that, too bad.
         */
        struct geometry_filter {
            // Using a third-party library because CMake doesn't (yet) support C++17 (probably, updating tool is hard)
            std::experimental::optional<bool> should_be_block;
            std::experimental::optional<bool> should_be_selection_box;
            std::experimental::optional<bool> should_be_entity;
            std::experimental::optional<bool> should_be_particle;
            std::experimental::optional<bool> should_be_sky_object;

            std::vector<geometry_type> geometry_types;

            std::vector<std::string>> names;
            std::vector<std::string>> name_parts;

            std::experimental::optional<bool> should_be_transparent;
            std::experimental::optional<bool> should_be_cutout;
            std::experimental::optional<bool> should_be_emissive;
            std::experimental::optional<bool> should_be_damaged;

            /*!
             * \brief Tells this filter that it should accept blocks
             */
            geometry_filter& block();

            /*!
             * \brief Tells this filter that it should reject blocks
             */
            geometry_filter& not_block();

            /*!
             * \brief Tells this filter that it should accept the selection box
             */
            geometry_filter& selection_box();

            /*!
             * \brief Tells this filter that it should reject the selection box
             */
            geometry_filter& not_selection_box();

            /*!
             * \brief Tells this filter that it should accept entites
             */
            geometry_filter& entity();

            /*!
             * \brief Tells this filter that it should reject entities
             */
            geometry_filter& not_entity();

            /*!
             * \brief Tells this filter that it should accept particles
             */
            geometry_filter& particle();

            /*!
             * \brief Tells this filter that it should reject particles
             */
            geometry_filter& not_particle();

            /*!
             * \brief Tells this filter that it should accept sky objects
             */
            geometry_filter& sky_object();

            /*!
             * \brief Tells this filter that it should reject sky objects
             */
            geometry_filter& not_sky_object();

            /*!
             * \brief Tells this filter to accept geometry of the given type
             *
             * Allowed geometry types are added to a list, and geometry which matches one of the values on that list is
             * accepted
             *
             * \param type The geometry_type to accept
             */
            geometry_filter& add_geometry_type(geometry_type type);

            /*!
             * \brief Tell this filter to accept geometry with the given name
             *
             * Allowed geometry names are added to a list, and geometry which has a name on the list is accepted
             *
             * \param name The name of the geometry to accept
             */
            geometry_filter& add_name(std::string name);

            /*!
             * \brief Tells this filter to accept geometry whose name contains the given string
             *
             * Geometry name parts are added to a list, and geometry whose name contains one of the name parts on the
             * list is accepted
             *
             * \param name_part The name part to accept
             */
            geometry_filter& add_name_part(std::string name_part);

            /*!
             * \brief Tells this filter that it should accept transparent geometry
             */
            geometry_filter& transparent();

            /*!
             * \brief Tells this filter that it should reject transparent geometry
             */
            geometry_filter& not_transparent();

            /*!
             * \brief Tells this filter that it should accept cutout geometry
             */
            geometry_filter& cutout();

            /*!
             * \brief Tells this filter that it should reject curtout geometry
             */
            geometry_filter& not_cutout();

            /*!
             * \brief Tells this filter that it should accept emissive geometry
             */
            geometry_filter& emissive();

            /*!
             * \brief Tells this filter that it should reject emissive geometry
             */
            geometry_filter& not_emissive();

            /*!
             * \brief Tells this filter that it should accept damaged geometry
             */
            geometry_filter& damaged();

            /*!
             * \brief Tells this filter that it should reject damaged geometry
             */
            geometry_filter& not_damaged();

            /*!
             * \brief Creates a copy of this geometry filter so that there won't be a stupid reference hanging out
             */
            geometry_filter build();
        };

        /*!
         * \brief Holds all the filters that a shader might need
         */
        class NOVA_API shader_tree_node {
        public:
            std::string shader_name;

            // If enabled is true, then the shader that corresponds to this shader_tree_node has been loaded. This
            // means that the search for filters that apply to a given shader should not include this shader_tree_node
            // or its children
            bool enabled;

            shader_tree_node(std::string name, geometry_filter filter);

            shader_tree_node(std::string name, geometry_filter filter, std::vector<shader_tree_node> children);

            void calculate_filters(std::vector<std::string> loaded_shaders);

            geometry_filter& get_filter();

            /*!
             * \brief Performs a depth-first traversal of the tree, funning the provided function on each element in the
             * tree
             *
             * \param f The function to run on the tree
             */
            void foreach_df(std::function<void(shader_tree_node &)> f);

        private:
            std::vector<geometry_filter> filters; // Instance variable so it doesn't get cleaned up
            geometry_filter filter;

            std::vector<shader_tree_node> children;
        };

        /*!
         * \brief Provides an interface to interact with shaders
         *
         * This includes functionality like getting the filters for a shader, getting the framebuffer textures for a
         * shader, and all sorts of other fun stuff
         */
        class shader_facade {
        public:
            void set_shader_definitions(std::unordered_map<std::string, model::shader_definition> &definitions);

            gl_shader_program &operator[](std::string key);

            std::unordered_map<std::string, gl_shader_program> &get_loaded_shaders();

            /*!
             * \brief Sends the shaders to the GPU
			 *
			 * The idea here is that the shaders are loaded in the main thread, then sent to the GPU in the render
			 * thread. This lets me have an OpenGL 4.5 context for the render, while letting Minecraft's 2.1 context 
			 * persist
             */
            void upload_shaders();

        private:
            /*!
             * \brief Defines which shaders to use if a given shader is not present
             */
            static shader_tree_node gbuffers_shaders;

            // The shaders and filters are two separate data structures because they're used differently. Here's how I
            // envision their use:
            //      - Shaders are loaded. Their source code gets stuck in the shader_definitions map
            //      - Based on which shaders are loaded,
            std::unordered_map<std::string, model::shader_definition> shader_definitions;
            std::unordered_map<std::string, gl_shader_program> loaded_shaders;
            std::unordered_map<std::string, std::function<bool(const render_object &)>> filters;

            //! \brief Protects reading the shader definitions
            std::mutex shaderpack_reading_guard;

            /*!
             * \brief Creates the filters to get the geometry that the current shaderpack needs
             */
            void build_filters();
        };
    }
}


#endif //RENDERER_SHADER_INTERFACE_H
