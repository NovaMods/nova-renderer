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

TEST(graphics_pipeline_validator, no_warnings_or_errors) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(graphics_pipeline_validator, missing_name) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline <NAME_MISSING>: Missing field name");
}

TEST(graphics_pipeline_validator, missing_pass) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field pass");
}

TEST(graphics_pipeline_validator, missing_vertex_fields) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field vertexFields");
}

TEST(graphics_pipeline_validator, missing_vertex_shader) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field vertexShader");
}

TEST(graphics_pipeline_validator, missing_parent_name) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field parentName. A default value of '\"\"' will be used");
}

TEST(graphics_pipeline_validator, missing_defines) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field defines. A default value of 'null' will be used");
}

TEST(graphics_pipeline_validator, missing_states) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field states. A default value of 'null' will be used");
}

TEST(graphics_pipeline_validator, missing_front_face) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field frontFace. A default value of 'null' will be used");
}

TEST(graphics_pipeline_validator, missing_back_face) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field backFace. A default value of 'null' will be used");
}

TEST(graphics_pipeline_validator, missing_fallback) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field fallback. A default value of '\"\"' will be used");
}

TEST(graphics_pipeline_validator, missing_depth_bias) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field depthBias. A default value of '0' will be used");
}

TEST(graphics_pipeline_validator, missing_slope_scaled_depth_bias) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field slopeScaledDepthBias. A default value of '0' will be used");
}

TEST(graphics_pipeline_validator, missing_stencil_ref) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field stencilRef. A default value of '0' will be used");
}

TEST(graphics_pipeline_validator, missing_stencil_read_mask) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field stencilReadMask. A default value of '0' will be used");
}

TEST(graphics_pipeline_validator, missing_stencil_write_mask) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field stencilWriteMask. A default value of '0' will be used");
}

TEST(graphics_pipeline_validator, missing_msaa_support) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field msaaSupport. A default value of '\"None\"' will be used");
}

TEST(graphics_pipeline_validator, missing_source_blend_factor) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0],
              "Pipeline TestPipeline: Missing field sourceBlendFactor. A default value of '\"One\"' will be used");
}

TEST(graphics_pipeline_validator, missing_destination_blend_factor) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field destinationBlendFactor. A default value of '\"Zero\"' will be used");
}

TEST(graphics_pipeline_validator, missing_alpha_src) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field alphaSrc. A default value of '\"One\"' will be used");
}

TEST(graphics_pipeline_validator, missing_alpha_dst) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field alphaDst. A default value of '\"Zero\"' will be used");

}

TEST(graphics_pipeline_validator, missing_depth_func) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field depthFunc. A default value of '\"Less\"' will be used");
}

TEST(graphics_pipeline_validator, missing_render_queue) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field renderQueue. A default value of '\"Opaque\"' will be used");
}

TEST(graphics_pipeline_validator, missing_tessellation_control_shader) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field tessellationControlShader. A default value of '\"\"' will be used");
}

TEST(graphics_pipeline_validator, missing_tessellation_evaluation_shader) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field tessellationEvaluationShader. A default value of '\"\"' will be used");
}

TEST(graphics_pipeline_validator, missing_geometry_shader) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field geometryShader. A default value of '\"\"' will be used");
}

TEST(graphics_pipeline_validator, missing_fragment_shader) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_graphics_pipeline(pipeline);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Pipeline TestPipeline: Missing field fragmentShader. A default value of '\"\"' will be used");
}

/************************************************
 *      Dynamic resources validator tests       *
 ************************************************/

TEST(resources_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_shaderpack_resources_data(resources);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(resources_validator, texture_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
        { "name",     "TestSampler" },
        { "filter",   "Bilinear" },
        { "wrapMode", "Clamp" }
    };


    nlohmann::json resources;
    resources["samplers"] = nlohmann::json::array({sampler});
    
    nova::validation_report report = nova::validate_shaderpack_resources_data(resources);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Resources file: Missing dynamic resources. If you ONLY use the backbuffer in your shaderpack, you can ignore this message");
}

TEST(resources_validator, samplers_missing) {
    TEST_SETUP_LOGGER();

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

    nova::validation_report report = nova::validate_shaderpack_resources_data(resources);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    
    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Resources file: No samplers defined, but dynamic textures are defined. You need to define your own samplers to access a texture with");
}

TEST(resources_validator, texture_warnings_propagate) {
    TEST_SETUP_LOGGER();

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

    nova::validation_report report = nova::validate_shaderpack_resources_data(resources);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default value of '\"RGBA8\"' will be used");
    
    EXPECT_EQ(resources.at("textures").at(0).at("format").at("pixelFormat"), "RGBA8");
}

TEST(resources_validator, sampler_errors_propagate) {
    TEST_SETUP_LOGGER();

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

    nova::validation_report report = nova::validate_shaderpack_resources_data(resources);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field filter");
}

/************************************
 *      Texture validator tests     *
 ************************************/

