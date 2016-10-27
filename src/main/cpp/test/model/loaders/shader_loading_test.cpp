/*!
 * \brief Contains tests for the shader loading stuff
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <gtest/gtest.h>

#include <model/loaders/shader_loading.h>

TEST(shader_loading, load_shader_stream_one_extension_no_includes) {
    auto shader_path = std::string{"default/shaders/gui.frag"};
    auto shader_stream = std::ifstream("shaderpacks/" + shader_path);

    ASSERT_EQ(shader_stream.good(), true);

    auto shader_file = nova::model::read_shader_stream(shader_stream, shader_path);

    // Did we read in the right number of lines?
    EXPECT_EQ(shader_file.size(), 13);

    // Do the lines we read in make sense? Let's grab a couple and see what's up
    auto line_5 = shader_file[4];
    EXPECT_EQ(line_5.line_num, 5);
    EXPECT_EQ(line_5.shader_name, shader_path);
    EXPECT_EQ(line_5.line, "in vec2 uv;");
}
