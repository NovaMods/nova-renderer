/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GL_VERTEX_BUFFER_H
#define RENDERER_GL_VERTEX_BUFFER_H

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "mesh_definition.h"

namespace nova {
    class render_context;

    /*!
     * \brief Represents a buffer which holds vertex information
     *
     * Buffers of this type can hold positions, positions and texture coordinates, or positions, texture coordinates,
     * lightmap coordinates, normals, and tangents.
     */
    class vk_mesh {
    public:
        vk_mesh(std::shared_ptr<render_context> context);

        explicit vk_mesh(const mesh_definition &definition);

        ~vk_mesh();

        void destroy();

        /*!
         * \brief Sets the given data as this vertex buffer's data, and uploads that data to the GPU
         *
         * \param vertex_data The interleaved vertex data
         * \param data_format The format of the data (\see format)
         */
        void set_data(const std::vector<int>& vertex_data, const std::vector<int>& index_data);

        /*!
         * \brief Returns the format of this vertex buffer
         *
         * \return The format of this vertex buffer
         */
        format get_format();

        bool has_data() const;

        vk::Buffer vertex_buffer;
        vk::Buffer indices;

        uint32_t num_indices;
    private:
        std::shared_ptr<render_context> context;

        format data_format;

        VmaAllocation vertex_alloc;

        VmaAllocation indices_alloc;

        void upload_vertex_data(const std::vector<int> &vertex_data);

        void upload_index_data(const std::vector<int>& index_data);
    };
}

#endif //RENDERER_GL_VERTEX_BUFFER_H
