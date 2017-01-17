/*!
 * \brief A collection of functions to test the mesh_store class
 *
 * \author ddubois 
 * \date 17-Jan-17.
 */

#include <gtest/gtest.h>
#include "../../../render/nova_renderer.h"


TEST(mesh_store_test, add_gui_geometry_test) {
    nova::nova_renderer::init();

    nova::mesh_store meshes;

    mc_gui_screen gui_screen;

    nova::nova_renderer::deinit();
}
