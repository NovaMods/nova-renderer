/*!
 * \brief Tests the shader facade
 *
 * \author ddubois 
 * \date 26-Oct-16.
 */

#include <gtest/gtest.h>

#include <model/shaders/shader_facade.h>

TEST(shader_tree_node, calculate_filters) {
    auto node = nova::model::shader_tree_node("test");
    auto loaded_shaders = std::vector<std::string>{"test"};
    auto filters = std::unordered_map<std::string, std::function<bool(const nova::model::render_object&)>>{
            {"test", [](const auto& geom) {return true;}}
    };

    node.calculate_filters(filters, loaded_shaders);

    auto filter_function = node.get_filter_function();

    
}
