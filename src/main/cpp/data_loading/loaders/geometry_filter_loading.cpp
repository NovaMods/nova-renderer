/*! 
 * \author gold1 
 * \date 12-Jun-17.
 */

#include "geometry_filter_loading.h"
#include "../../utils/utils.h"

namespace nova {
    std::shared_ptr<igeometry_filter> parse_filter_expression(std::string expression) {
        // Currently recognized tokens: AND and OR
        // Everything else is just a literal right now
        // TODO: grouping of some sort

        auto tokens = split(expression, ' ');

        if(tokens.size() % 2 == 0) {
            throw new std::runtime_error("Cannot have an even number of tokens. Check yourself and try again");
        }

        if(tokens.size() == 1) {
            // One token, this should be easy to parse
            auto token = tokens[0];
            if(token.find("geometry_type::") == 0) {
                auto type_name_str = token.substr(15);
                auto type_name = geometry_type::from_string(type_name_str);
                return std::make_shared<geometry_type_geometry_filter>(type_name);

            } else if(token.find("name::") == 0) {
                auto name = token.substr(6);
                return std::make_shared<name_geometry_filter>(name);

            } else if(token.find("name_part::") == 0) {
                auto name_part = token.substr(11);
                return std::make_shared<name_part_geometry_filter>(name_part);
            }
        }

    }
}