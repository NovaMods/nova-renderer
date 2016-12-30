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
#include "geometry_filter.h"

namespace nova {
    namespace view {
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
