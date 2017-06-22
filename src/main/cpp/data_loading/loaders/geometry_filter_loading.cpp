/*! 
 * \author gold1 
 * \date 12-Jun-17.
 */

#include "geometry_filter_loading.h"
#include "../../utils/utils.h"

namespace nova {
    std::shared_ptr<igeometry_filter> parse_filter_expression(const std::string& expression) {
        // Currently recognized tokens: AND and OR
        // Everything else is just a literal right now
        // TODO: grouping of some sort

        auto tokens = split(expression, ' ');

        if(tokens.size() % 2 == 0) {
            throw new std::runtime_error("Cannot have an even number of tokens. Check yourself and try again");
        }

        auto tokens_itr = tokens.begin();

        auto filter0 = make_filter_from_token(*tokens_itr);
        if(tokens_itr + 1 == tokens.end()) {
            return filter0;
        }

        return make_filter_expression(filter0, ++tokens_itr, tokens.end());
    }

    std::shared_ptr<igeometry_filter> make_filter_expression(std::shared_ptr<igeometry_filter> previous_filter,
                                                             std::vector<std::string>::iterator tokens_itr,
                                                             std::vector<std::string>::iterator end_itr) {
        auto has_another_expression = tokens_itr + 2 < end_itr;
        std::shared_ptr<igeometry_filter> this_filter;

        if(*tokens_itr == "AND") {
            this_filter = std::make_shared<and_geometry_filter>(previous_filter,
                                                                    make_filter_from_token(*(tokens_itr + 1)));

        } else if(*tokens_itr == "OR") {
            this_filter = std::make_shared<or_geometry_filter>(previous_filter,
                                                                   make_filter_from_token(*(tokens_itr + 1)));

        } else {
            // Regular filter
            return make_filter_from_token(*tokens_itr);
        }

        if(has_another_expression) {
            return make_filter_expression(this_filter, tokens_itr + 2, end_itr);

        } else {
            return this_filter;
        }
    }

    std::shared_ptr<igeometry_filter> make_filter_from_token(const std::string& token) {
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

        } else if(token.find("transparent") == 0) {
            return std::make_shared<transparent_geometry_filter>();

        } else if(token.find("not_transparent") == 0) {
            return std::make_shared<transparent_geometry_filter>(false);

        } else if(token.find("emissive") == 0) {
            return std::make_shared<emissive_geometry_filter>();

        } else if(token.find("not_emissive") == 0) {
            return std::make_shared<emissive_geometry_filter>(false);
        }

        throw std::runtime_error("Could not parse token " + token);
    }
}