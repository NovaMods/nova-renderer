/*!
 * \brief Contains tests for the shader loading stuff
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <fstream>

#include <gtest/gtest.h>
#include "../../../render/nova_renderer.h"
#include "../../../data_loading/loaders/loaders.h"
#include "../../../data_loading/loaders/shader_loading.h"
#include "../../../utils/utils.h"
#include "../../test_utils.h"

namespace nova {
    namespace test {
        class shader_loading_test : public nova_test {};
        
        TEST(shader_loading, read_shader_stream_no_includes) {
            auto shader_path = "shaderpacks/default/shaders/gui.frag";
            auto shader_stream = std::ifstream(shader_path);
        
            ASSERT_TRUE(shader_stream.good());
        
            auto shader_file = nova::read_shader_stream(shader_stream, shader_path);
        
            // Did we read in the right number of lines?
            EXPECT_EQ(shader_file.size(), 70);
        
            // Do the lines we read in make sense? Let's grab a couple and see what's up
            auto line_5 = shader_file[4];
            EXPECT_EQ(line_5.line_num, 5);
            EXPECT_EQ(line_5.shader_name, shader_path);
            EXPECT_EQ(line_5.line, "layout(std140) uniform per_frame_uniforms {");
        }
        
        TEST(shader_loading, read_shader_stream_includes) {
            auto shader_path = "shaderpacks/default/shaders/gui_with_include.frag";
            auto shader_stream = std::ifstream(shader_path);
        
            ASSERT_TRUE(shader_stream.good());
        
            auto shader_file = nova::read_shader_stream(shader_stream, shader_path);
        
            EXPECT_EQ(shader_file.size(), 84);
        
            auto line_5 = shader_file[4];
            EXPECT_EQ(line_5.line_num, 3);
            EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
            EXPECT_EQ(line_5.line, "layout(binding = 0) uniform sampler2D colortex;");
        
            auto line_83 = shader_file[82];
            EXPECT_EQ(line_83.line_num, 14);
            EXPECT_EQ(line_83.shader_name, shader_path);
            EXPECT_EQ(line_83.line, "    color = vec3(1, 0, 1);");
        }
        
        TEST(shader_loading, get_filename_from_include_relative) {
            auto include_line = "#include \"gui.frag\"";
        
            std::string filename = nova::get_filename_from_include(include_line);
        
            EXPECT_EQ(filename, "gui.frag");
        }
        
        TEST(shader_loading, get_filename_from_include_absolute) {
            auto include_line = "#include \"/include/lighting.glsl\"";
        
            std::string filename = nova::get_filename_from_include(include_line);
        
            EXPECT_EQ(filename, "/include/lighting.glsl");
        }
        
        TEST(shader_loading, load_included_file) {
            auto shader_path = "shaderpacks/default/shaders/gui_with_include.frag";
            auto include_line = "#include \"gui.frag\"";
        
            auto shader_file = nova::load_included_file(shader_path, include_line);
        
            EXPECT_EQ(shader_file.size(), 70);
        
            auto line_5 = shader_file[4];
            EXPECT_EQ(line_5.line_num, 5);
            EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
            EXPECT_EQ(line_5.line, "layout(std140) uniform per_frame_uniforms {");
        }
        
        TEST(shader_loading, load_shader_file_no_includes_no_extensions) {
            auto shader_path = "shaderpacks/default/shaders/gui";
            auto extensions = std::vector<std::string>{"", ".zip", ".docx"};
        
            EXPECT_THROW(nova::load_shader_file(shader_path, extensions), nova::resource_not_found);
        }
        
        TEST(shader_loading, load_shader_file_no_inlcudes_one_extension) {
            auto shader_path = "shaderpacks/default/shaders/gui";
            auto extensions = std::vector<std::string>{".frag"};
        
            auto shader_file = nova::load_shader_file(shader_path, extensions);
        
            EXPECT_EQ(shader_file.size(), 70);
        
            auto line_5 = shader_file[4];
            EXPECT_EQ(line_5.line_num, 5);
            EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
            EXPECT_EQ(line_5.line, "layout(std140) uniform per_frame_uniforms {");
        }
        
        TEST(shader_loading, load_shader_file_no_includes_two_extensions) {
            auto shader_path = "shaderpacks/default/shaders/gui";
            auto extensions = std::vector<std::string>{".fsh", ".frag"};
        
            auto shader_file = nova::load_shader_file(shader_path, extensions);
        
            EXPECT_EQ(shader_file.size(), 70);
        
            auto line_5 = shader_file[4];
            EXPECT_EQ(line_5.line_num, 5);
            EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
            EXPECT_EQ(line_5.line, "layout(std140) uniform per_frame_uniforms {");
        }
        
        TEST(shader_loading, load_shader_file_one_include_one_extension) {
            auto shader_path = "shaderpacks/default/shaders/gui_with_include";
            auto extensions = std::vector<std::string>{".frag"};
        
            auto shader_file = nova::load_shader_file(shader_path, extensions);
        
            EXPECT_EQ(shader_file.size(), 84);
        
            auto line_5 = shader_file[4];
            EXPECT_EQ(line_5.line_num, 3);
            EXPECT_EQ(line_5.shader_name, "shaderpacks/default/shaders/gui.frag");
            EXPECT_EQ(line_5.line, "layout(binding = 0) uniform sampler2D colortex;");
        
            auto line_33 = shader_file[82];
            EXPECT_EQ(line_33.line_num, 14);
            EXPECT_EQ(line_33.shader_name, "shaderpacks/default/shaders/gui_with_include.frag");
            EXPECT_EQ(line_33.line, "    color = vec3(1, 0, 1);");
        }
        
        TEST(shader_loading, load_sources_from_folder) {
            auto shaderpack_name = "default";
            auto shader_names = std::vector<std::string>{"gui"};
        
            auto shaderpack = nova::load_sources_from_folder(shaderpack_name, shader_names);
        
            auto gui_shader = shaderpack["gui"];
        
            EXPECT_EQ(gui_shader.get_name(), "gui");
        }
        
        TEST_F(shader_loading_test, load_shaderpack_folder) {        
            auto shaderpack_name = "default";
        
            auto shaderpack = nova::load_shaderpack(shaderpack_name);
        
            auto gui_shader = shaderpack["gui"];
        
            EXPECT_EQ(gui_shader.get_name(), "gui");
        }
        
        TEST(shader_loading, get_shader_definitions) {
            auto gui_json = nlohmann::json{
                    {"name", "gui"},
                    {"filters", "geometry_type::gui"},
                    {"fallback", "gbuffers_textured"}
            };
        
            auto json = nlohmann::json{
                    { "shaders", { gui_json } }
            };
        
            auto def = nova::shader_definition{gui_json};
        
            auto definitions = nova::get_shader_definitions(json);
        
            EXPECT_EQ(definitions.size(), 1);
        
            auto gui_def = definitions[0];
            EXPECT_EQ(gui_def.name, def.name);
            EXPECT_EQ(gui_def.fallback_name, def.fallback_name);
        }
    }
}
