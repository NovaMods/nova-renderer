#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include  <optional>
#include <string>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "rhi/rhi_enums.hpp"
#include "rx/core/json.h"

namespace nova::renderer {
    class Renderpass;
} // namespace nova::renderer

namespace nova::renderer::renderpack {
    /*!
     * \brief Controls the rasterizer's state
     */
    enum class RasterizerState {
        /*!
         * \brief Enable blending for this material state
         */
        Blending,

        /*!
         * \brief Render backfaces and cull frontfaces
         */
        InvertCulling,

        /*!
         * \brief Don't cull backfaces or frontfaces
         */
        DisableCulling,

        /*!
         * \brief Don't write to the depth buffer
         */
        DisableDepthWrite,

        /*!
         * \brief Don't perform a depth test
         */
        DisableDepthTest,

        /*!
         * \brief Perform the stencil test
         */
        EnableStencilTest,

        /*!
         * \brief Write to the stencil buffer
         */
        StencilWrite,

        /*!
         * \brief Don't write to the color buffer
         */
        DisableColorWrite,

        /*!
         * \brief Enable alpha to coverage
         */
        EnableAlphaToCoverage,

        /*!
         * \brief Don't write alpha
         */
        DisableAlphaWrite,
    };

    enum class TextureFilter { TexelAA, Bilinear, Point };

    enum class WrapMode { Repeat, Clamp };

    /*!
     * \brief Where the texture comes from
     */
    enum class TextureLocation {
        /*!
         * \brief The texture is written to by a shader
         */
        Dynamic,

        /*!
         * \brief The texture is loaded from the textures/ folder in the current renderpack
         */
        InUserPackage,

        /*!
         * \brief The texture is provided by Nova or by Minecraft
         */
        InAppPackage
    };

    enum class MsaaSupport { MSAA, Both, None };

    enum class RPStencilOp { Keep, Zero, Replace, Increment, IncrementAndWrap, Decrement, DecrementAndWrap, Invert };

    enum class RPCompareOp { Never, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual, Always };

    enum class RPPrimitiveTopology { Triangles, Lines };

    enum class RPBlendFactor {
        One,
        Zero,
        SrcColor,
        DstColor,
        OneMinusSrcColor,
        OneMinusDstColor,
        SrcAlpha,
        DstAlpha,
        OneMinusSrcAlpha,
        OneMinusDstAlpha
    };

    enum class RenderQueue { Transparent, Opaque, Cutout };

    enum class ScissorTestMode {
        Off,
        StaticScissorRect,
        DynamicScissorRect,
    };

    enum class TextureDimensionType { ScreenRelative, Absolute };

    enum class ImageUsage {
        RenderTarget,
        SampledImage,
    };

    /*!
     * \brief Defines a sampler to use for a texture
     *
     * At the time of writing I'm not sure how this is correlated with a texture, but all well
     */
    struct SamplerCreateInfo {
        std::string name;

        /*!
         * \brief What kind of texture filter to use
         *
         * texel_aa does something that I don't want to figure out right now. Bilinear is your regular bilinear filter,
         * and point is the point filter. Aniso isn't an option and I kinda hope it stays that way
         */
        TextureFilter filter{};

        /*!
         * \brief How the texture should wrap at the edges
         */
        WrapMode wrap_mode{};

        static SamplerCreateInfo from_json(const nlohmann::json& json);
    };

    struct StencilOpState {
        RPStencilOp fail_op;
        RPStencilOp pass_op;
        RPStencilOp depth_fail_op;
        RPCompareOp compare_op;
        uint32_t compare_mask;
        uint32_t write_mask;

        static StencilOpState from_json(const nlohmann::json& json);
    };

    struct RenderpackShaderSource {
        std::string filename;
        std::vector<uint32_t> source;
    };

    /*!
     * \brief All the data that Nova uses to build a pipeline
     */
    struct PipelineData {
        /*!
         * \brief The name of this pipeline
         */
        std::string name;

        /*!
         * \brief The pipeline that this pipeline inherits from
         */
        std::optional<std::string> parent_name;