TEST(texture_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_texture_data(texture);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(texture_validator, name_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture = {
            {"format", {
                             {"pixelFormat", "RGBA8"},
                             {"dimensionType", "Absolute"},
                             {"width", 1920},
                             {"height", 1080}
                     }}
    };

    nova::validation_report report = nova::validate_texture_data(texture);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Texture <NAME_MISSING>: Missing field name");
}

TEST(texture_validator, format_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture = {
            {"name", "TestTexture"}
    };

    nova::validation_report report = nova::validate_texture_data(texture);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Texture TestTexture: Missing field format");
}

TEST(texture_validator, texture_format_warnings_propagate) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture = {
            {"name", "TestTexture"},
            {"format", {
                             {"dimensionType", "Absolute"},
                             {"width", 1920},
                             {"height", 1080}
                     }}
    };

    nova::validation_report report = nova::validate_texture_data(texture);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default value of '\"RGBA8\"' will be used");

    EXPECT_EQ(texture.at("format").at("pixelFormat").get<std::string>(), "RGBA8");
}

/********************************************
 *      Texture format validation tests     *
 ********************************************/

TEST(texture_format_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"width", 1920},
            {"height", 1080}
    };

    const nova::validation_report report = nova::validate_texture_format(texture_format, "TestTexture");
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(texture_format_validator, pixel_format_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"dimensionType", "Absolute"},
            {"width", 1920},
            {"height", 1080}
    };

    const nova::validation_report report = nova::validate_texture_format(texture_format, "TestTexture");
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default value of '\"RGBA8\"' will be used");

    EXPECT_EQ(texture_format.at("pixelFormat").get<std::string>(), "RGBA8");
}

TEST(texture_format_validator, dimension_type_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"width", 1920},
            {"height", 1080}
    };

    const nova::validation_report report = nova::validate_texture_format(texture_format, "TestTexture");
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field dimensionType. A default value of '\"Absolute\"' will be used");

    EXPECT_EQ(texture_format.at("dimensionType").get<std::string>(), "Absolute");
}

TEST(texture_format_validator, width_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"height", 1080}
    };

    const nova::validation_report report = nova::validate_texture_format(texture_format, "TestTexture");
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field width");
}

TEST(texture_format_validator, height_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"width", 1920}
    };

    const nova::validation_report report = nova::validate_texture_format(texture_format, "TestTexture");
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field height");
}

/****************************************
 *      Sampler validation tests        *
 ****************************************/
 
TEST(sampler_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();
    
    nlohmann::json sampler = {
        { "name",     "TestSampler" },
        { "filter",   "Bilinear" },
        { "wrapMode", "Clamp" }
    };

    const nova::validation_report report = nova::validate_sampler_data(sampler);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(sampler_validator, missing_name) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
            {"filter", "Bilinear"},
            {"wrapMode", "Clamp"}
    };

    const nova::validation_report report = nova::validate_sampler_data(sampler);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler <NAME_MISSING>: Missing field name");
}

TEST(sampler_validator, missing_filter) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
            {"name", "TestSampler"},
            {"wrapMode", "Clamp"}
    };

    const nova::validation_report report = nova::validate_sampler_data(sampler);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field filter");
}

TEST(sampler_validator, missing_wrap_mode) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
            {"name", "TestSampler"},
            {"filter", "Bilinear"}
    };

    const nova::validation_report report = nova::validate_sampler_data(sampler);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field wrapMode");
}

/****************************************
 *      Material validation tests       *
 ****************************************/

TEST(material_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

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
                                {"ModelMatrix", "NovaPerModelUBO"}
                            }
                        }
                    }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(material_validator, bindings_missing) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Material pass main in material TestMaterial: Missing field bindings");
}

TEST(material_validator, bindings_empty) {
    TEST_SETUP_LOGGER();

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

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Material pass main in material TestMaterial: Field bindings exists but it's empty");
}

TEST(material_validator, filter_missing) {
    TEST_SETUP_LOGGER();

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
                                {"ModelMatrix", "NovaPerModelUBO"}
                            }
                        }
                    }
                }
            }
        }
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Missing geometry filter");
}

TEST(material_validator, name_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
        {"passes",
            {
                {
                    {"name", "main"},
                    {"pipeline", "TexturedLit"},
                    {"bindings",
                        {
                            {
                                 {"ModelMatrix", "NovaPerModelUBO"}
                            }
                        }
                    }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material <NAME_MISSING>: Missing material name");
}

TEST(material_validator, passes_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Missing material passes");
}

TEST(material_validator, passes_wrong_type) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes", 42},
        {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Passes field must be an array");
}

TEST(material_validator, passes_empty_array) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", std::vector<std::string>{}},
            {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Passes field must have at least one item");
}

TEST(material_validator, pass_no_pipeline) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes",
            {
                {
                    {"name", "main"},
                    {"bindings",
                        {
                            {
                                {"ModelMatrix", "NovaPerModelUBO"}
                            }
                        }
                    }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material pass main in material TestMaterial: Missing field pipeline");
}

TEST(material_validator, pass_no_name) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes",
            {
                {
                    {"pipeline", "TexturedLit"},
                    {"bindings",
                        {
                            {
                                {"ModelMatrix", "NovaPerModelUBO"}
                            }
                        }
                     }
                }
            }
        },
        {"filter", "geometry_type::block"}
    };

    const nova::validation_report report = nova::validate_material(material);
    nova::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material pass <NAME_MISSING> in material TestMaterial: Missing field name");
}
