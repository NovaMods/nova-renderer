/*!
 * \brief Tests the GUI geometry builder
 *
 * \author ddubois 
 * \date 06-Nov-16.
 */

#include <vector>
#include <gtest/gtest.h>

#include <model/geometry_cache/builders/gui_geometry_builder.h>

TEST(gui_geometry_builder, add_vertex_test) {
    auto vertex_buffer = std::vector<float>{};

    nova::model::add_vertex(vertex_buffer, 5, 5, 0.5, 0.5);

    EXPECT_EQ(vertex_buffer.size(), 5);

    EXPECT_EQ(vertex_buffer[0], 5);
    EXPECT_EQ(vertex_buffer[1], 5);
    EXPECT_EQ(vertex_buffer[2], 0);
    EXPECT_EQ(vertex_buffer[3], 0.5);
    EXPECT_EQ(vertex_buffer[4], 0.5);
}

TEST(gui_geometry_builder, add_vertices_from_button_test) {
    auto vertex_buffer = std::vector<float>{};
    auto button = mc_gui_button{0, 0, 100, 100, "Main Menu", false};
    auto uvs = std::vector<float>{0, 0, 0, 1, 1, 0, 1, 1};

    nova::model::add_vertices_from_button(vertex_buffer, button, uvs);

    EXPECT_EQ(vertex_buffer.size(), 20);

    // First vertex
    EXPECT_EQ(vertex_buffer[0], 0);
    EXPECT_EQ(vertex_buffer[1], 0);
    EXPECT_EQ(vertex_buffer[2], 0);
    EXPECT_EQ(vertex_buffer[3], 0);

    // Third vertex because I'm too lazy to check all of them
    EXPECT_EQ(vertex_buffer[10], 0);
    EXPECT_EQ(vertex_buffer[11], 100);
    EXPECT_EQ(vertex_buffer[13], 1);
    EXPECT_EQ(vertex_buffer[14], 0);
}