        /*!
         * \brief The name of the pass that this pipeline belongs to
         */
        std::string pass;

        /*!
         * \brief All of the symbols in the shader that are defined by this state
         */
        std::vector<std::string> defines{};

        /*!
         * \brief Defines the rasterizer state that's active for this pipeline
         */
        std::vector<RasterizerState> states{};

        /*!
         * \brief The stencil buffer operations to perform on the front faces
         */
        std::optional<StencilOpState> front_face;

        /*!
         * \brief The stencil buffer operations to perform on the back faces
         */
        std::optional<StencilOpState> back_face;

        /*!
         * \brief The material to use if this one's shaders can't be found
         */
        std::optional<std::string> fallback;

        /*!
         * \brief A bias to apply to the depth
         */
        float depth_bias{};

        /*!
         * \brief The depth bias, scaled by slope I guess?
         */
        float slope_scaled_depth_bias{};

        /*!
         * \brief The reference value to use for the stencil test
         */
        uint32_t stencil_ref{};

        /*!
         * \brief The mask to use when reading from the stencil buffer
         */
        uint32_t stencil_read_mask{};

        /*!
         * \brief The mask to use when writing to the stencil buffer
         */
        uint32_t stencil_write_mask{};

        /*!
         * \brief How to handle MSAA for this state
         */
        MsaaSupport msaa_support{};

        /*!
         * \brief
         */
        RPPrimitiveTopology primitive_mode{};

        /*!
         * \brief Where to get the blending factor for the source
         */
        RPBlendFactor source_color_blend_factor{};

        /*!
         * \brief Where to get the blending factor for the destination
         */
        RPBlendFactor destination_color_blend_factor{};

        /*!
         * \brief How to get the source alpha in a blend
         */
        RPBlendFactor source_alpha_blend_factor{};

        /*!
         * \brief How to get the destination alpha in a blend
         */
        RPBlendFactor destination_alpha_blend_factor{};

        /*!
         * \brief The function to use for the depth test
         */
        RPCompareOp depth_func{};

        /*!
         * \brief The render queue that this pass belongs to
         *
         * This may or may not be removed depending on what is actually needed by Nova
         */
        RenderQueue render_queue{};

        ScissorTestMode scissor_mode = ScissorTestMode::Off;

        RenderpackShaderSource vertex_shader{};

        std::optional<RenderpackShaderSource> geometry_shader;
        std::optional<RenderpackShaderSource> tessellation_control_shader;
        std::optional<RenderpackShaderSource> tessellation_evaluation_shader;
        std::optional<RenderpackShaderSource> fragment_shader;

        static PipelineData from_json(const nlohmann::json& json);
    };

    struct TextureFormat {
        /*!
         * \brief The format of the texture
         */
        rhi::PixelFormat pixel_format{};

        /*!
         * \brief How to interpret the dimensions of this texture
         */
        TextureDimensionType dimension_type{};

        /*!
         * \brief The width, in pixels, of the texture
         */
        float width = 0;

        /*!
         * \brief The height, in pixels, of the texture
         */
        float height = 0;

        constexpr TextureFormat() = default;

        [[nodiscard]] glm::uvec2 get_size_in_pixels(const glm::uvec2& screen_size) const;

        bool operator==(const TextureFormat& other) const;
        bool operator!=(const TextureFormat& other) const;

        static TextureFormat from_json(const nlohmann::json& json);
    };

