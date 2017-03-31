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
#include "../render/objects/render_object.h"
#include "../render/objects/shaders/geometry_filter.h"
#include "../render/objects/shaders/shaderpack.h"
#include "../mc_interface/mc_gui_objects.h"
#include "../mc_interface/mc_objects.h"

namespace nova {
    /*!
         * \brief Provides access to the meshes that Nova will want to deal with
         *
         * The primary way it does this is by allowing the user to specify
         */
    class mesh_store {
    public:
        void add_gui_buffers(mc_gui_send_buffer_command* command);

        /*!
         * \brief Adds a chunk to the mesh store if the chunk doesn't exist, or replaces the chunks if it does exist
         *
         * \param chunk The chunk to add or update
         */
        void add_or_update_chunk(mc_chunk& chunk);

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
        std::vector<render_object>& get_meshes_for_shader(std::string shader_name);

        /*!
        * \brief Removes all gui render objects and thereby deletes all the buffers
        */
        void remove_gui_render_objects();

        /*!
         * \brief Registers a simple model for later use
         *
         * \param model_name The name of the model
         * \param model The model
         */
        void register_simple_model(std::string model_name, mc_simple_model& model);

        /*!
         * \brief Deregisters a model from Nova
         *
         * \param model_name The name of the model to remove
         */
        void deregister_model(std::string model_name);

    private:
        std::unordered_map<std::string, std::vector<render_object>> renderables_grouped_by_shader;
        std::unordered_map<std::string, mesh_definition> simple_models;
        std::vector<mc_chunk> all_chunks;

        shaderpack* shaders;

        /*!
         * \brief Puts the provided render object into all the proper lists, so that it can be rendered by the right
         * shader
         *
         * \param object The render_object to sort
         */
        void sort_render_object(render_object& object);


        /*!
         * \brief Removes all the render_objects from the lists of render_objects that match the given fitler funciton
         *
         * The idea here is that when things like the GUI screen change, or when a chunk changes, old geometry will need
         * to be removed. This should accomplish that.
         *
         * \param filter The function to use to decide which (if any) objects to remove
         */
        void remove_render_objects(std::function<bool(render_object&)> fitler);

        /*!
         * \brief Creates geometry for the given chunk and adds it to the list of geometry to render
         * \param chunk The chunk to generate geometry for
         */
        void generate_chunk_geometry(mc_chunk &chunk);
    };

};

#endif //RENDERER_GEOMETRY_CACHE_H
