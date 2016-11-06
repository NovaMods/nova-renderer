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
#include "gl_shader_program.h"
#include "model/geometry_cache/render_object.h"

namespace nova {
    namespace model {
        /*!
         * \brief Holds all the filters that a shader might need
         */
        class NOVA_API shader_tree_node {
        public:
            std::string shader_name;

            shader_tree_node(std::string name, std::function<bool(const render_object&)> filter);
            shader_tree_node(std::string name, std::function<bool(const render_object&)> filter, std::vector<shader_tree_node> children);

            void calculate_filters(std::vector<std::string> loaded_shaders);

            std::function<bool(const render_object&)> get_filter_function();

            /*!
             * \brief Performs a depth-first traversal of the tree, funning the provided function on each element in the
             * tree
             *
             * \param f The function to run on the tree
             */
            void foreach_df(std::function<void(shader_tree_node&)> f);

        private:
            std::vector<std::function<bool(const render_object&)>> filters; // Instance variable so it doesn't get cleaned up
            std::function<bool(const render_object&)> filter;

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
            void set_shader_definitions(std::unordered_map<std::string, shader_definition>& definitions);

            gl_shader_program& operator[](std::string key);

            std::unordered_map<std::string, gl_shader_program>& get_loaded_shaders();

            /*!
             * \brief
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
            std::unordered_map<std::string, shader_definition> shader_definitions;
            std::unordered_map<std::string, gl_shader_program> loaded_shaders;
            std::unordered_map<std::string, std::function<bool(const render_object&)>> filters;

            /*!
             * \brief Creates the filters to get the geometry that the current shaderpack needs
             */
            void build_filters();
        };
    }
}

#endif //RENDERER_SHADER_INTERFACE_H
