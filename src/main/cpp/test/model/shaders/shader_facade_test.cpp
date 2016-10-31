/*!
 * \brief Tests the shader facade
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <gtest/gtest.h>

#include <model/shaders/shader_facade.h>
#include <view/nova_renderer.h>

TEST(shader_tree_node, calculate_filters_simple) {
    auto node = nova::model::shader_tree_node("test", [](const auto& geom) {return geom.is_block;});
    auto loaded_shaders = std::vector<std::string>{"test"};

    node.calculate_filters(loaded_shaders);

    auto filter_function = node.get_filter_function();

    // Check that the filter function we get back is the same one we sent in
    // I don't know of a better way to test this... and technically this test case will test both calcualte_filters and
    // get_filter_function. Oops.

    nova::model::render_object block = {};
    block.is_block = true;

    nova::model::render_object not_a_block = {};

    EXPECT_EQ(true, filter_function(block)) << "Failed to accept a block";
    EXPECT_EQ(false, filter_function(not_a_block)) << "Failed to reject a not-block";
}

TEST(shader_tree_node, calculate_filters_one_child) {
    auto node = nova::model::shader_tree_node(
            "gbuffers_terrain",
            [](const auto &geom) {
                return geom.type == nova::model::geometry_type::block && !geom.is_transparent;
            },
            {
                    nova::model::shader_tree_node(
                            "gbuffers_damaged_block", [](const auto &geom) {
                                return geom.type == nova::model::geometry_type::block && geom.is_transparent;
                            }
                    )
            }
    );

    auto loaded_shaders = std::vector<std::string>{"gbuffers_terrain"};

    node.calculate_filters(loaded_shaders);

    auto filter_function = node.get_filter_function();

    nova::model::render_object terrain_block = {};
    terrain_block.type = nova::model::geometry_type::block;
    terrain_block.is_transparent = false;

    nova::model::render_object damaged_block = {};
    damaged_block.type = nova::model::geometry_type::block;
    damaged_block.damage_level = 2;

    nova::model::render_object cloud = {};
    cloud.type = nova::model::geometry_type::cloud;

    EXPECT_EQ(true, filter_function(terrain_block)) << "Filter function did not accept a terrain block!";
    EXPECT_EQ(true, filter_function(damaged_block)) << "Filter function did not accept a damaged block!";
    EXPECT_EQ(false, filter_function(cloud)) << "Filter function did not reject a cloud!";
}

