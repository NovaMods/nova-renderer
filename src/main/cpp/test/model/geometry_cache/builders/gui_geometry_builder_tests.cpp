/*!
 * \brief Tests the GUI geometry builder
 *
 * \author ddubois 
 * \date 06-Nov-16.
 */

#include <vector>
#include <gtest/gtest.h>
#include "../../../../mc_interface/mc_gui_objects.h"
#include "../../../../data_loading/geometry_cache/builders/gui_geometry_builder.h"

TEST(gui_geometry_builder, add_vertex_test) {
    auto vertex_buffer = std::vector<float>{};

    nova::add_vertex(vertex_buffer, 5, 5, 0.5, 0.5);

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

    nova::add_vertices_from_button(vertex_buffer, button, uvs);

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

TEST(gui_geometry_builder, add_indices_with_offset_positive_test) {
    auto indices = std::vector<unsigned>{};
    auto start_pos = 3;

    nova::add_indices_with_offset(indices, (unsigned int) start_pos);

    EXPECT_EQ(indices[0], 3);
    EXPECT_EQ(indices[1], 4);
    EXPECT_EQ(indices[2], 5);
    EXPECT_EQ(indices[3], 5);
    EXPECT_EQ(indices[4], 4);
    EXPECT_EQ(indices[5], 6);
}

TEST(gui_geometry_builder, add_indices_with_offset_negative_offset_test) {
    auto indices = std::vector<unsigned>{};
    auto start_pos = -3;

    nova::add_indices_with_offset(indices, (unsigned int) start_pos);

    EXPECT_EQ(indices[0], 4294967293);
    EXPECT_EQ(indices[1], 4294967294);
    EXPECT_EQ(indices[2], 4294967295);
    EXPECT_EQ(indices[3], 4294967295);
    EXPECT_EQ(indices[4], 4294967294);
    EXPECT_EQ(indices[5], 0);
}

TEST(gui_geometry_builder, build_gui_geometry_no_buttons_test) {
    mc_gui_screen gui = {};
    gui.num_buttons = 0;

    nova::mesh_definition gui_mesh = nova::build_gui_geometry(gui);
}
