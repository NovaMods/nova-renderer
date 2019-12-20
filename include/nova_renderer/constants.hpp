//! \brief A collection of constants for Nova to use

#pragma once

#include <string>

namespace nova::renderer {
    const std::string MODEL_MATRIX_BUFFER_NAME = "NovaModelMatrixUBO";
    const std::string PER_FRAME_DATA_NAME = "NovaPerFrameUBO";

    constexpr uint32_t AMD_PCI_VENDOR_ID = 0x1022;
    constexpr uint32_t INTEL_PCI_VENDOR_ID = 8086;
    constexpr uint32_t NVIDIA_PCI_VENDOR_ID = 0x10DE;

    constexpr uint8_t NUM_IN_FLIGHT_FRAMES = 3;

    /*!
     * \brief Name of the builtin pass Nova uses to render UI
     *
     * This pass reads from the writes to the backbuffer. UI renderpasses are expected to use something like blending or the stencil butter
     * to layer the UI on top of the 3D scene.
     */
    const std::string UI_RENDER_PASS_NAME = "NovaUI";

    const std::string SCENE_OUTPUT_RENDER_TARGET_NAME = "NovaFinal";

    /*!
     * \brief Name of the backbuffer
     *
     * Nova presents the backbuffer to the screen every frame. The builtin UI render pass adds the UI to the backbuffer after the rest of
     * the rendergraph has finished
     */
    const std::string BACKBUFFER_NAME = "NovaBackbuffer";
} // namespace nova::renderer
