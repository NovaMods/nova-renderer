/*!
 * \author ddubois
 * \date 9-Oct-2018
 */

#include "../../../src/general_test_setup.hpp"

#include "../../../../src/loading/shaderpack/shaderpack_validator.hpp"
#undef TEST
#include <gtest/gtest.h>

/****************************************
 *      Pipeline validator tests        *
 ****************************************/

TEST(GraphicsPipelineValidator, NoWarningsOrErrors) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "ParentOfTestPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
                {"failOp", "Keep"},
                {"passOp", "Keep"},
                {"depthFailOp", "Replace"},
                {"compareOp", "Less"},
                {"compareMask", 0xFF},
                {"writeMask", 0xFF}
            }
        },
        {"backFace",
            {
                {"failOp", "Keep"},
                {"passOp", "Keep"},
                {"depthFailOp", "Replace"},
                {"compareOp", "Less"},
                {"compareMask", 0xFF},
                {"writeMask", 0xFF}
            }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        {"destinationBlendFactor", "Zero"},
        {"fallback", ""},
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::shaderpack::validation_report report = nova::renderer::shaderpack::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(GraphicsPipelineValidator, MissingName) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"parentName", "ParentOfTestPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline <NAME_MISSING>: Missing field name");
}

TEST(GraphicsPipelineValidator, MissingPass) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "ParentOfTestPipeline"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field pass");
}

TEST(GraphicsPipelineValidator, MissingVertexFields) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "ParentOfTestPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field vertexFields");
}

TEST(GraphicsPipelineValidator, MissingVertexShader) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "ParentOfTestPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field vertexShader");
}

TEST(GraphicsPipelineValidator, MissingParentName) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field parentName. A default value of '\"\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingDefines) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        { "states",
                 {"DisableDepthTest"}
        },
        { "vertexFields",
                 {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
                 {
                  {"failOp", "Keep"},
                                   {"passOp", "Keep"},
                                     {"depthFailOp", "Replace"},
                                               {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
                 {
                  {"failOp", "Keep"},
                                   {"passOp", "Keep"},
                                     {"depthFailOp", "Replace"},
                                               {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field defines. A default value of '[]' will be used");
}

TEST(GraphicsPipelineValidator, MissingStates) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field states. A default value of '[]' will be used");
}

TEST(GraphicsPipelineValidator, MissingFrontFace) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field frontFace. A default value of '{}' will be used");
}

TEST(GraphicsPipelineValidator, MissingBackFace) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field backFace. A default value of '{}' will be used");
}

TEST(GraphicsPipelineValidator, MissingFallback) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field fallback. A default value of '\"\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingDepthBias) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field depthBias. A default value of '0' will be used");
}

TEST(GraphicsPipelineValidator, MissingSlopeScaledDepthBias) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field slopeScaledDepthBias. A default value of '0' will be used");
}

TEST(GraphicsPipelineValidator, MissingStencilRef) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field stencilRef. A default value of '0' will be used");
}

TEST(GraphicsPipelineValidator, MissingStencilReadMask) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field stencilReadMask. A default value of '0' will be used");
}

TEST(GraphicsPipelineValidator, MissingStencilWriteMask) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field stencilWriteMask. A default value of '0' will be used");
}

