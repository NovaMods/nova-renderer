/*! 
 * \author gold1 
 * \date 02-Jul-17.
 */

#include "vertex_format_decoder.h"
#include "chunk_builder.h"
#include <easylogging++.h>

#include <stdexcept>

namespace nova {

    std::vector<block_vertex> decode_block_vertices(int * data, int num_elements) {
        if(num_elements % 7 != 0) {
            LOG(ERROR) << "Received " << num_elements << " elements, which is not divisible by 7. Please help";
            throw new std::runtime_error("The number of elements is not divisible by seven, I don't know what's up");
        }

        auto vertices = std::vector<block_vertex>{};

        for(int i = 0; i < num_elements; i += 7) {
            mc_block_vertex* mc_vertex = (mc_block_vertex *) &data[i];
            block_vertex vertex;
            vertex.position = mc_vertex->position;
            vertex.color = mc_vertex->color;
            vertex.uv = mc_vertex->uv;
            vertex.lightmap_uv.s = mc_vertex->lightmap_s;
            vertex.lightmap_uv.t = mc_vertex->lightmap_t;
            LOG(TRACE) << "Made vertex " << vertex;
            vertices.push_back(vertex);
        }

        LOG(DEBUG) << "Created " << vertices.size() << " vertices";

        calculate_normals_for_vertices(vertices);

        return vertices;
    }

    void calculate_normals_for_vertices(std::vector<block_vertex> &vertices) {
        auto vector_up = vertices[1].position - vertices[0].position;
        auto vector_right = vertices[1].position - vertices[2].position;
        glm::vec3 binormal;
        glm::vec3 tangent;

        // Detect texture coordinates direction
        auto texture_coords_up = vertices[1].uv - vertices[0].uv;
        auto texture_coords_right = vertices[1].uv - vertices[2].uv;

        if(texture_coords_up.x == 0) {
            // There's no change in the X coord when you go up the face, so going up the face gives us the direction of the binormal
            binormal = vector_up;
            tangent = vector_right * (std::signbit(texture_coords_up.y) ? -1.f : 1.f);
        } else if(texture_coords_up.y == 0) {
            // There's no change in the Y coord when you go up the face, so going up the face gives us the direction of the tangent
            tangent = vector_up * (std::signbit(texture_coords_up.x) ? -1.f : 1.f);
            binormal = vector_right;
        }

        glm::vec3 normal = cross(tangent, normal);

        for(auto& vertex : vertices) {
            vertex.normal = normal;
            vertex.tangent = tangent;
        }
    }
}
