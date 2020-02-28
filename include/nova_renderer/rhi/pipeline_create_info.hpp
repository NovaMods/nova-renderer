#pragma once

#include <rx/core/string.h>

#include "nova_renderer/rhi/rhi_types.hpp"

/*!
 * \file pipeline_create_info.hpp
 *
 * \brief All the data needed for the RHI to create a pipeline state object
 */

namespace nova::renderer {
    /*!
     * \brief SPIR-V shader source
     */
    struct ShaderSource {
        /*!
         * \brief Filename of the file that this shader source originated from
         */
        rx::string filename{};

        /*!
         * \brief SPIR-V shader code
         */
        rx::vector<uint32_t> source{};
    };

    enum class PrimitiveTopology {
        PointList,
        LineList,
        TriangleList,
    };

    /*!
     * \brief How to fill in the rasterized primitives
     */
    enum class FillMode {
        /*!
         * \brief Completely fill the primitive
         */
        Solid,

        /*!
         * \brief Only draw lines between the vertices of each primitive
         */
        Wireframe,

        /*!
         * \brief Only draw points at the vertices of each primitive
         */
        Points
    };

    /*!
     * \brief Which kind of primitives to cull
     */
    enum class PrimitiveCullingMode {
        /*!
         * \brief Cull back faces - faces with a clockwise winding order
         */
        BackFace,

        /*!
         * \brief Cull front faces - faces with a counterclockwise winding order
         */
        FrontFace,

        /*!
         * \brief Don't cull any faces
         */
        None,
    };

    /*!
     * \brief State of the pixel rasterizer
     */
    struct RasterizerState {
        /*!
         * \brief Enables or disabled depth clamping
         *
         * If depth clamping is enabled, then pixels with a depth value outside of the active viewport will have their depth value clamped
         * to the viewport. This can help prevent holes in primitives that are particularly close to or far from the active view
         */
        bool enable_depth_clamping = false;

        /*!
         * \brief How to fill in rasterized primitives
         */
        FillMode fill_mode{};

        /*!
         * \brief Which faces, if any, should be culled based on their winding order
         */
        PrimitiveCullingMode cull_mode{};

        /*!
         * \brief Constant amount to bias the output depth
         */
        float depth_bias = 0;

        /*!
         * \brief Slope scaled amount to bias the output depth
         *
         * This value is multiplied by the output pixel's depth slope before being added to the output pixel's output depth
         */
        float slope_scaled_depth_bias = 0;

        /*!
         * \brief Maximum amount of allowed depth bias
         */
        float maximum_depth_bias = 0;
    };

    struct MultisamplingState {
        // TODO: Figure out multisampling. The Vulkan book's section is sadly incomplete
    };

    enum class StencilOp { Keep, Zero, Replace, Increment, IncrementAndWrap, Decrement, DecrementAndWrap, Invert };

    enum class CompareOp { Never, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual, Always };

    /*!
     * \brief Describes the stencil buffer operations to perform
     *
     * I don't like this name but idk a better one
     */
    struct StencilOpState {
        /*!
         * \brief What to do when the stencil test fails
         */
        StencilOp fail_op{};

        /*!
         * \brief What to do when the stencil op passes
         */
        StencilOp pass_op{};

        /*!
         * \brief What to do when the depth test fails
         */
        StencilOp depth_fail_op{};

        /*!
         * \brief How to compare a stencil value to the value in the stencil buffer
         */
        CompareOp compare_op{};

        /*!
         * \brief A bitmask of which stencil bits to compare
         */
        uint32_t compare_mask{};

        /*!
         * \brief A bitmask of what stencil bits to write
         */
        uint32_t write_mask{};

        /*!
         * \brief Reference value to use in the stencil test
         */
        uint32_t reference_value{};
    };

    /*!
     * \brief What stencil buffer operations the pipeline state should use
     */
    struct StencilState {
        /*!
         * \brief The stencil operations to perform for front faces
         */
        StencilOpState front_face_op{};

        /*!
         * \brief The stencil operations to perform for back faces
         */
        StencilOpState back_face_op{};
    };

    /*!
     * \brief Depth bounds test mode
     */
    enum class DepthBoundsTestMode {
        /*!
         * \brief Test against a static depth bounds, as specified in the depth bounds test info
         */
        Static,

        /*!
         * \brief Test against a dynamic depth bounds, as specified when recording a command list
         */
        Dynamic,
    };

    /*!
     * \brief State to use for a static depth bounds test
     */
    struct StaticDepthBoundsTestState {
        /*!
         * \brief Minimum depth bound value
         */
        float min_bound = 0;

        /*!
         * \brief Maximum depth bound value
         */
        float max_bound = 0;
    };

    /*!
     * \brief State to use for a dynamic depth bounds test
     */
    struct DynamicDepthBoundsTestState {
        // Intentionally empty so I can use a union
    };

