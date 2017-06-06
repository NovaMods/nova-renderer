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

namespace nova {
    namespace test {
        class mesh_store_test : public nova_test {};

        TEST_F(mesh_store_test, add_gui_geometry_test) {
            auto shaderpack_name = "default";
            auto loaded_shaderpack = std::make_shared<shaderpack>(nova::load_shaderpack(shaderpack_name));

            nova::mesh_store meshes;
            meshes.set_shaderpack(loaded_shaderpack);

            mc_gui_send_buffer_command send_gui_buffer_command = {};
            send_gui_buffer_command.texture_name = "gui/widgets";
            send_gui_buffer_command.vertex_buffer = new float[90]{
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0,
            };
            send_gui_buffer_command.index_buffer = new int[10]{
                    0, 1, 2, 1, 2, 3, 4, 5, 6, 4,
            };
            send_gui_buffer_command.index_buffer_size = 10;
            send_gui_buffer_command.vertex_buffer_size = 10;
            send_gui_buffer_command.atlas_name = "gui";

            meshes.add_gui_buffers(&send_gui_buffer_command);

            auto& gui_meshes = meshes.get_meshes_for_shader("gui");

            ASSERT_EQ(1, gui_meshes.size());

            auto& gui_mesh = gui_meshes[0];

            ASSERT_EQ(0, gui_mesh.parent_id);
            ASSERT_EQ(true, gui_mesh.is_solid);
            ASSERT_EQ(false, gui_mesh.is_transparent);
            ASSERT_EQ(false, gui_mesh.is_cutout);
            ASSERT_EQ(false, gui_mesh.is_emissive);
            ASSERT_EQ(0, gui_mesh.damage_level);
            ASSERT_EQ(nova::geometry_type::gui, gui_mesh.type);
            ASSERT_EQ("gui", gui_mesh.name);
            ASSERT_EQ("gui", gui_mesh.color_texture);
            ASSERT_EQ(false, gui_mesh.normalmap.has_value());
            ASSERT_EQ(false, gui_mesh.data_texture.has_value());
        }

        TEST_F(mesh_store_test, test_set_shaderpack) {
            //auto shaders = shaderpack();
        }
    }
}


