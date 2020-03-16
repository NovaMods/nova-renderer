#include "nova_renderer/util/utils.hpp"

#include "../../../../src/loading/renderpack/renderpack_validator.hpp"
#include "../../../src/general_test_setup.hpp"
#undef TEST
#include <gtest/gtest.h>
#include <string.h>

/****************************************
 *      Pipeline validator tests        *
 ****************************************/

RX_LOG("renderpackValidatorTests", logger);

TEST(GraphicsPipelineValidator, NoWarningsOrErrors) {
    rx::json pipeline_json{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const auto err = pipeline_json.error();
    if(err) {
        logger->error("Could not create JSON resource: %s", *err);
    }

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline_json);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(GraphicsPipelineValidator, MissingName) {
    rx::json pipeline{R"({
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline <NAME_MISSING>: Missing field name");
}

TEST(GraphicsPipelineValidator, MissingPass) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field pass");
}

TEST(GraphicsPipelineValidator, MissingVertexShader) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Pipeline TestPipeline: Missing field vertexShader");
}

TEST(GraphicsPipelineValidator, MissingParentName) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingDefines) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingStates) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingFrontFace) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingBackFace) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingDepthBias) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingSlopeScaledDepthBias) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingStencilRef) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingStencilReadMask) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingStencilWriteMask) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingMsaaSupport) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingSourceBlendFactor) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingDestinationBlendFactor) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingAlphaSrc) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingAlphaDst) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingDepthFunc) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingTessellationControlShader) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingTessellationEvaluationShader) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingGeometryShader) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader",
        "fragmentShader": "TestFragmentShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(GraphicsPipelineValidator, MissingFragmentShader) {
    rx::json pipeline{R"({
        "name": "TestPipeline",
        "parentName": "ParentOfTestPipeline",
        "pass": "TestPass",
        "defines": [
            "USE_NORMALMAP",
            "USE_SPECULAR"
        ],
        "states": [
            "DisableDepthTest"
        ],
        "vertexFields": [
            "Position",
            "UV0",
            "Normal",
            "Tangent"
        ],
        "frontFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "backFace": {
            "failOp": "Keep",
            "passOp": "Keep",
            "depthFailOp": "Replace",
            "compareOp": "Less",
            "compareMask": 255,
            "writeMask": 255
        },
        "depthBias": 0,
        "slopeScaledDepthBias": 0.01,
        "stencilRef": 0,
        "stencilReadMask": 255,
        "stencilWriteMask": 255,
        "msaaSupport": "None",
        "primitiveMode": "Triangles",
        "sourceBlendFactor": "One",
        "destinationBlendFactor": "Zero",
        "fallback": "",
        "alphaSrc": "One",
        "alphaDst": "Zero",
        "depthFunc": "Less",
        "renderQueue": "Opaque",
        "vertexShader": "TestVertexShader",
        "geometryShader": "TestGeometryShader",
        "tessellationControlShader": "TestTessellationControlShader",
        "tessellationEvaluationShader": "TestTessellationEvaluationShader"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_graphics_pipeline(pipeline);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

/************************************************
 *      Dynamic resources validator tests       *
 ************************************************/