    /*!
     * \brief A texture that a pass can use
     */
    struct TextureCreateInfo {
        /*!
         * \brief The name of the texture
         *
         * Nova implicitly defines a few textures for you to use:
         * - NovaColorVirtualTexture
         *      - Virtual texture atlas that holds color textures
         *      - Textures which have the exact name as requested by Minecraft are in this atlas
         *      - Things without a color texture get a pure white texture
         *      - Always has a format of R8G8B8A8
         *      - Can only be used as a pass's input
         * - NovaNormalVirtualTexture
         *      - Virtual texture atlas that holds normal textures
         *      - Textures which have `_n` after the name requested by Minecraft are in this atlas
         *      - If no normal texture exists for a given object, a texture with RGBA of (0, 0, 1, 1) is used
         *      - Always has a format of R8G8B8A8
         *      - Can only be used as a pass's input
         * - NovaDataVirtualTexture
         *      - Virtual texture atlas that holds data textures
         *      - Textures which have a `_s` after the name requested by Minecraft are in this atlas
         *      - If no data texture exists for a given object, a texture with an RGBA of (0, 0, 0, 0) is used
         *      - Always has a format of R8G8B8A8
         *      - Can only be used as a pass's input
         * - NovaLightmap
         *      - Minecraft lightmap, loaded from the current resourcepack
         *      - Format of RGBA8
         *      - Can only be used as an input
         * - NovaFinal
         *      - The texture that gets presented to the screen
         *      - Always has a format of RGB8
         *      - Can only be used as a pass's output
         *
         * If you use one of the virtual textures, then all fields except the binding are ignored
         * If you use `NovaFinal`, then all fields are ignored since the backbuffer is always bound to output location 0
         */
        std::string name;

        // TODO: Renderpack developers shouldn't have to worry about this
        ImageUsage usage;

        TextureFormat format{};

        static TextureCreateInfo from_json(const nlohmann::json& json);
    };

    struct RenderpackResourcesData {
        std::vector<TextureCreateInfo> render_targets;
        std::vector<SamplerCreateInfo> samplers;

        static RenderpackResourcesData from_json(const nlohmann::json& json);
    };

    /*!
     * \brief A description of a texture that a render pass outputs to
     */
    struct TextureAttachmentInfo {
        /*!
         * \brief The name of the texture
         */
        std::string name{};

        rhi::PixelFormat pixel_format;

        /*!
         * \brief Whether to clear it
         *
         * If the texture is a depth buffer, it gets cleared to 1
         * If the texture is a stencil buffer, it gets cleared to 0xFFFFFFFF
         * If the texture is a color buffer, it gets cleared to (0, 0, 0, 0)
         */
        bool clear = false;

        bool operator==(const TextureAttachmentInfo& other) const;

        static TextureAttachmentInfo from_json(const nlohmann::json& json);
    };

    /*!
     * \brief A pass over the scene
     *
     * A pass has a few things:
     * - What passes MUST be executed before this one
     * - What inputs this pass's shaders have
     *      - What uniform buffers to use
     *      - What vertex data to use
     *      - Any textures that are needed
     * - What outputs this pass has
     *      - Framebuffer attachments
     *      - Write buffers
     *
     * The inputs and outputs of a pass must be resources declared in the renderpack's `resources.json` file (or the
     * default resources.json), or a resource that's internal to Nova. For example, Nova provides a UBO of uniforms that
     * change per frame, a UBO for per-model data like the model matrix, and the virtual texture atlases. The default
     * resources.json file sets up sixteen framebuffer color attachments for ping-pong buffers, a depth attachment,
     * some shadow maps, etc
     */
    struct RenderPassCreateInfo {
        /*!
         * \brief The name of this render pass
         */
        std::string name;

        /*!
         * \brief The textures that this pass will read from
         */
        std::vector<std::string> texture_inputs{};

        /*!
         * \brief The textures that this pass will write to
         */
        std::vector<TextureAttachmentInfo> texture_outputs{};

        /*!
         * \brief The depth texture this pass will write to
         */
        std::optional<TextureAttachmentInfo> depth_texture;

        /*!
         * \brief All the buffers that this renderpass reads from
         */
        std::vector<std::string> input_buffers{};

        /*!
         * \brief All the buffers that this renderpass writes to
         */
        std::vector<std::string> output_buffers{};

        /*!
         * \brief Names of all the pipelines that use this renderpass
         */
        std::vector<std::string> pipeline_names;

        RenderPassCreateInfo() = default;

        static RenderPassCreateInfo from_json(const nlohmann::json& json);
    };

    /*!
     * \brief All the data to create one rendergraph, including which builtin passes the renderpack wants to use in its rendergraph
     */
    struct RendergraphData {
        /*!
         * \brief The renderpack-supplied passes
         */
        std::vector<RenderPassCreateInfo> passes;