    /*!
     * \brief The state to use for the depth bounds test
     */
    struct DepthBoundsTestState {
        /*!
         * \brief Depth bounds test mode
         */
        DepthBoundsTestMode mode{};

        /*!
         * \brief State for the depth bounds test
         */
        union {
            StaticDepthBoundsTestState static_state{};
            DynamicDepthBoundsTestState dynamic_state;
        };
    };

    struct DepthState {
        /*!
         * \brief Whether to write the pixel shader's output depth value to the depth buffer
         */
        bool enable_depth_write = true;

        /*!
         * \brief Which operation to use for depth comparisons
         */
        CompareOp compare_op = CompareOp::Greater;

        /*!
         * \brief State to use for the depth bounds test
         *
         * If this optional is empty, the depth bounds test is disabled. Otherwise, the depth bounds test is enabled
         */
        rx::optional<DepthBoundsTestState> bounds_test_state{};
    };

    /*!
     * \brief A factor to use in blending
     */
    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        ClampedSrcAlpha,
    };

    /*!
     * \brief Operation to use to combine the blend factors
     */
    enum class BlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    /*!
     * \brief Blending state for a single attachment
     */
    struct RenderTargetBlendState {
        /*!
         * \brief Whether to enable blending for this attachment
         *
         * If false, all the other values in this struct are ignored
         */
        bool enable = false;

        /*!
         * \brief Blending factor for the pixel shader's output color
         */
        BlendFactor src_color_factor = BlendFactor::SrcAlpha;

        /*!
         * \brier Blending factor for the color in the output render target
         */
        BlendFactor dst_color_factor = BlendFactor::OneMinusSrcAlpha;

        /*!
         * \brief The operation to use when blending color
         */
        BlendOp color_op = BlendOp::Add;

        /*!
         * \brief Blending factor for the pixel shader's output alpha
         */
        BlendFactor src_alpha_factor = BlendFactor::SrcAlpha;

        /*!
         * \brief Blending factor for the alpha in the output render target
         */
        BlendFactor dst_alpha_factor = BlendFactor::OneMinusSrcAlpha;

        /*!
         * \brief The operation to use when blending alpha
         */
        BlendOp alpha_op = BlendOp::Add;
    };

    /*!
     * \brief How to blend colors
     */
    struct BlendState {
        /*!
         * \brief How to blend each render target that this pipeline state renders to
         */
        rx::vector<RenderTargetBlendState> render_target_states{};

        /*!
         * \brief Constant values to use for any render targets where one of the blend factors involves either a constant color or constant
         * alpha
         */
        glm::vec4 blend_constants{};
    };

    /*!
     * \brief All the information needed to create a pipeline state
     */
    struct PipelineStateCreateInfo {
        /*!
         * \brief Name of this pipeline state
         */
        rx::string name{};

        /*!
         * \brief Vertex shader to use
         */
        ShaderSource vertex_shader{};

        /*!
         * \brief Geometry shader to use
         */
        rx::optional<ShaderSource> geometry_shader{};

        /*!
         * \brier Pixel shader to use
         */
        rx::optional<ShaderSource> pixel_shader{};

        /*!
         * \brief Description of the fields in the vertex data
         */
        rx::vector<rhi::RhiVertexField> vertex_fields{};

        /*!
         * \brief Size of the viewport that this pipeline state renders to, measured in pixels
         */
        glm::vec2 viewport_size{};

        /*!
         * \brief Enables the scissor test, allowing e.g. UI elements to only render to a specific portion of the screen
         */
        bool enable_scissor_test = false;

        /*!
         * \brief Topology of the vertex data
         */
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        /*!
         * \brief The state of the rasterizer when this pipeline state is active
         */
        RasterizerState rasterizer_state{};

        rx::optional<MultisamplingState> multisampling_state{};

        /*!
         * \brief What depth operations to perform
         *
         * If this optional has a value, the depth test is enabled. If false, the depth test is disabled
         */
        rx::optional<DepthState> depth_state = DepthState{};

        /*!
         * \brief What stencil operations to perform
         *
         * If this optional has a value, the stencil test will be enabled. Otherwise, the stencil test will be disabled
         */
        rx::optional<StencilState> stencil_state{};

        /*!
         * \brief How to blend colors
         *
         * If this optional has a value, blending will be enabled. Otherwise, blending will be disabled
         */
        rx::optional<BlendState> blend_state{};

        bool enable_color_write = true;

        bool enable_alpha_write = true;

        /*!
         * \brief All the color attachments that this pipeline writes to
         */
        rx::vector<renderpack::TextureAttachmentInfo> color_attachments{};

        /*!
         * \brief The depth texture that this pipeline writes to, if it writes to a depth texture
         */
        rx::optional<renderpack::TextureAttachmentInfo> depth_texture{};
    };
} // namespace nova::renderer
