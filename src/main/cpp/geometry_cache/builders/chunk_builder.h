/*!
 * \brief Contains a bunch of functions to generate the geometry for a chunk
 *
 * Due to a quirk of this system, it'll generate faces at chunk boundaries for blocks other than air
 *
 * \author ddubois 
 * \date 02-Mar-17.
 */

#ifndef RENDERER_CHUNK_BUILDER_H
#define RENDERER_CHUNK_BUILDER_H

#include <string>
#include <vector>
#include <optional.hpp>
#include "../../mc_interface/mc_objects.h"
#include "../../render/objects/shaders/shaderpack.h"
#include "vertex_format_decoder.h"
#include <easylogging++.h>

using namespace std::experimental;

namespace nova {

    class chunk_builder {
    public:

        /*!
         * \brief Finds the indices of all the blocks that match the provided filter, and returns those indices
         *
         * \param chunk The chunk to look at the blocks of
         * \param filter The filter to match blocks against
         * \return A list of all the positions of blocks that match the filter
         */
        std::vector<glm::ivec3> get_blocks_that_match_filter(const mc_chunk &chunk, const std::shared_ptr<igeometry_filter> filter);

        /*!
         * \brief Makes a mesh_definition for all the provided blocks
         *
         * This function will generate block faces at the interface between chunks (it won't generate them if the chunk
         * in question has an air block at its edge, obviously). This is because I look at chunks one at a time because
         * it's easy
         *
         * \param blocks The blocks to create a mesh from
         * \return The mesh that was created from the blocks
         */
        mesh_definition make_mesh_for_blocks(const std::vector<glm::ivec3>& blocks, const mc_chunk& chunk);

        /*!
         * \brief Determines if the block in the provided chunk at the given position should be rendered with a simple cube
         * or if the block has a more complex model
         *
         * \param pos The position to check for the block at
         * \param chunk The chunk that has the block in question in it
         * \return True if the block at the provided position is a simple cube, false if it should use a more complex model
         */
        bool is_cube(const glm::ivec3 pos, const mc_chunk& chunk);

        /*!
         * \brief Helper function to convert from nice easy vec3 to position in the blocks array
         *
         * \param pos The position to convert
         * \return The index in the block array that corresponds to the pos
         */
        int pos_to_idx(const glm::ivec3& pos);

        /*!
         * \brief Checks if the block at the given position in the provided chunk is a block you can see through, and thus
         * if a face is needed for the interface between the current block and the block we're checking
         *
         * \param block_pos The index in the blocks array of the block to check
         * \param chunk The chunk to check the blocks in
         * \return True if the block at the provided position is not fully opaque or is not within the given chunk, false
         * otherwise
         */
        bool block_at_pos_is_opaque(glm::ivec3 block_pos, const mc_chunk& chunk);


        bool block_at_offset_is_same(glm::ivec3 block_pos, glm::ivec3 offset, const mc_chunk& chunk);

        /*!
         * \brief Makes the geometry for the provided block in the given chunk
         */
        std::vector<block_face> make_geometry_for_block(const glm::ivec3& block_pos, const mc_chunk& chunk, baked_model& model);

        /*!
         * \brief Gets the AO in the provided direction
         *
         * AO is computed from the blocks around the current block. AO will not compute correctly in the current version
         * because this function only deals with one chunk at a time and AO can be easily influenced by multiple chunks
         *
         * \param position The position of the block to get the AO for
         * \param face_to_check The block face to get AO for
         * \param chunk The chunk that the block lives in
         * \return A float from 0 to 1. 0 means no AO, 1 means all the AO
         */
        float get_ao_in_direction(const glm::vec3 position, const face_id face_to_check, const mc_chunk& chunk);

        std::unordered_map<int, mc_block_definition>& get_block_definitions();

        /*!
         * \brief Registers a block model for later use
         *
         * \param state The block state that this model corresponds to
         * \param num_quads The number of quads in the state
         * \param quads The quads in the state
         */
        void register_block_model(std::string state, int num_quads, mc_baked_quad quads[]);

    private:
        std::unordered_map<int, mc_block_definition> block_definitions;
        std::unordered_map<std::string, baked_model> block_models;
    };

    bool is_in_plane(const std::vector<block_vertex>& vertices, int plane);

    bool is_in_xy_plane(const std::vector<block_vertex>& vertices);

    bool is_at_max_z(const std::vector<block_vertex>& vertices);

    /*!
     * \brief Prints a block_vertex object into the easylogging++ output stream
     *
     * \param out The output stream to print to
     * \param vert The vertex to print
     * \return The output stream that was provided
     */
    el::base::Writer &operator<<(el::base::Writer &out, const block_vertex& vert);
}


#endif //RENDERER_CHUNK_BUILDER_H
