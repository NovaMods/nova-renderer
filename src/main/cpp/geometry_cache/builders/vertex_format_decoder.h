/*!
 * \brief Holds functions to decode all of Minecraft's vertex formats
 *
 * \author gold1 
 * \date 02-Jul-17.
 */

#ifndef RENDERER_VERTEX_FORMAT_DECODER_H
#define RENDERER_VERTEX_FORMAT_DECODER_H

#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

namespace nova {
    /*!
     * \brief A single vertex, in the format used by Minecraft
     */
    struct mc_block_vertex {
        glm::vec3 position;
        unsigned int color;
        glm::vec2 uv;
        short lightmap_s;
        short lightmap_t;
    };

    enum class face_id {
        LEFT,
        RIGHT,
        BOTTOM,
        TOP,
        BACK,
        FRONT,
        INSIDE_BLOCK,
    };

    /*!
     * \brief Contains all the data needed for a single vertex in a block
     */
    struct block_vertex {
        glm::vec3 position;
        unsigned int color;
        glm::vec2 uv;
        glm::vec2 lightmap_uv;
        glm::vec3 normal;
        glm::vec3 tangent;
    };

    struct block_face {
        block_vertex vertices[4];
    };

    struct baked_model {
        std::unordered_map<face_id, block_face> faces;
    };

    /*!
     * \brief Decodes vertex data from the block vertex format into a format that Nova understands
     *
     * We have to decode the data so that Nova can process it
     *
     * Block format is as follows:
     *  * Position      3f      12 bytes
     *  * Color         4ub     4 bytes
     *  * UV            2f      8 bytes
     *  * Lightmap UV   2s      4 bytes
     *
     * Sample data:
     *  0 0 1065353216 -8421505 1027607101 1050673807 0
     *  0 0 0 -8421505 1027607101 1051721073 0
     *  1065353216 0 0 -8421505 1031796163 1051721073 0
     *  1065353216 0 1065353216 -8421505 1031796163 1050673807 0
     *
     * \param data
     * \param num_elements
     * \return
     */
    std::vector<block_vertex> decode_block_vertices(int* data, int num_elements);

    /*!
     * \brief Calculates the normals for the block vertices given in the input vector. The normals and tangents are
     * written directly to the passed in array
     *
     * \param vertices The vertices to calculate the normals and tangents for
     */
    void calculate_normals_for_vertices(std::vector<block_vertex> &vertices);
}

#endif //RENDERER_VERTEX_FORMAT_DECODER_H
