/*!
 * \brief Tests geometry filters and the loading thereof
 *
 * Technically this file should only test the loading and not the filters themselves, but it does both. All well.
 *
 * \author gold1 
 * \date 13-Jun-17.
 */

#include <gtest/gtest.h>
#include "../../../data_loading/loaders/geometry_filter_loading.h"

namespace nova {
    namespace test {
        TEST(geometry_loading_test, make_filter_from_token_geometry_type) {
            auto token = "geometry_type::block";
            auto filter = make_filter_from_token(token);
            auto block = mc_block();

            ASSERT_TRUE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_geometry_type_invalid) {
            auto token = "geometry_type::invalid_af";
            ASSERT_THROW(make_filter_from_token(token), std::exception);
        }

        TEST(geometry_filter_loading, make_filter_from_token_name) {
            auto token = "name::john";
            auto filter = make_filter_from_token(token);
            auto block = mc_block();

            block.name = "john";
            ASSERT_TRUE(filter->matches(block));

            block.name = "this includes john";
            ASSERT_FALSE(filter->matches(block));

            block.name = "does not include token";
            ASSERT_FALSE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_name_part) {
            auto token = "name_part::fred";

            auto filter = make_filter_from_token(token);

            auto block = mc_block();

            block.name = "fred_flintstone";
            ASSERT_TRUE(filter->matches(block));

            block.name = "things before fred and after";
            ASSERT_TRUE(filter->matches(block));

            block.name = "things before fred";
            ASSERT_TRUE(filter->matches(block));

            block.name = "does not contain token";
            ASSERT_FALSE(filter->matches(block));

            block.name = "contains part fre";
            ASSERT_FALSE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_transparent) {
            auto token = "transparent";
            auto filter = make_filter_from_token(token);
            auto block = mc_block();

            block.is_opaque = false;
            ASSERT_TRUE(filter->matches(block));

            block.is_opaque = true;
            ASSERT_FALSE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_not_transparent) {
            auto token = "not_transparent";
            auto filter = make_filter_from_token(token);
            auto block = mc_block();

            block.is_opaque = true;
            ASSERT_TRUE(filter->matches(block));

            block.is_opaque = false;
            ASSERT_FALSE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_emissive) {
            auto token = "emissive";
            auto filter = make_filter_from_token(token);
            auto block = mc_block();

            block.light_value = 16;
            ASSERT_TRUE(filter->matches(block));

            block.light_value = 0;
            ASSERT_FALSE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_not_emissive) {
            auto token = "not_emissive";
            auto filter = make_filter_from_token(token);
            auto block = mc_block();

            block.light_value = 0;
            ASSERT_TRUE(filter->matches(block));

            block.light_value = 16;
            ASSERT_FALSE(filter->matches(block));
        }

        TEST(geometry_filter_loading, make_filter_from_token_invalid_token) {
            auto token = "invalid token";
            ASSERT_THROW(make_filter_from_token(token), std::runtime_error);
        }

        TEST(geometry_filter_loading, make_filter_expression_and) {
            auto tokens = std::vector<std::string>{"transparent", "AND", "not_emissive"};
            auto previous_filter = make_filter_from_token(tokens[0]);
            auto tokens_itr = tokens.begin() + 1;

            auto filter = make_filter_expression(previous_filter, tokens_itr, tokens.end());

            auto block = mc_block();
            block.is_opaque = false;
            block.light_value = 0;

            ASSERT_TRUE(filter->matches(block));

            block.is_opaque = true;
            ASSERT_FALSE(filter->matches(block));

            block.light_value = 16;
            block.is_opaque = false;
            ASSERT_FALSE(filter->matches(block));

            block.is_opaque = true;
            ASSERT_FALSE(filter->matches(block));
        }
    }
}