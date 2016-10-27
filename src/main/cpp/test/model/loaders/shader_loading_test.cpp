/*!
 * \brief Contains tests for the shader loading stuff
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <model/loaders/shader_loading.h>
#include <3rdparty/googletest/googletest/include/gtest/gtest.h>

TEST(shader_loading, load_shader_stream_one_extension_no_includes) {
    auto shader_path = std::string{"default/shaders/gui"};
    auto shader_stream = std::ifstream("shaderpacks/" + shader_path);

    auto shader_file = nova::model::read_shader_stream(shader_stream, shader_path);

    // Did we read in the right number of lines?
    EXPECT_EQ(shader_file.size(), 14);

    // Do the lines we read in make sense? Let's grab a couple and see what's up
}