TEST(GraphicsPipelineValidator, MissingMsaaSupport) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field msaaSupport. A default value of '\"None\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingSourceBlendFactor) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        { "name",                         "TestPipeline" },
        { "parentName",                   "TestParentPipeline" },
        { "pass",                         "TestPass" },
        { "defines",
                                          { "USE_NORMALMAP", "USE_SPECULAR" }
        },
        { "states",
                                          { "DisableDepthTest" }
        },
        { "vertexFields",
                                          { "Position",      "UV0", "Normal", "Tangent" }
        },
        { "frontFace",
                                          {
                                            { "failOp", "Keep" },
                                                             { "passOp", "Keep" },
                                                                    { "depthFailOp", "Replace" },
                                                                              { "compareOp", "Less" },
                                              { "compareMask", 0xFF },
                                              { "writeMask", 0xFF }
                                          }
        },
        { "backFace",
                                          {
                                            { "failOp", "Keep" },
                                                             { "passOp", "Keep" },
                                                                    { "depthFailOp", "Replace" },
                                                                              { "compareOp", "Less" },
                                              { "compareMask", 0xFF },
                                              { "writeMask", 0xFF }
                                          }
        },
        { "depthBias",                    0 },
        { "slopeScaledDepthBias",         0.01 },
        { "stencilRef",                   0 },
        { "stencilReadMask",              0xFF },
        { "stencilWriteMask",             0xFF },
        { "msaaSupport",                  "None" },
        { "primitiveMode",                "Triangles" },
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        { "alphaSrc",                     "One" },
        { "alphaDst",                     "Zero" },
        { "depthFunc",                    "Less" },
        { "renderQueue",                  "Opaque" },
        { "vertexShader",                 "TestVertexShader" },
        { "geometryShader",               "TestGeometryShader" },
        { "tessellationControlShader",    "TestTessellationControlShader" },
        { "tessellationEvaluationShader", "TestTessellationEvaluationShader" },
        { "fragmentShader",               "TestFragmentShader" },
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field sourceBlendFactor. A default value of '\"One\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingDestinationBlendFactor) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0],
              "Pipeline TestPipeline: Missing field destinationBlendFactor. A default value of '\"Zero\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingAlphaSrc) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field alphaSrc. A default value of '\"One\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingAlphaDst) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field alphaDst. A default value of '\"Zero\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingDepthFunc) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field depthFunc. A default value of '\"Less\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingRenderQueue) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field renderQueue. A default value of '\"Opaque\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingTessellationControlShader) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        {"destinationBlendFactor", "Zero" },
        {"fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field tessellationControlShader. A default value of '\"\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingTessellationEvaluationShader) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0],
              "Pipeline TestPipeline: Missing field tessellationEvaluationShader. A default value of '\"\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingGeometryShader) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
        {"fragmentShader", "TestFragmentShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field geometryShader. A default value of '\"\"' will be used");
}

TEST(GraphicsPipelineValidator, MissingFragmentShader) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json pipeline = {
        {"name", "TestPipeline"},
        {"parentName", "TestParentPipeline"},
        {"pass", "TestPass"},
        {"defines",
            {"USE_NORMALMAP", "USE_SPECULAR"}
        },
        { "states",
            {"DisableDepthTest"}
        },
        { "vertexFields",
            {"Position", "UV0", "Normal", "Tangent"}
        },
        {"frontFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"backFace",
            {
             {"failOp", "Keep"},
                              {"passOp", "Keep"},
                                {"depthFailOp", "Replace"},
                                          {"compareOp", "Less"},
                     {"compareMask", 0xFF},
                     {"writeMask", 0xFF}
                 }
        },
        {"depthBias", 0},
        {"slopeScaledDepthBias", 0.01},
        {"stencilRef", 0},
        {"stencilReadMask", 0xFF},
        {"stencilWriteMask", 0xFF},
        {"msaaSupport", "None"},
        {"primitiveMode", "Triangles"},
        {"sourceBlendFactor", "One"},
        { "destinationBlendFactor", "Zero" },
        { "fallback", "" },
        {"alphaSrc", "One"},
        {"alphaDst", "Zero"},
        {"depthFunc", "Less"},
        {"renderQueue", "Opaque"},
        {"vertexShader", "TestVertexShader"},
        {"geometryShader", "TestGeometryShader"},
        {"tessellationControlShader", "TestTessellationControlShader"},
        {"tessellationEvaluationShader", "TestTessellationEvaluationShader"},
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::shaderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field fragmentShader. A default value of '\"\"' will be used");
}

/************************************************
 *      Dynamic resources validator tests       *
 ************************************************/

TEST(ResourcesValidator, NoErrorsOrWarnings) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json resources = {
        { "textures",
            {
                {
                    { "name", "TestTexture" },
                    { "format",
                        {
                            { "pixelFormat", "RGBA8" },
                            { "dimensionType", "Absolute" },
                            { "width", 1920 },
                            { "height", 1080 }
                        }
                    }
                }
            }
        },
        { "samplers",
            {
                {
                    { "name",     "TestSampler" },
                    { "filter",   "Bilinear" },
                    { "wrapMode", "Clamp" }
                }
            }
        }
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_shaderpack_resources_data(resources);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(ResourcesValidator, TextureMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json sampler = {
        { "name",     "TestSampler" },
        { "filter",   "Bilinear" },
        { "wrapMode", "Clamp" }
    };
    // clang-format on

    nlohmann::json resources;
    resources["samplers"] = nlohmann::json::array({sampler});

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_shaderpack_resources_data(resources);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0],
              "Resources file: Missing dynamic resources. If you ONLY use the backbuffer in your shaderpack, you can ignore this message");
}

