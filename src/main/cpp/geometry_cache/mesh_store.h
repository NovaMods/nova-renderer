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
#include <queue>
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
        void add_geometry_for_filter(std::string filter_name, mc_basic_render_object& chunk);

        /*!
         * \brief Retrieves the list of meshes that the shader with the provided name should render
         *
         * \param shader_name The name of the shader to get meshes for
         * \return All the meshes that should be rendered with the given name
         */
        std::vector<render_object>& get_meshes_for_shader(std::string shader_name);

        /*!
         * \brief Takes geometry that's been added in the last frame and sends it to the GPU
         */
        void upload_new_geometry();

        /*!
        * \brief Removes all gui render objects and thereby deletes all the buffers
        */
        void remove_gui_render_objects();

    private:
        std::unordered_map<std::string, std::vector<render_object>> renderables_grouped_by_shader;

        std::mutex chunk_parts_to_upload_lock;
        /*!
         * \brief A list of chunk renderable things that are ready to upload to the GPU
         */
        std::queue<std::tuple<std::string, mesh_definition>> chunk_parts_to_upload;

        float seconds_spent_updating_chunks = 0;
        long total_chunks_updated = 0;

        /*!
         * \brief Removes all the render_objects from the lists of render_objects that match the given fitler funciton
         *
         * The idea here is that when things like the GUI screen change, or when a chunk changes, old geometry will need
         * to be removed. This should accomplish that.
         *
         * \param filter The function to use to decide which (if any) objects to remove
         */
        void remove_render_objects(std::function<bool(render_object&)> fitler);
    };

};

#endif //RENDERER_GEOMETRY_CACHE_H
