//! \brief A collection of constants for Nova to use

#pragma once

#include <string>

#include "nova_renderer/memory/bytes.hpp"

using namespace nova::mem::operators;

namespace nova::renderer {
    const std::string MODEL_MATRIX_BUFFER_NAME = "NovaModelMatrixUBO";
    const std::string PER_FRAME_DATA_NAME = "NovaPerFrameUBO";

    constexpr uint32_t AMD_PCI_VENDOR_ID = 0x1022;
    constexpr uint32_t INTEL_PCI_VENDOR_ID = 8086;
    constexpr uint32_t NVIDIA_PCI_VENDOR_ID = 0x10DE;

    constexpr uint8_t NUM_IN_FLIGHT_FRAMES = 3;

    constexpr mem::Bytes PER_FRAME_MEMORY_SIZE = 2_mb;

    const fs::path RENDERPACK_DIRECTORY = "shaderpacks";

    /*!
     * \brief Name of Nova's white texture
     *
     * The white texture is a 4x4 texture where each texel has the RGBA value of (1, 1, 1, 1)
     */
    const std::string WHITE_TEXTURE_NAME = "NovaWhiteTexture";

    /*!
     * \brief Name of Nova's gray texture
     *
     * The gray texture is a 4x4 texture where each texel has the RGBA value of (0.5, 0.5, 0.5, 0.5)
     */
    const std::string GRAY_TEXTURE_NAME = "NovaGrayTexture";

    /*!
     * \brief Name of Nova's black texture
     *
     * The black texture is a 4x4 texture where each texel has the RGBA value of (0, 0, 0, 0)
     */
    const std::string BLACK_TEXTURE_NAME = "NovaBlackTexture";

    /*!
     * \brief Name of the builtin pass Nova uses to render UI
     *
     * This pass reads from the writes to the backbuffer. UI renderpasses are expected to use something like blending or the stencil butter
     * to layer the UI on top of the 3D scene.
     */
    const std::string UI_RENDER_PASS_NAME = "NovaUI";

    /*!
     * \brief Name of the render target that renderpacks must render to
     */
    const std::string SCENE_OUTPUT_RT_NAME = "NovaSceneOutput";

    /*!
     * \brief Name of the backbuffer
     *
     * Nova presents the backbuffer to the screen every frame. The builtin UI render pass adds the UI to the backbuffer after the rest of
     * the rendergraph has finished
     */
    const std::string BACKBUFFER_NAME = "NovaBackbuffer";
} // namespace nova::renderer
