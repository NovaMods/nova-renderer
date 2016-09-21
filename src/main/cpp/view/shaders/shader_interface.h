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
#include "model/gl/gl_shader_program.h"
#include "model/render_object.h"

namespace nova {
    namespace view {
        /*!
         * \brief Holds all the filters that a shader might need
         */
        class shader_tree {
        public:
            std::string shader_name;

            shader_tree(std::string name);
            shader_tree(std::string name, std::initializer_list<shader_tree> children);

            void calculate_filters(std::unordered_map<std::string, std::function<bool(model::render_object)>> filters, std::vector<std::string> loaded_shaders);

            std::function<bool(model::render_object)> get_filter_function();

            template<typename Func>
            void map(Func f) {
                f(*this);
                for(auto& child : children) {
                    child.map(f);
                }
            }

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
        class shader_interface {
        private:
            /*!
             * \brief Defines which shaders to use if a given shader is not present
             */
            static shader_tree gbuffers_shaders;

            std::unordered_map<std::string, std::function<bool(model::render_object)>> filters;
            std::unordered_map<std::string, model::gl_shader_program&> loaded_shaders;

            /*!
             * \brief Creates the filters to get the geometry that the current shaderpack needs
             */
            void build_filters();
        };
    }
}

#endif //RENDERER_SHADER_INTERFACE_H
