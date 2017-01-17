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
#include "../../../test_utils.h"

TEST(gui_geometry_builder, add_vertex) {
    auto vertex_buffer = std::vector<float>{};

    nova::add_vertex(vertex_buffer, 5, 5, 0.5, 0.5);

    EXPECT_EQ(vertex_buffer.size(), 5);

    EXPECT_EQ(vertex_buffer[0], 5);
    EXPECT_EQ(vertex_buffer[1], 5);
    EXPECT_EQ(vertex_buffer[2], 0);
    EXPECT_EQ(vertex_buffer[3], 0.5);
    EXPECT_EQ(vertex_buffer[4], 0.5);
}

TEST(gui_geometry_builder, add_vertices_from_button) {
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

TEST(gui_geometry_builder, add_indices_with_offset_positive) {
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

TEST(gui_geometry_builder, add_indices_with_offset_negative_offset) {
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

TEST(gui_geometry_builder, build_gui_geometry_no_buttons) {
    mc_gui_screen gui = {};
    gui.num_buttons = 0;

    nova::mesh_definition gui_mesh = nova::build_gui_geometry(gui);

    EXPECT_EQ(gui_mesh.vertex_data.size(), 0);
    EXPECT_EQ(gui_mesh.indices.size(), 0);
    EXPECT_EQ(gui_mesh.vertex_format, nova::format::POS_UV);
}

TEST(gui_geometry_builder, build_gui_geometry_one_button) {
    mc_gui_screen gui = get_gui_screen_one_button();

    nova::mesh_definition gui_mesh = nova::build_gui_geometry(gui);

    EXPECT_EQ(gui_mesh.vertex_data.size(), 20);
    auto expected_vertex_data = std::vector<float>{
            0,     0, 0,  0,        0.3359375f,
            100,   0, 0,  0.78125f, 0.3359375f,
            0,   100, 0,  0.0f,     0.4156963f,
            100, 100, 0,  0.78125f, 0.4156963f
    };
    EXPECT_EQ(gui_mesh.vertex_data, expected_vertex_data);

    EXPECT_EQ(gui_mesh.indices.size(), 6);
    auto expected_indices = std::vector<unsigned>{
            0, 1, 2, 2, 1, 3
    };
    EXPECT_EQ(gui_mesh.indices, expected_indices);

    EXPECT_EQ(gui_mesh.vertex_format, nova::format::POS_UV);
}