TEST(ResourcesValidator, NoErrorsOrWarnings) {
    rx::json resources{R"({
        "textures": [
            {
                "name": "TestTexture",
                "format": {
                    "pixelFormat": "RGBA8",
                    "dimensionType": "Absolute",
                    "width": 1920,
                    "height": 1080
                }
            }
        ],
        "samplers": [
            {
                "name": "TestSampler",
                "filter": "Bilinear",
                "wrapMode": "Clamp"
            }
        ]
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_renderpack_resources_data(resources);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(ResourcesValidator, TextureMissing) {
    rx::json resources{R"({
        "samplers": [
            {
                "name": "TestSampler",
                "filter": "Bilinear",
                "wrapMode": "Clamp"
            }
        ]
    })"};

    nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_renderpack_resources_data(resources);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(ResourcesValidator, TextureWarningsPropagate) {
    rx::json resources{R"({
        "textures": [
            {
                "name": "TestTexture",
                "format": {
                    "dimensionType": "Absolute",
                    "width": 1920,
                    "height": 1080
                }
            }
        ],
        "samplers": [
            {
                "name": "TestSampler",
                "filter": "Bilinear",
                "wrapMode": "Clamp"
            }
        ]
    })"};

    nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_renderpack_resources_data(resources);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(ResourcesValidator, SamplerErrorsPropagate) {
    rx::json resources{R"({
        "textures": [
            {
                "name": "TestTexture",
                "format": {
                    "pixelFormat": "RGBA8",
                    "dimensionType": "Absolute",
                    "width": 1920,
                    "height": 1080
                }
            }
        ],
        "samplers": [
            {
                "name": "TestSampler",
                "wrapMode": "Clamp"
            }
        ]
    })"};

    nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_renderpack_resources_data(resources);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

/************************************
 *      Texture validator tests     *
 ************************************/

TEST(TextureValidator, NoErrorsOrWarnings) {
    rx::json texture{R"({
        "name": "TestTexture",
        "format": {
            "pixelFormat": "RGBA8",
            "dimensionType": "Absolute",
            "width": 1920,
            "height": 1080
        }
    })"};

    const auto err = texture.error();
    if(err) {
        logger->error("%s", *err);
    }

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_data(texture);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(TextureValidator, NameMissing) {
    rx::json texture{R"({
        "format": {
            "pixelFormat": "RGBA8",
            "dimensionType": "Absolute",
            "width": 1920,
            "height": 1080
        }
    })"};

    nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_data(texture);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(TextureValidator, FormatMissing) {
    rx::json texture{R"({
        "name": "TestTexture"
    })"};

    nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_data(texture);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(TextureValidator, TextureFormatWarningsPropagate) {
    rx::json texture{R"({
        "name": "TestTexture",
        "format": {
            "dimensionType": "Absolute",
            "width": 1920,
            "height": 1080
        }
    })"};

    nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_data(texture);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

/********************************************
 *      Texture format validation tests     *
 ********************************************/

TEST(TextureFormatValidator, NoErrorsOrWarnings) {
    rx::json texture_format{R"({
        "pixelFormat": "RGBA8",
        "dimensionType": "Absolute",
        "width": 1920,
        "height": 1080
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_format(texture_format,
                                                                                                                    "TestTexture");
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(TextureFormatValidator, PixelFormatMissing) {
    rx::json texture_format{R"({
        "dimensionType": "Absolute",
        "width": 1920,
        "height": 1080
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_format(texture_format,
                                                                                                                    "TestTexture");
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(TextureFormatValidator, DimensionTypeMissing) {
    rx::json texture_format{R"({
        "pixelFormat": "RGBA8",
        "width": 1920,
        "height": 1080
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_format(texture_format,
                                                                                                                    "TestTexture");
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(TextureFormatValidator, WidthMissing) {
    rx::json texture_format{R"({
        "pixelFormat": "RGBA8",
        "dimensionType": "Absolute",
        "height": 1080
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_format(texture_format,
                                                                                                                    "TestTexture");
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(TextureFormatValidator, HeightMissing) {
    rx::json texture_format{R"({
        "pixelFormat": "RGBA8",
        "dimensionType": "Absolute",
        "width": 1920
    })"};

    if(const auto err = texture_format.error()) {
        logger->error("%s", *err);
    }

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_texture_format(texture_format,
                                                                                                                    "TestTexture");
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

/****************************************
 *      Sampler validation tests        *
 ****************************************/

TEST(SamplerValidator, NoErrorsOrWarnings) {
    rx::json sampler{R"({
        "name": "TestSampler",
        "filter": "Bilinear",
        "wrapMode": "Clamp"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_sampler_data(sampler);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(SamplerValidator, MissingName) {
    rx::json sampler{R"({
        "filter": "Bilinear",
        "wrapMode": "Clamp"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_sampler_data(sampler);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(SamplerValidator, MissingFilter) {
    rx::json sampler{R"({
        "name": "TestSampler",
        "wrapMode": "Clamp"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_sampler_data(sampler);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(SamplerValidator, MissingWrapMode) {
    rx::json sampler{R"({
        "name": "TestSampler",
        "filter": "Bilinear"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_sampler_data(sampler);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

/****************************************
 *      Material validation tests       *
 ****************************************/

TEST(MaterialValidator, NoErrorsOrWarnings) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [
            {
                "name": "main",
                "pipeline": "TexturedLit",
                "bindings": [
                    {
                        "variable": "ModelMatrix",
                        "resource": "NovaModelMatrixBuffer"
                    }
                ]
            }
        ],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

TEST(MaterialValidator, BindingsMissing) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [
            {
                "name": "main",
                "pipeline": "TexturedLit"
            }
        ],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(MaterialValidator, BindingsEmpty) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [
            {
                "name": "main",
                "pipeline": "TexturedLit",
                "bindings": []
            }
        ],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
}

TEST(MaterialValidator, FilterMissing) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [
            {
                "name": "main",
                "pipeline": "TexturedLit",
                "bindings": [
                    {
                        "variable": "ModelMatrix",
                        "resource": "NovaModelMatrixBuffer"
                    }
                ]
            }
        ]
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(MaterialValidator, NameMissing) {
    rx::json material{R"({
        "passes": [
            {
                "name": "main",
                "pipeline": "TexturedLit",
                "bindings": [
                    {
                        "variable": "ModelMatrix",
                        "resource": "NovaModelMatrixBuffer"
                    }
                ]
            }
        ],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(MaterialValidator, PassesMissing) {
    rx::json material{R"({
        "name": "TestMaterial",
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(MaterialValidator, PassesWrongType) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes":42,
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(MaterialValidator, PassesEmptyArray) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(MaterialValidator, PassNoPipeline) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [
            {
                "name": "main",
                "bindings": [
                    {
                        "variable": "ModelMatrix",
                        "resource": "NovaModelMatrixBuffer"
                    }
                ]
            }
        ],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}

TEST(MaterialValidator, PassNoName) {
    rx::json material{R"({
        "name": "TestMaterial",
        "passes": [
            {
                "pipeline": "TexturedLit",
                "bindings": [
                    {
                        "variable": "ModelMatrix",
                        "resource": "NovaModelMatrixBuffer"
                    }
                ]
            }
        ],
        "filter": "geometry_type::block"
    })"};

    const nova::renderer::renderpack::ValidationReport report = nova::renderer::renderpack::validate_material(material);
    nova::renderer::renderpack::print(report);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
}
