/*!
 * \brief Contains tests for the shader loading stuff
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <fstream>

#include <gtest/gtest.h>

#include <model/loaders/shader_loading.h>
#include <model/loaders/loaders.h>
#include <utils/utils.h>
#include <view/nova_renderer.h>

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

TEST(shader_loading, load_shader_file_no_includes_no_extensions) {
    auto shader_path = "shaderpacks/default/shaders/gui";
    auto extensions = std::vector<std::string>{"", ".zip", ".docx"};

    EXPECT_THROW(nova::model::load_shader_file(shader_path, extensions), nova::resource_not_found);
}

TEST(shader_loading, load_shader_file_no_inlcudes_one_extension) {
    auto shader_path = "shaderpacks/default/shaders/gui";
    auto extensions = std::vector<std::string>{".frag"};

    auto shader_file = nova::model::load_shader_file(shader_path, extensions);

    EXPECT_EQ(shader_file.size(), 13);

    auto line_5 = shader_file[4];
    EXPECT_EQ(line_5.line_num, 5);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "in vec2 uv;");
}

TEST(shader_loading, load_shader_file_no_includes_two_extensions) {
    auto shader_path = "shaderpacks/default/shaders/gui";
    auto extensions = std::vector<std::string>{".fsh", ".frag"};

    auto shader_file = nova::model::load_shader_file(shader_path, extensions);

    EXPECT_EQ(shader_file.size(), 13);

    auto line_5 = shader_file[4];
    EXPECT_EQ(line_5.line_num, 5);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "in vec2 uv;");
}

TEST(shader_loading, load_shader_file_one_include_one_extension) {
    auto shader_path = "shaderpacks/default/shaders/gui_with_include";
    auto extensions = std::vector<std::string>{".frag"};

    auto shader_file = nova::model::load_shader_file(shader_path, extensions);

    EXPECT_EQ(shader_file.size(), 27);

    auto line_5 = shader_file[4];
    EXPECT_EQ(line_5.line_num, 3);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "layout(binding = 0) uniform sampler2D colortex;");

    auto line_18 = shader_file[16];
    EXPECT_EQ(line_18.line_num, 5);
    EXPECT_EQ(line_18.shader_name, "shaderpacks/default/shaders/gui_with_include.frag");
    EXPECT_EQ(line_18.line, "layout(binding = 0) uniform sampler2D colortex;");
}

TEST(shader_loading, load_sources_from_folder) {
    auto shaderpack_name = "default";
    auto shader_names = std::vector<std::string>{"gui"};

    auto shaderpack = nova::model::load_sources_from_folder(shaderpack_name, shader_names);

    auto gui_location = shaderpack.find("gui");

    ASSERT_NE(gui_location, shaderpack.end());

    auto gui_shader = gui_location->second;
    auto fragment_source = gui_shader.fragment_source;

    EXPECT_EQ(fragment_source.size(), 13);

    auto line_5 = fragment_source[4];
    EXPECT_EQ(line_5.line_num, 5);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "in vec2 uv;");

    auto vertex_source = gui_shader.vertex_source;

    auto line_6 = vertex_source[5];
    EXPECT_EQ(line_6.line_num, 6);
    EXPECT_EQ(line_6.shader_name, "shaderpacks/default/shaders/gui.vert");
    EXPECT_EQ(line_6.line, "layout(binding = 20, std140) uniform gui_uniforms {");
}

TEST(shader_loading, load_shaderpack_folder) {
    nova::view::nova_renderer::init_instance();

    auto shaderpack_name = "default";

    auto shaderpack = nova::model::load_shaderpack(shaderpack_name);

    auto gui_location = shaderpack.find("gui");

    ASSERT_NE(gui_location, shaderpack.end());

    auto gui_shader = gui_location->second;
    auto gui_fragment_source = gui_shader.fragment_source;

    EXPECT_EQ(gui_fragment_source.size(), 13);

    auto line_5 = gui_fragment_source[4];
    EXPECT_EQ(line_5.line_num, 5);
    EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
    EXPECT_EQ(line_5.line, "in vec2 uv;");

    auto gui_vertex_source = gui_shader.vertex_source;

    auto line_6 = gui_vertex_source[5];
    EXPECT_EQ(line_6.line_num, 6);
    EXPECT_EQ(line_6.shader_name, "shaderpacks/default/shaders/gui.vert");
    EXPECT_EQ(line_6.line, "layout(binding = 20, std140) uniform gui_uniforms {");
}
