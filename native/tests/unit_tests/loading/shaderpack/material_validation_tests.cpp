/*!
 * \author ddubois
 * \date 9-Oct-2018
 */

#include <gtest/gtest.h>
#include "../../../src/general_test_setup.hpp"

#include "../../../../src/loading/shaderpack/shaderpack_validator.hpp"

/*!
 * \brief Happy path test. The material JSON is well-formed and filled with the most wonderful data
 */
TEST(nova_shaderpack_validation, material_validation_no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
        {"name", "TestMaterial"},
        {"passes", {
            {"name", "main"},
            {"pipeline", "TexturedLit"},
            {"bindings", {
                {"ModelMatrix", "NovaPerModelUBO"}
            }}
        }},
        {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

/*!
 * \brief Tests the material validator's warning about missing bindings
 */
TEST(nova_shaderpack_validation, material_bindings_warning) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", {
                             {"name", "main"},
                             {"pipeline", "TexturedLit"}
                     }},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.errors.size(), 0);
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Material TestMaterial: No bindings defined");
}

/*!
 * \brief Tests the material validator's error about a missing filter
 */
TEST(nova_shaderpack_validation, material_filter_error) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", {
                             {"name", "main"},
                             {"pipeline", "TexturedLit"},
                             {"bindings", {
                                                  {"ModelMatrix", "NovaPerModelUBO"}
                                          }}
                     }}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Missing geometry filter");
}

TEST(nova_shaderpack_validation, material_name_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"passes", {
                             {"name", "main"},
                             {"pipeline", "TexturedLit"},
                             {"bindings", {
                                                  {"ModelMatrix", "NovaPerModelUBO"}
                                          }}
                     }},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material <NAME_MISSING>: Missing material name");
}

TEST(nova_shaderpack_validation, material_passes_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Missing material passes");
}

TEST(nova_shaderpack_validation, material_passes_wrong_type) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", 42},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Passes field must be an array");
}

TEST(nova_shaderpack_validation, material_passes_empty_array) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", std::vector<std::string>{}},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material TestMaterial: Passes field must have at least one item");
}

TEST(nova_shaderpack_validation, material_pass_no_pipeline) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", {
                             {"name", "main"},
                             {"bindings", {
                                                  {"ModelMatrix", "NovaPerModelUBO"}
                                          }}
                     }},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material pass main in material TestMaterial: Missing field pipeline");
}

TEST(nova_shaderpack_validation, material_pass_no_name) {
    TEST_SETUP_LOGGER();

    nlohmann::json material = {
            {"name", "TestMaterial"},
            {"passes", {
                             {"pipeline", "TexturedLit"},
                             {"bindings", {
                                                  {"ModelMatrix", "NovaPerModelUBO"}
                                          }}
                     }},
            {"filter", "geometry_type::block"}
    };

    const validation_report report = validate_material(material);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Material pass <NAME_MISSING> for material TestMaterial: Missing field name");
}
