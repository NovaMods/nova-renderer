/*!
 * \author ddubois
 * \date 9-Oct-2018
 */

#include <gtest/gtest.h>
#include "../../../src/general_test_setup.hpp"

#include "../../../../src/loading/shaderpack/shaderpack_validator.hpp"

/************************************************
 *      Dynamic resources validator tests       *
 ************************************************/

/*!
 * \brief Tests the happy path for the dynamic resources validator
 */
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
                            { "heignt", 1080 }
                        }
                    }
                }
            }
        },
        { "samplers",
            {
                { "name",     "TestSampler" },
                { "filter",   "Bilinear" },
                { "wrapMode", "Clamp" }
            }
        }
    };

    validation_report report = validate_shaderpack_resources_data(resources);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

/*!
 * \brief Tests that the resources validator can handle missing textures
 */
TEST(resources_validator, texture_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json resources = {
        { "samplers",
            {
                { "name",     "TestSampler" },
                { "filter",   "Bilinear" },
                { "wrapMode", "Clamp" }
            }
        }
    };

    validation_report report = validate_shaderpack_resources_data(resources);

    EXPECT_EQ(report.errors.size(), 0);
    
    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Resources file: Missing dynamic resources. If you ONLY use the backbuffer in your shaderpack, you can ignore this message");
}

/*!
 * \brief Tests that the resources validator can handle missing samplers
 */
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
                            { "heignt", 1080 }
                        }
                    }
                }
            }
        }
    };

    validation_report report = validate_shaderpack_resources_data(resources);

    EXPECT_EQ(report.warnings.size(), 0);
    
    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Resources file: No samplers defined, but dynamic textures are defined. You need to define your own samplers to access a texture with");
}

/*!
 * \brief Tests that texture validator warnings propagate to the resources validator
 */
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
                { "name",     "TestSampler" },
                { "filter",   "Bilinear" },
                { "wrapMode", "Clamp" }
            }
        }
    };

    validation_report report = validate_shaderpack_resources_data(resources);

    EXPECT_EQ(report.warnings.size(), 0);
    
    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field pixelFormat. A default of RGBA8 was used");
    
    EXPECT_EQ(resources.at("textures").at(0).at("format").at("pixelFormat"), "RGBA8");
}

/*!
 * \brief Tests that sampler validator errors properly propagate to the resources validator
 */
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
                                      { "heignt", 1080 }
                                  }
                              }
                }
            }
        },
        { "samplers",
            {
                { "name",     "TestSampler" },
                { "wrapMode", "Clamp" }
            }
        }
    };

    validation_report report = validate_shaderpack_resources_data(resources);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field filter");
}

/************************************
 *      Texture validator tests     *
 ************************************/

/*!
 * \brief Tests that the texture format validator correctly handles the happy path
 */
TEST(texture_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture = {
        { "name", "TestTexture" },
        { "format",
             {
                 { "pixelFormat", "RGBA8" },
                 { "dimensionType", "Absolute" },
                 { "width", 1920 },
                 { "heignt", 1080 }
             }
        }
    };

    validation_report report = validate_texture_data(texture);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

/*!
 * \brief Tests that the texture validator handles a missing name
 */
TEST(texture_validator, name_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture = {
            {"format", {
                             {"pixelFormat", "RGBA8"},
                             {"dimensionType", "Absolute"},
                             {"width", 1920},
                             {"heignt", 1080}
                     }}
    };

    validation_report report = validate_texture_data(texture);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Texture <NAME_MISSING>: Missing field name");
}

/*!
 * \brief Tests that the texture format validator handles a missing filter
 */
TEST(texture_validator, format_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture = {
            {"name", "TestTexture"}
    };

    validation_report report = validate_texture_data(texture);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Texture TestTexture: Missing field format");
}

/*!
 * \brief Tests that a warning in the format validator propagates to the texture validator
 */
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

    validation_report report = validate_texture_data(texture);

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default of RGBA8 was used");

    EXPECT_EQ(texture.at("format").at("pixelFormat").get<std::string>(), "RGBA8");
}

/********************************************
 *      Texture format validation tests     *
 ********************************************/

/*!
 * \brief Tests that the texture format validator doesn't give any errors for a well-formed texture format
 */
TEST(texture_format_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"width", 1920},
            {"heignt", 1080}
    };

    const validation_report report = validate_texture_format(texture_format, "TestTexture");

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

/*!
 * \brief Tests that the texture format validator gives the right warning for a missing pixel format
 */
