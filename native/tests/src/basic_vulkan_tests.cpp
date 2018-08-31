//
// Created by jannis on 30.08.18.
//

#include <gtest/gtest.h>
#include "../../src/nova_renderer.hpp"
#include "../../src/render_engine/vulkan/vulkan_render_engine.hpp"

TEST(nova_vulkan, basic_init) {
    auto render_engine = std::make_shared<nova::vulkan_render_engine>();
    nova::settings_options options;
    options.application_name = "Test app";
    options.application_version = {1, 0, 0};
    nova::nova_renderer *renderer = nova::nova_renderer::initialize(render_engine, options);
}