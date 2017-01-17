/*!
 * \brief Tests the gl_shader_program class
 *
 * \author ddubois 
 * \date 13-Jan-17.
 */

#include <gtest/gtest.h>
#include "../../../render/nova_renderer.h"

nlohmann::json get_gui_def_json();
void add_shader_source_to_definition(nova::shader_definition& def);

TEST(gl_shader_program, constructor_name_test) {
    nova::nova_renderer::init();

    auto json = get_gui_def_json();
    nova::shader_definition def(json);
    add_shader_source_to_definition(def);

    nova::gl_shader_program shader(def);

    EXPECT_EQ(shader.get_name(), "gui");

    auto filter = shader.get_filter();

    ASSERT_EQ(filter.geometry_types.size(), 1);
    EXPECT_EQ(filter.geometry_types[0], nova::geometry_type::gui);

    EXPECT_EQ(filter.names.size(), 0);
    EXPECT_EQ(filter.name_parts.size(), 0);
    nova::nova_renderer::deinit();
}

nlohmann::json get_gui_def_json() {
    return {
            { "name", "gui" },
            { "filters", { "geometry_type::gui" } },
            { "fallback", "gbuffers_textured" }
    };
};

void add_shader_source_to_definition(nova::shader_definition& def) {
    auto version_line = nova::shader_line{0, "gbuffers_basic.vert", "#version 450"};
    auto lines = std::vector<nova::shader_line>{version_line};

    def.vertex_source = lines;
    def.fragment_source = lines;
}