TEST(texture_format_validator, pixel_format_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"dimensionType", "Absolute"},
            {"width", 1920},
            {"heignt", 1080}
    };

    const validation_report report = validate_texture_format(texture_format, "TestTexture");

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field pixelFormat. A default of RGBA8 was used");

    EXPECT_EQ(texture_format.at("pixelFormat").get<std::string>(), "RGBA8");
}

/*!
 * \brief Tests that the texture format validator gives the right warning for a missing dimension type
 */
TEST(texture_format_validator, dimension_type_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"width", 1920},
            {"heignt", 1080}
    };

    const validation_report report = validate_texture_format(texture_format, "TestTexture");

    EXPECT_EQ(report.errors.size(), 0);

    ASSERT_EQ(report.warnings.size(), 1);
    EXPECT_EQ(report.warnings[0], "Format of texture TestTexture: Missing field dimensionType. A default value of Absolute was used");

    EXPECT_EQ(texture_format.at("dimensionType").get<std::string>(), "Absolute");
}

/*!
 * \brief Tests that the texture format validator correctly checks for a missing width
 */
TEST(texture_format_validator, width_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"heignt", 1080}
    };

    const validation_report report = validate_texture_format(texture_format, "TestTexture");

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field width");
}

/*!
 * \brief Tests that the texture format validator correctly checks for a missing height
 */
TEST(texture_format_validator, height_missing) {
    TEST_SETUP_LOGGER();

    nlohmann::json texture_format = {
            {"pixelFormat", "RGBA8"},
            {"dimensionType", "Absolute"},
            {"width", 1920}
    };

    const validation_report report = validate_texture_format(texture_format, "TestTexture");

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Format of texture TestTexture: Missing field height");
}

/****************************************
 *      Sampler validation tests        *
 ****************************************/
 
/*!
 * \brief Tests that the sampler validator doesn't give any errors or warnings for well-formed sampler data
 */
TEST(sampler_validator, no_errors_or_warnings) {
    TEST_SETUP_LOGGER();
    
    nlohmann::json sampler = {
        { "name",     "TestSampler" },
        { "filter",   "Bilinear" },
        { "wrapMode", "Clamp" }
    };

    const validation_report report = validate_sampler(sampler);

    EXPECT_EQ(report.warnings.size(), 0);
    EXPECT_EQ(report.errors.size(), 0);
}

/*!
 * \brief Tests that the sampler validator correctly checks for a missing name
 */
TEST(sampler_validator, missing_name) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
            {"filter", "Bilinear"},
            {"wrapMode", "Clamp"}
    };

    const validation_report report = validate_sampler(sampler);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler <NAME_MISSING>: Missing field name");
}

/*!
 * \brief Tests that the sampler validator correctly checks for a missing filter
 */
TEST(sampler_validator, missing_filter) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
            {"name", "TestSampler"},
            {"wrapMode", "Clamp"}
    };

    const validation_report report = validate_sampler(sampler);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field filter");
}

/*!
 * \brief Tests that the sampler validator correctly checks for a missing wrapMode
 */
TEST(sampler_validator, missing_wrap_mode) {
    TEST_SETUP_LOGGER();

    nlohmann::json sampler = {
            {"name", "TestSampler"},
            {"filter", "Bilinear"}
    };

    const validation_report report = validate_sampler(sampler);

    EXPECT_EQ(report.warnings.size(), 0);

    ASSERT_EQ(report.errors.size(), 1);
    EXPECT_EQ(report.errors[0], "Sampler TestSampler: Missing field wrapMode");
}

/****************************************
 *      Material validation tests       *
 ****************************************/

/*!
 * \brief Happy path test. The material JSON is well-formed and filled with the most wonderful data
 */
TEST(material_validator, no_errors_or_warnings) {
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
TEST(material_validator, bindings_missing) {
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
TEST(material_validator, filter_missing) {
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

/*!
 * \brief Tests that the material validator correctly checks for a missing name
 */
TEST(material_validator, name_missing) {
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

/*!
 * \brief Tests that the material validator correctly checks for missing passes
 */
TEST(material_validator, passes_missing) {
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

/*!
 * \brief Tests that the material validator correctly checks for the passes field being the wrong type
 */
TEST(material_validator, passes_wrong_type) {
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

/*!
 * \brief Tests that the material validator correctly checks for the passes field being an empty array
 */
TEST(material_validator, passes_empty_array) {
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

/*!
 * \brief Tests that the material validator correctly checks if a material pass doesn't have a pipeline
 */
TEST(material_validator, pass_no_pipeline) {
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

/*!
 * \brief Tests that the material validator correctly checks is a material pass doesn't have a name
 */
TEST(material_validator, pass_no_name) {
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
