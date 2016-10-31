/*!
 * \brief Contains tests for the shader loading stuff
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <gtest/gtest.h>

#include <model/loaders/shader_loading.h>

TEST(shader_loading, read_shader_stream_no_includes) {
    auto shader_path = "shaderpacks/default/shaders/gui.frag";
    auto shader_stream = std::ifstream(shader_path);

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

TEST(shader_loading, read_shader_stream_includes) {
    auto shader_path = "shaderpacks/default/shaders/gui_with_include.frag";
    auto shader_stream = std::ifstream(shader_path);

    ASSERT_EQ(shader_stream.good(), true);

    auto shader_file = nova::model::read_shader_stream(shader_stream, shader_path);

    EXPECT_EQ(shader_file.size(), 27);

    auto line_5 = shader_file[4];
    EXPECT_EQ(line_5.line_num, 3);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "layout(binding = 0) uniform sampler2D colortex;");

    auto line_18 = shader_file[16];
    EXPECT_EQ(line_18.line_num, 5);
    EXPECT_EQ(line_18.shader_name, shader_path);
    EXPECT_EQ(line_18.line, "layout(binding = 0) uniform sampler2D colortex;");
}

TEST(shader_loading, get_filename_from_include_relative) {
    auto include_line = "#include \"gui.frag\"";

    std::string filename = nova::model::get_filename_from_include(include_line);

    EXPECT_EQ(filename, "gui.frag");
}

TEST(shader_loading, get_filename_from_include_absolute) {
    auto include_line = "#include \"/include/lighting.glsl\"";

    std::string filename = nova::model::get_filename_from_include(include_line);

    EXPECT_EQ(filename, "/include/lighting.glsl");
}

TEST(shader_loading, load_included_file) {
    auto shader_path = "shaderpacks/default/shaders/gui_with_include.frag";
    auto include_line = "#include \"gui.frag\"";

    auto shader_file = nova::model::load_included_file(shader_path, include_line);

    EXPECT_EQ(shader_file.size(), 13);

    auto line_5 = shader_file[4];
    EXPECT_EQ(line_5.line_num, 5);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "in vec2 uv;");
}
