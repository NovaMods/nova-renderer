/*!
 * \brief A collection of functions to test the mesh_store class
 *
 * \author ddubois 
 * \date 17-Jan-17.
 */

#include <gtest/gtest.h>
#include "../../../render/nova_renderer.h"
#include "../../test_utils.h"
#include "../../../data_loading/loaders/loaders.h"


TEST(mesh_store_test, add_gui_geometry_test) {
    nova::nova_renderer::init();

    auto shaderpack_name = "default";
    auto shaderpack = nova::load_shaderpack(shaderpack_name);

    nova::mesh_store meshes;
    meshes.set_shaderpack(shaderpack);

    mc_gui_screen gui_screen = get_gui_screen_one_button();
    meshes.add_gui_geometry(gui_screen);

    auto gui_meshes = meshes.get_meshes_for_shader("gui");

    ASSERT_EQ(gui_meshes.size(), 1);

    auto mesh = gui_meshes[0];

    EXPECT_EQ(mesh->is_solid, true);
    EXPECT_EQ(mesh->is_transparent, false);
    EXPECT_EQ(mesh->is_cutout, false);
    EXPECT_EQ(mesh->is_emissive, false);
    EXPECT_EQ(mesh->damage_level, 0);
    EXPECT_EQ(mesh->type, nova::geometry_type::gui);
    EXPECT_EQ(mesh->name, "gui");

    nova::nova_renderer::deinit();
}
