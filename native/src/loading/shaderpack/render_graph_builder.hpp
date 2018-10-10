/*!
 * \author ddubois
 * \date 17-Sep-18.
 */

#ifndef NOVA_RENDERER_RENDER_GRAPH_BUILDER_HPP
#define NOVA_RENDERER_RENDER_GRAPH_BUILDER_HPP

#include <string>
#include <unordered_map>
#include "shaderpack_data.hpp"

namespace nova {
    NOVA_EXCEPTION(pass_ordering_exception);
    NOVA_EXCEPTION(circular_rendergraph_exception);

    struct range {
        uint32_t first_write_pass = ~0u;
        uint32_t last_write_pass = 0;
        uint32_t first_read_pass = ~0u;
        uint32_t last_read_pass = 0;

        bool has_writer() const;

        bool has_reader() const;

        bool is_used() const;

        bool can_alias() const;

        unsigned last_used_pass() const;

        unsigned first_used_pass() const;

        bool is_disjoint_with(const range &other) const;
    };

    /*!
     * \brief Orders the provided render passes to satisfy both their implicit and explicit dependencies
     *
     * The provided submission order may not be valid for all sets of passes. It is pretty un-optimized, preferring to
     * execute a pass earlier in the frame - so it may lead to some stalls somewhere? But I'm using OpenGL and the
     * driver deals with all that. There's things I can do that will give me much higher framerate gains than optimizing
     * the submission order
     *
     * \param passes A map from pass name to pass of all the passes to order
     * \return The names of the passes in submission order
     */
    std::vector<std::string> order_passes(const std::unordered_map<std::string, render_pass_data> &passes);

    /*!
     * \brief Puts textures in usage order and determines which have overlapping usage ranges
     *
     * Knowing which textures have an overlapping usage range is super important cause if their ranges overlap, they can't be aliased
     *
     * \param passes All the passes in the current frame graph
     * \param resource_used_range A map to hold the usage ranges of each texture
     * \param resources_in_order A vector to hold the textures in usage order
     */
    void determine_usage_order_of_textures(const std::vector<render_pass_data> &passes, std::unordered_map<std::string, range> &resource_used_range, std::vector<std::string> &resources_in_order);

    /*!
     * \brief Determines which textures can be aliased to which other textures
     *
     * \param textures All the dynamic textures that this frame graph needs
     * \param resource_used_range The range of passes where each texture is used
     * \param resources_in_order The dynamic textures in usage order
     *
     * \return A map from texture name to the name of the texture the first texture can be aliased with
     */
    std::unordered_map<std::string, std::string> determine_aliasing_of_textures(
        const std::unordered_map<std::string, texture_resource_data> &textures, const std::unordered_map<std::string, range> &resource_used_range, const std::vector<std::string> &resources_in_order);
}  // namespace nova

#endif  // NOVA_RENDERER_RENDER_GRAPH_BUILDER_HPP
