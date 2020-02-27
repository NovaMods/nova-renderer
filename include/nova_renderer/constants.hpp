//! \brief A collection of constants for Nova to use

#pragma once

#include "nova_renderer/util/bytes.hpp"

using namespace nova::mem::operators;

namespace nova::renderer {
    constexpr const char* MODEL_MATRIX_BUFFER_NAME = "NovaModelMatrixUBO";
    constexpr const char* PER_FRAME_DATA_NAME = "NovaPerFrameUBO";

    constexpr uint32_t AMD_PCI_VENDOR_ID = 0x1022;
    constexpr uint32_t INTEL_PCI_VENDOR_ID = 8086;
    constexpr uint32_t NVIDIA_PCI_VENDOR_ID = 0x10DE;

    constexpr uint32_t NUM_IN_FLIGHT_FRAMES = 3;

    constexpr uint32_t MAX_NUM_CAMERAS = 65536;

    constexpr mem::Bytes PER_FRAME_MEMORY_SIZE = 2_mb;

    constexpr const char* RENDERPACK_DIRECTORY = "renderpacks";
    constexpr const char* MATERIALS_DIRECTORY = "materials";
    constexpr const char* SHADERS_DIRECTORY = "shaders";
    constexpr const char* RENDERPACK_DESCRIPTOR_FILE = "renderpack.json";
    constexpr const char* RESOURCES_FILE = "resources.json";
    constexpr const char* MATERIAL_FILE_EXTENSION = ".mat";

    /*!
     * \brief Name of Nova's white texture
     *
     * The white texture is a 4x4 texture where each texel has the RGBA value of (1, 1, 1, 1)
     */
    constexpr const char* WHITE_TEXTURE_NAME = "NovaWhiteTexture";

    /*!
     * \brief Name of Nova's gray texture
     *
     * The gray texture is a 4x4 texture where each texel has the RGBA value of (0.5, 0.5, 0.5, 0.5)
     */
    constexpr const char* GRAY_TEXTURE_NAME = "NovaGrayTexture";

    /*!
     * \brief Name of Nova's black texture
     *
     * The black texture is a 4x4 texture where each texel has the RGBA value of (0, 0, 0, 0)
     */
    constexpr const char* BLACK_TEXTURE_NAME = "NovaBlackTexture";

    /*!
     * \brief Name of the builtin pass Nova uses to render UI
     *
     * This pass reads from the writes to the backbuffer. UI renderpasses are expected to use something like blending or the stencil butter
     * to layer the UI on top of the 3D scene.
     */
    constexpr const char* UI_RENDER_PASS_NAME = "NovaUI";

    /*!
     * \brief Name of the renderpass that outputs to the backbuffer
     */
    constexpr const char* BACKBUFFER_OUTPUT_RENDER_PASS_NAME = "BackbufferOutput";

    constexpr const char* BACKBUFFER_OUTPUT_PIPELINE_NAME = "BackbufferOutput";

    constexpr const char* BACKBUFFER_OUTPUT_MATERIAL_NAME = "BackbufferOutput";

    /*!
     * \brief Name of the render target that renderpacks must render to
     */
    constexpr const char* SCENE_OUTPUT_RT_NAME = "NovaSceneOutput";

    /*!
     * \brief Name of the UI render target
     *
     * All UI renderpasses MUST will render to this render target
     */
    constexpr const char* UI_OUTPUT_RT_NAME = "NovaUiOutput";

    /*!
     * \brief Name of the backbuffer
     *
     * Nova presents the backbuffer to the screen every frame. The builtin UI render pass adds the UI to the backbuffer after the rest of
     * the rendergraph has finished
     */
    constexpr const char* BACKBUFFER_NAME = "NovaBackbuffer";

    constexpr const char* POINT_SAMPLER_NAME = "NovaPointSampler";
} // namespace nova::renderer