        /*!
         * \brief Names of all the builtin renderpasses that the renderpack wants to use
         */
        std::vector<std::string> builtin_passes;

        static RendergraphData from_json(const nlohmann::json& json);
    };

    struct MaterialPass {
        std::string name;
        std::string material_name;
        std::string pipeline;

        std::unordered_map<std::string, std::string> bindings;

        /*!
         * \brief All the descriptor sets needed to bind everything used by this material to its pipeline
         *
         * All the material's resources get bound to its descriptor sets when the material is created. Updating
         * descriptor sets is allowed, although the result won't show up on screen for a couple frames because Nova
         * (will) copies its descriptor sets to each in-flight frame
         */
        std::vector<VkDescriptorSet> descriptor_sets;

        static MaterialPass from_json(const nlohmann::json& json);
    };

    struct MaterialData {
        std::string name;
        std::vector<MaterialPass> passes;
        std::string geometry_filter;

        static MaterialData from_json(const nlohmann::json& json);
    };

    /*!
     * \brief All the data that can be in a renderpack
     */
    struct RenderpackData {
        std::vector<PipelineData> pipelines;

        /*!
         * \brief All the renderpasses that this renderpack needs, in submission order
         */
        RendergraphData graph_data;

        std::vector<MaterialData> materials;

        RenderpackResourcesData resources;

        std::string name;
    };

    [[nodiscard]] rhi::PixelFormat pixel_format_enum_from_string(const std::string& str);
    [[nodiscard]] TextureDimensionType texture_dimension_type_enum_from_string(const std::string& str);
    [[nodiscard]] TextureFilter texture_filter_enum_from_string(const std::string& str);
    [[nodiscard]] WrapMode wrap_mode_enum_from_string(const std::string& str);
    [[nodiscard]] RPStencilOp stencil_op_enum_from_string(const std::string& str);
    [[nodiscard]] RPCompareOp compare_op_enum_from_string(const std::string& str);
    [[nodiscard]] MsaaSupport msaa_support_enum_from_string(const std::string& str);
    [[nodiscard]] RPPrimitiveTopology primitive_topology_enum_from_string(const std::string& str);
    [[nodiscard]] RPBlendFactor blend_factor_enum_from_string(const std::string& str);
    [[nodiscard]] RenderQueue render_queue_enum_from_string(const std::string& str);
    [[nodiscard]] ScissorTestMode scissor_test_mode_from_string(const std::string& str);
    [[nodiscard]] RasterizerState state_enum_from_string(const std::string& str);

    [[nodiscard]] rhi::PixelFormat pixel_format_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] TextureDimensionType texture_dimension_type_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] TextureFilter texture_filter_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] WrapMode wrap_mode_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] RPStencilOp stencil_op_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] RPCompareOp compare_op_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] MsaaSupport msaa_support_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] RPPrimitiveTopology primitive_topology_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] RPBlendFactor blend_factor_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] RenderQueue render_queue_enum_from_json(const nlohmann::json& j);
    [[nodiscard]] ScissorTestMode scissor_test_mode_from_json(const nlohmann::json& j);
    [[nodiscard]] RasterizerState state_enum_from_json(const nlohmann::json& j);

    [[nodiscard]] std::string to_string(rhi::PixelFormat val);
    [[nodiscard]] std::string to_string(TextureDimensionType val);
    [[nodiscard]] std::string to_string(TextureFilter val);
    [[nodiscard]] std::string to_string(WrapMode val);
    [[nodiscard]] std::string to_string(RPStencilOp val);
    [[nodiscard]] std::string to_string(RPCompareOp val);
    [[nodiscard]] std::string to_string(MsaaSupport val);
    [[nodiscard]] std::string to_string(RPPrimitiveTopology val);
    [[nodiscard]] std::string to_string(RPBlendFactor val);
    [[nodiscard]] std::string to_string(RenderQueue val);
    [[nodiscard]] std::string to_string(RasterizerState val);

    [[nodiscard]] uint32_t pixel_format_to_pixel_width(rhi::PixelFormat format);
} // namespace nova::renderer::renderpack
