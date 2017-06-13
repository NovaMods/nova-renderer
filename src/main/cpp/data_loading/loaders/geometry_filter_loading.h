/*!
 * \brief Defines functions to parse a geometry filter expression into an igeometry_filter
 *
 * \author gold1 
 * \date 12-Jun-17.
 */

#ifndef RENDERER_GEOMETRY_FILTER_LOADING_H
#define RENDERER_GEOMETRY_FILTER_LOADING_H

#include <string>
#include <memory>
#include "../../render/objects/shaders/geometry_filter.h"

namespace nova {
    /*!
     * \brief Parses a geometry filter expression into a working geometry filter
     *
     * The language of the expression is as follows:
     *  * A token prefixed by `name::` will match geometry with the exact name as the bit after the prefix
     *  * A token prefixed by `name_part::` will match geometry whose name contains the bit after the prefix
     *  * A token prefixed by `geometry_type::` will match geometry of that type
     *  * `transparent` will match geometry which is transparent
     *  * `not_transparent` will match geometry which is not transparend
     *  * `emissive` will match geometry which is emissive
     *  * `not_emissive` will match geometry which is not emissive
     *  * `AND` will match geometry which matches both the filters on either side of the `AND` token
     *  * `OR` will match geometry which matches one of the filters on either side of the `OR` token
     *  * Expressions are combined from left-to-right
     *
     * Some examples:
     *  * `transparent` will generate a filter that matches transparent geometry
     *  * `geometry_type::block` will match blocks
     *  * `emissive AND not_transparent` will match geometry which is emissive but is in no way transparent
     *  * `emissive AND not_transparent OR geometry_type::entity` will match geometry which is emissive nad not
     *  transparent, or is an entity
     *
     * Currently this function does not support any sort of grouping. It probably will eventually, but now it does not
     *
     * \param expression The expression to parse into a geometry filter
     * \return A geometry filter which will match the things specified in the given expression
     */
    std::shared_ptr<igeometry_filter> parse_filter_expression(const std::string& expression);

    std::shared_ptr<igeometry_filter> make_filter_expression(std::shared_ptr<igeometry_filter> previous_filte,
                                                             std::vector<std::string>::iterator tokens_itr,
                                                             std::vector<std::string>::iterator end_itr);

    std::shared_ptr<igeometry_filter> make_filter_from_token(const std::string& token);
}

#endif //RENDERER_GEOMETRY_FILTER_LOADING_H
