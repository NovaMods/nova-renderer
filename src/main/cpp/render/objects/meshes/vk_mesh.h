/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GL_VERTEX_BUFFER_H
#define RENDERER_GL_VERTEX_BUFFER_H

#include <glad/glad.h>
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
        vk_mesh();

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

        void set_active(vk::CommandBuffer command) const;

        void draw() const;

        /*!
         * \brief Returns the format of this vertex buffer
         *
         * \return The format of this vertex buffer
         */
        format get_format();

        bool has_data() const;

    private:
        format data_format;

        vk::Buffer vertex_buffer;
        VmaAllocation vertex_alloc;

        vk::Buffer indices;
        VmaAllocation indices_alloc;

        uint32_t num_indices;

        void upload_vertex_data(const std::vector<int> &vertex_data, const nova::render_context &context);

        void upload_index_data(const std::vector<int>& index_data, const render_context &context);
    };
}

#endif //RENDERER_GL_VERTEX_BUFFER_H
