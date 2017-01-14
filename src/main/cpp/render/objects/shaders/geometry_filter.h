#ifndef GEOMETRY_FILTER_H
#define GEOMETRY_FILTER_H

#include <vector>
#include <string>
#include <optional.hpp>
#include <unordered_map>
#include "../render_object.h"

namespace nova {
    /*!
     * \brief Holds a bunch of information to filter geometry
     *
     * At first I used lambdas. Lambdas are very powerful: they can do anything. Anything!
     *
     * The lambdas were super hard to debug, though. I stored references to them in some kinda of structure, and
     * those references were really bad at pointing back to the line of code where the lambda was actually
     * constructed. I couldn't see what was actually happening. this would be fine, except that it wasn't and I
     * kept getting crashes that were super hard to debug.
     *
     * Now there's a simple data structure. Data is much easier to debug. No more stupid crashes for me!
     *
     * (i hope)
     *
     * There's a precidence for these filters:
     *  - If one of the optional values isn't set, then it's ignored.
     *  - Block, Entity, Sky, Particle, Geometry Type, and Name filters are applied first. They're all OR'd together
     *  - Transparent, Cutout, Damaged, and Emissive are applied next, OR'd together
     *  - Any geometry which is accepted by name, name part, or geometry_type is accepted even if other filter
     *  values would cause it to be reject
     *
     * This means you can have a filter which matches transparent blocks and entities, or all cutout blocks of a
     * certain geometry type, but you can't have a filter that matches cutout blocks and emissive particles. If you
     * need that, too bad.
     */
    struct geometry_filter {
        /*!
         * \brief Holds the functions used to modify a geometry_filter
         *
         * The idea here is that a list of filters will be provided in the shaders.json file. Each of those filters
         * corresponds to either a function in this map, or a request to add a specific geometry_type, name, or
         * name part to the filter.
         */
        static std::unordered_map<std::string, std::function<void(geometry_filter&)>> modifying_functions;

        std::vector<geometry_type> geometry_types;

        std::vector<std::string> names;
        std::vector<std::string> name_parts;

        // Using a third-party library because CMake doesn't (yet) support C++17 (probably, updating tool is hard)
        std::experimental::optional<bool> should_be_solid;
        std::experimental::optional<bool> should_be_transparent;
        std::experimental::optional<bool> should_be_cutout;
        std::experimental::optional<bool> should_be_emissive;
        std::experimental::optional<bool> should_be_damaged;
    };

    /*
     * A bunch of functions to operate on a geometry filter
     *
     * These are separate functions and not methods because this way, it's easier to map them to the values stored
     * in the shaders.json file
     */

    void accept_block(geometry_filter &filter);

    void reject_block(geometry_filter &filter);

    void accept_entity(geometry_filter &filter);

    void reject_entity(geometry_filter &filter);

    void accept_selection_box(geometry_filter &filter);

    void reject_selection_box(geometry_filter &filter);

    void accept_particle(geometry_filter &filter);

    void reject_particle(geometry_filter &filter);

    void accept_sky_object(geometry_filter &filter);

    void reject_sky_object(geometry_filter &filter);

    void accept_geometry_type(geometry_filter &filter, geometry_type type);

    void accept_name(geometry_filter &filter, std::string &name);

    void accept_name_part(geometry_filter &filter, std::string &name_part);

    void accept_solid(geometry_filter &filter);

    void reject_solid(geometry_filter &filter);

    void accept_transparent(geometry_filter &filter);

    void reject_transparent(geometry_filter &filter);

    void accept_cutout(geometry_filter &filter);

    void reject_cutout(geometry_filter &filter);

    void accept_emissive(geometry_filter &filter);

    void reject_emissive(geometry_filter &filter);

    void accept_damaged(geometry_filter &filter);

    void reject_damaged(geometry_filter &filter);

    /*!
     * \brief Tells the given filter to accept all geometry attributes that aren't explicitly rejected
     */
    void accept_everything_else(geometry_filter &fitler);

    /*!
     * \brief Tells the given filter to reject all geometry attributes that aren't explicitly allowed
     */
    void reject_everything_else(geometry_filter &filter);
}

#endif
