/*!
 * \brief
 *
 * \author ddubois 
 * \date 17-Jan-17.
 */

#include "test_utils.h"
#include "../render/nova_renderer.h"
#include "../utils/io.h"

namespace nova {
    namespace test {
        mc_gui_screen get_gui_screen_one_button()  {
            mc_gui_screen screen = {};
            screen.num_buttons = 1;

            mc_gui_button button;
            button.x_position = 0;
            button.y_position = 0;
            button.width = 100;
            button.height = 100;
            button.text = "Default text";
            button.is_pressed = false;

            screen.buttons[0] = button;

            return screen;
        }

        std::shared_ptr<mc_chunk_render_object> load_test_chunk(std::string chunk_file) {
            auto chunk_filename_full = TEST_RESOURCES_LOCATION + chunk_file;
            return load_chunk(chunk_filename_full);
        }

        void nova_test::SetUp() {
            nova::nova_renderer::init();
        }

        void nova_test::TearDown() {
            nova::nova_renderer::deinit();
        }
    }
}