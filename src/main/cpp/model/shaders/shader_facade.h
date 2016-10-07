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
#include "gl_shader_program.h"
#include "view/geometry_cache/render_object.h"

namespace nova {
    namespace model {
        /*!
         * \brief Holds all the filters that a shader might need
         */
        class shader_tree {
        public:
            std::string shader_name;

            shader_tree(std::string name);
            shader_tree(std::string name, std::initializer_list<shader_tree> children);

            void calculate_filters(std::unordered_map<std::string, std::function<bool(render_object)>> filters, std::vector<std::string> loaded_shaders);

            std::function<bool(render_object)> get_filter_function();

            /*!
             * \brief Performs a depth-first traversal of the tree, funning the provided function on each element in the
             * tree
             *
             * \param f The function to run on the tree
             */
            void foreach_df(std::function<void(shader_tree&)> f);

        private:
            std::vector<std::function<bool(model::render_object)>> filters;

            std::vector<shader_tree> children;
        };

        /*!
         * \brief Provides an interface to interact with shaders
         *
         * This includes functionality like getting the filters for a shader, getting the framebuffer textures for a shader,
         * and all sorts of other fun stuff
         */
        class shader_facade {
        public:
            void operator=(std::unordered_map<std::string, gl_shader_program>&& shaders);

            gl_shader_program& operator[](std::string key);

            std::unordered_map<std::string, gl_shader_program>& get_loaded_shaders();
        private:
            /*!
             * \brief Defines which shaders to use if a given shader is not present
             */
            static shader_tree gbuffers_shaders;

            std::unordered_map<std::string, std::function<bool(render_object)>> filters;
            std::unordered_map<std::string, gl_shader_program> loaded_shaders;

            /*!
             * \brief Creates the filters to get the geometry that the current shaderpack needs
             */
            void build_filters();
        };
    }
}

#endif //RENDERER_SHADER_INTERFACE_H