TEST(ResourcesValidator, SamplersMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json resources = {
        { "textures",
            {
                {
                    { "name", "TestTexture" },
                    { "format",
                        {
                            { "pixelFormat", "RGBA8" },
                            { "dimensionType", "Absolute" },
                            { "width", 1920 },
                            { "height", 1080 }
                        }
                    }
                }
            }
        }
    };
    // clang-format on

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_shaderpack_resources_data(resources);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(
        report.errors[0],
        "Resources file: No samplers defined, but dynamic textures are defined. You need to define your own samplers to access a texture with");
}

TEST(ResourcesValidator, TextureWarningsPropagate) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json resources = {
        { "textures",
            {
                {
                    { "name", "TestTexture" },
                              { "format",
                                  {
                                      { "dimensionType", "Absolute" },
                                      { "width", 1920 },
                                      { "height", 1080 }
                                  }
                              }
                }
            }
        },
        { "samplers",
            {
                {
                    { "name",     "TestSampler" },
                    { "filter",   "Bilinear" },
                    { "wrapMode", "Clamp" }
                }
            }
        }
    };
    // clang-format on

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_shaderpack_resources_data(resources);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default value of '\"RGBA8\"' will be used");

    EXPECT_EQ(resources.at("textures").at(0).at("format").at("pixelFormat"), "RGBA8");
}

TEST(ResourcesValidator, SamplerErrorsPropagate) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json resources = {
        { "textures",
            {
                {
                    { "name", "TestTexture" },
                              { "format",
                                  {
                                      { "pixelFormat", "RGBA8" },
                                      { "dimensionType", "Absolute" },
                                      { "width", 1920 },
                                      { "height", 1080 }
                                  }
                              }
                }
            }
        },
        { "samplers",
            {
                {
                    { "name",     "TestSampler" },
                    { "wrapMode", "Clamp" }
                }
            }
        }
    };
    // clang-format on

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_shaderpack_resources_data(resources);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field filter");
}

/************************************
 *      Texture validator tests     *
 ************************************/

TEST(TextureValidator, NoErrorsOrWarnings) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture = {
        { "name", "TestTexture" },
        { "format",
             {
                 { "pixelFormat", "RGBA8" },
                 { "dimensionType", "Absolute" },
                 { "width", 1920 },
                 { "height", 1080 }
             }
        }
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_data(texture);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(TextureValidator, NameMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture = {
            {"format", {
                             {"pixelFormat", "RGBA8"},
                             {"dimensionType", "Absolute"},
                             {"width", 1920},
                             {"height", 1080}
                     }}
    };
    // clang-format on

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_data(texture);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Texture <NAME_MISSING>: Missing field name");
}

TEST(TextureValidator, FormatMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture = {
            {"name", "TestTexture"}
    };
    // clang-format on

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_data(texture);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Texture TestTexture: Missing field format");
}

TEST(TextureValidator, TextureFormatWarningsPropagate) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture = {
            {"name", "TestTexture"},
            {"format", {
                             {"dimensionType", "Absolute"},
                             {"width", 1920},
                             {"height", 1080}
                     }}
    };
    // clang-format on

    nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_data(texture);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default value of '\"RGBA8\"' will be used");

    EXPECT_EQ(texture.at("format").at("pixelFormat").get<std::string>(), "RGBA8");
}

/********************************************
 *      Texture format validation tests     *
 ********************************************/

TEST(TextureFormatValidator, NoErrorsOrWarnings) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"width", 1920},
            {"height", 1080}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_format(texture_format, "TestTexture");
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(TextureFormatValidator, PixelFormatMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture_format = {
            {"dimensionType", "Absolute"},
            {"width", 1920},
            {"height", 1080}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_format(texture_format, "TestTexture");
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default value of '\"RGBA8\"' will be used");

    EXPECT_EQ(texture_format.at("pixelFormat").get<std::string>(), "RGBA8");
}

TEST(TextureFormatValidator, DimensionTypeMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"width", 1920},
            {"height", 1080}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_format(texture_format, "TestTexture");
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0],
              "Format of texture TestTexture: Missing field dimensionType. A default value of '\"Absolute\"' will be used");

    EXPECT_EQ(texture_format.at("dimensionType").get<std::string>(), "Absolute");
}

