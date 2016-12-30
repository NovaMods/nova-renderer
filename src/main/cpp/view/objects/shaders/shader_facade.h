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
         * \brief Represents a shaderpack in all of its glory
         */
        class shaderpack {
        public:
            /*!
             * \brief Loads the shaderpack with the given name
             *
             * This is kinda gross because the shaderpack loading logic is all 
             * in the data_loading module... thing is, there's no longer any
             * reason to keep that running in a separate thread, so why not put
             * it here?
             *
             * \param shaderpack_name The name of the shaderpcack to load
             *
             */
            shaderpack(std::string shaderpack_name);

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

            /*!
             * \brief Holds the functions used to modify a geometry_filter 
             *
             * The idea here is that a list of filters will be provided in the shaders.json file. Each of those filters 
             * corresponds to either a function in this map, or a request to add a specific geometry_type, name, or
             * name part to the filter.
             */
            static unordered_map<std::string, std::function<void(geometry_filter)>> filter_modifying_functions;

            /*!
             * \brief Holds the default filters, shader heirarchy, and shader 
             * order
             *
             * One super important thing: shaders.json is only relevant for 
             * drawing the gbuffers phase. It doesn't control fullscreen passes
             * because the fullscreen passes are very well defined: all 
             * composites are rendered in alphabetical order, then final is 
             * rendered. All fullscreen passes use the same geometry, so 
             * there's no need at all to define per-shader filters
             */
            static nlohmann::json default_shaders_json;

            static cool default_shaders_json_loaded;
 
            std::unordered_map<std::string, gl_shader_program> loaded_shaders;
            std::unordered_map<std::string, geometry_filter> filters;

            //! \brief Protects reading the shader definitions
            std::mutex shaderpack_reading_guard;

            /*!
             * \brief Loads the default shaders.json file from the config 
             * folder so that shaderpacks that don't provide a shader.json file
             * can still work
             */
            void load_default_shaders_json();

            /*!
             * \brief Creates the filters to get the geometry that the current shaderpack needs
             */
            void build_filters();
        };
    }
}

#endif //RENDERER_SHADER_INTERFACE_H.
