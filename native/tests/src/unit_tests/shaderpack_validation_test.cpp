/*!
 * \author ddubois
 * \date 9-Oct-2018
 */

#include <gtest/gtest.h>
#include "../general_test_setup.hpp"

#include "../../../src/loading/shaderpack/shaderpack_validator.hpp"

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

    EXPECT_EQ(report.errors.size(), 0);
    EXPECT_EQ(report.warnings.size(), 0);
}

/*!
 * \brief Tests the material validator's warning about missing bindings
 */
TEST(nova_shaderpack_validation, material_warning) {
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