TEST(TextureFormatValidator, WidthMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"height", 1080}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_format(texture_format, "TestTexture");
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field width");
}

TEST(TextureFormatValidator, HeightMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"width", 1920}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_texture_format(texture_format, "TestTexture");
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field height");
}

/****************************************
 *      Sampler validation tests        *
 ****************************************/

TEST(SamplerValidator, NoErrorsOrWarnings) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json sampler = {
        { "name",     "TestSampler" },
        { "filter",   "Bilinear" },
        { "wrapMode", "Clamp" }
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_sampler_data(sampler);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(SamplerValidator, MissingName) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json sampler = {
            {"filter", "Bilinear"},
            {"wrapMode", "Clamp"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_sampler_data(sampler);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler <NAME_MISSING>: Missing field name");
}

TEST(SamplerValidator, MissingFilter) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json sampler = {
            {"name", "TestSampler"},
            {"wrapMode", "Clamp"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_sampler_data(sampler);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field filter");
}

TEST(SamplerValidator, MissingWrapMode) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json sampler = {
            {"name", "TestSampler"},
            {"filter", "Bilinear"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_sampler_data(sampler);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field wrapMode");
}

/****************************************
 *      Material validation tests       *
 ****************************************/

TEST(MaterialValidator, NoErrorsOrWarnings) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes", 
            {
                {
                    {"name", "main"},
                    {"pipeline", "TexturedLit"},
                    {"bindings",
                        {
                            {
                                {"ModelMatrix", "NovaModelMatrixBuffer"}
                            }
                        }
                    }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(MaterialValidator, BindingsMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes",
            {
                {
                    {"name", "main"},
                    {"pipeline", "TexturedLit"}
                }
            }
        },
        {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Material pass main in material TestMaterial: Missing field bindings");
}

TEST(MaterialValidator, BindingsEmpty) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        { "name", "TestMaterial" },
        { "passes",
            {
                {
                    { "name", "main" },
                    { "pipeline", "TexturedLit" },
                    { "bindings", {} }
                }
            }
        },
        { "filter", "geometry_type::block" }
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Material pass main in material TestMaterial: Field bindings exists but it's empty");
}

TEST(MaterialValidator, FilterMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes",
            {
                {
                    {"name", "main"},
                    {"pipeline", "TexturedLit"},
                    {"bindings",
                        {
                            {
                                {"ModelMatrix", "NovaModelMatrixBuffer"}
                            }
                        }
                    }
                }
            }
        }
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Missing geometry filter");
}

TEST(MaterialValidator, NameMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"passes",
            {
                {
                    {"name", "main"},
                    {"pipeline", "TexturedLit"},
                    {"bindings",
                        {
                            {
                                 {"ModelMatrix", "NovaModelMatrixBuffer"}
                            }
                        }
                    }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material <NAME_MISSING>: Missing material name");
}

TEST(MaterialValidator, PassesMissing) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Missing material passes");
}

TEST(MaterialValidator, PassesWrongType) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes", 42},
        {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Passes field must be an array");
}

TEST(MaterialValidator, PassesEmptyArray) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", std::vector<std::string>{}},
            {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Passes field must have at least one item");
}

TEST(MaterialValidator, PassNoPipeline) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes",
            {
                {
                    {"name", "main"},
                    {"bindings",
                        {
                            {
                                {"ModelMatrix", "NovaModelMatrixBuffer"}
                            }
                        }
                    }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material pass main in material TestMaterial: Missing field pipeline");
}

TEST(MaterialValidator, PassNoName) {
    TEST_SETUP_LOGGER();

    // clang-format off
    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes",
            {
                {
                    {"pipeline", "TexturedLit"},
                    {"bindings",
                        {
                            {
                                {"ModelMatrix", "NovaModelMatrixBuffer"}
                            }
                        }
                     }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };
    // clang-format on

    const nova::renderer::shaderpack::validation_report report = nova::renderer::validate_material(material);
    nova::renderer::shaderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material pass <NAME_MISSING> in material TestMaterial: Missing field name");
}
