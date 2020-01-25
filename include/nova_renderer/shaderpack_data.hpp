#pragma once

#include <glm/glm.hpp>
#include <rx/core/map.h>
#include <rx/core/optional.h>
#include <rx/core/string.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

namespace nova::renderer {
    namespace rhi {
        enum class PixelFormat;
    }

    class Renderpass;
} // namespace nova::renderer

namespace nova::renderer::shaderpack {
    /*!
     * \brief Controls the rasterizer's state
     */
    enum class StateEnum {
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

    enum class TextureFilterEnum { TexelAA, Bilinear, Point };

    enum class WrapModeEnum { Repeat, Clamp };

    /*!
     * \brief Where the texture comes from
     */
    enum class TextureLocationEnum {
        /*!
         * \brief The texture is written to by a shader
         */
        Dynamic,

        /*!
         * \brief The texture is loaded from the textures/ folder in the current shaderpack
         */
        InUserPackage,

        /*!
         * \brief The texture is provided by Nova or by Minecraft
         */
        InAppPackage
    };

    enum class MsaaSupportEnum { MSAA, Both, None };

    enum class StencilOpEnum { Keep, Zero, Replace, Increment, IncrementAndWrap, Decrement, DecrementAndWrap, Invert };

    enum class CompareOpEnum { Never, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual, Always };

    enum class PrimitiveTopologyEnum { Triangles, Lines };

    enum class BlendFactorEnum {
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

    enum class RenderQueueEnum { Transparent, Opaque, Cutout };

    enum class ScissorTestMode {
        Off,
        StaticScissorRect,
        DynamicScissorRect,
    };

    enum class PixelFormatEnum {
        RGBA8,
        RGBA16F,
        RGBA32F,
        Depth,
        DepthStencil,
    };

    enum class TextureDimensionTypeEnum { ScreenRelative, Absolute };

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
        rx::string name;

        /*!
         * \brief What kind of texture filter to use
         *
         * texel_aa does something that I don't want to figure out right now. Bilinear is your regular bilinear filter,
         * and point is the point filter. Aniso isn't an option and I kinda hope it stays that way
         */
        TextureFilterEnum filter{};

        /*!
         * \brief How the texture should wrap at the edges
         */
        WrapModeEnum wrap_mode{};
    };

    struct StencilOpState {
        StencilOpEnum fail_op;
        StencilOpEnum pass_op;
        StencilOpEnum depth_fail_op;
        CompareOpEnum compare_op;
        uint32_t compare_mask;
        uint32_t write_mask;
    };

    struct ShaderSource {
        rx::string filename;
        rx::vector<uint32_t> source;
    };

    /*!
     * \brief All the data that Nova uses to build a pipeline
     */
    struct PipelineCreateInfo {
        /*!
         * \brief The name of this pipeline
         */
        rx::string name;

        /*!
         * \brief The pipeline that this pipeline inherits from
         */
        rx::optional<rx::string> parent_name;

        /*!
         * \brief The name of the pass that this pipeline belongs to
         */
        rx::string pass;

        /*!
         * \brief All of the symbols in the shader that are defined by this state
         */
        rx::vector<rx::string> defines{};

        /*!
         * \brief Defines the rasterizer state that's active for this pipeline
         */
        rx::vector<StateEnum> states{};

        /*!
         * \brief The stencil buffer operations to perform on the front faces
         */
        rx::optional<StencilOpState> front_face;

        /*!
         * \brief The stencil buffer operations to perform on the back faces
         */
        rx::optional<StencilOpState> back_face;

        /*!
         * \brief The material to use if this one's shaders can't be found
         */
        rx::optional<rx::string> fallback;

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
        MsaaSupportEnum msaa_support{};

        /*!
         * \brief
         */
        PrimitiveTopologyEnum primitive_mode{};

        /*!
         * \brief Where to get the blending factor for the source
         */
        BlendFactorEnum source_color_blend_factor{};

        /*!
         * \brief Where to get the blending factor for the destination
         */
        BlendFactorEnum destination_color_blend_factor{};

        /*!
         * \brief How to get the source alpha in a blend
         */
        BlendFactorEnum source_alpha_blend_factor{};

        /*!
         * \brief How to get the destination alpha in a blend
         */
        BlendFactorEnum destination_alpha_blend_factor{};

        /*!
         * \brief The function to use for the depth test
         */
        CompareOpEnum depth_func{};

        /*!
         * \brief The render queue that this pass belongs to
         *
         * This may or may not be removed depending on what is actually needed by Nova
         */
        RenderQueueEnum render_queue{};

        ScissorTestMode scissor_mode = ScissorTestMode::Off;

        ShaderSource vertex_shader{};

        rx::optional<ShaderSource> geometry_shader;
        rx::optional<ShaderSource> tessellation_control_shader;
        rx::optional<ShaderSource> tessellation_evaluation_shader;
        rx::optional<ShaderSource> fragment_shader;
    };

    struct TextureFormat {
        /*!
         * \brief The format of the texture
         */
        PixelFormatEnum pixel_format;

        /*!
         * \brief How to interpret the dimensions of this texture
         */
        TextureDimensionTypeEnum dimension_type;

        /*!
         * \brief The width, in pixels, of the texture
         */
        float width;
        /*!
         * \brief The height, in pixels, of the texture
         */
        float height;

        [[nodiscard]] glm::uvec2 get_size_in_pixels(const glm::uvec2& screen_size) const;

        bool operator==(const TextureFormat& other) const;
        bool operator!=(const TextureFormat& other) const;
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
        rx::string name;

        // TODO: Renderpack developers shouldn't have to worry about this
        ImageUsage usage;

        TextureFormat format{};
    };

    struct ShaderpackResourcesData {
        rx::vector<TextureCreateInfo> render_targets;
        rx::vector<SamplerCreateInfo> samplers;

        // TODO: Figure out shader readable textures
    };

    /*!
     * \brief A description of a texture that a render pass outputs to
     */
    struct TextureAttachmentInfo {
        /*!
         * \brief The name of the texture
         */
        rx::string name{};

        PixelFormatEnum pixel_format;

        /*!
         * \brief Whether to clear it
         *
         * If the texture is a depth buffer, it gets cleared to 1
         * If the texture is a stencil buffer, it gets cleared to 0xFFFFFFFF
         * If the texture is a color buffer, it gets cleared to (0, 0, 0, 0)
         */
        bool clear = false;

        bool operator==(const TextureAttachmentInfo& other) const;
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
     * The inputs and outputs of a pass must be resources declared in the shaderpack's `resources.json` file (or the
     * default resources.json), or a resource that's internal to Nova. For example, Nova provides a UBO of uniforms that
     * change per frame, a UBO for per-model data like the model matrix, and the virtual texture atlases. The default
     * resources.json file sets up sixteen framebuffer color attachments for ping-pong buffers, a depth attachment,
     * some shadow maps, etc
     */
    struct RenderPassCreateInfo {
        /*!
         * \brief The name of this render pass
         */
        rx::string name;

        /*!
         * \brief The textures that this pass will read from
         */
        rx::vector<rx::string> texture_inputs{};

        /*!
         * \brief The textures that this pass will write to
         */
        rx::vector<TextureAttachmentInfo> texture_outputs{};

        /*!
         * \brief The depth texture this pass will write to
         */
        rx::optional<TextureAttachmentInfo> depth_texture;

        /*!
         * \brief All the buffers that this renderpass reads from
         */
        rx::vector<rx::string> input_buffers{};

        /*!
         * \brief All the buffers that this renderpass writes to
         */
        rx::vector<rx::string> output_buffers{};

        /*!
         * \brief Names of all the pipelines that use this renderpass
         */
        rx::vector<rx::string> pipeline_names;

        RenderPassCreateInfo() = default;
    };

    /*!
     * \brief All the data to create one rendergraph, including which builtin passes the renderpack wants to use in its rendergraph
     */
    struct RendergraphData {
        /*!
         * \brief The shaderpack-supplied passes
         */
        rx::vector<RenderPassCreateInfo> passes;

        /*!
         * \brief Names of all the builtin renderpasses that the renderpack wants to use
         */
        rx::vector<rx::string> builtin_passes;
    };

    struct MaterialPass {
        rx::string name;
        rx::string material_name;
        rx::string pipeline;

        rx::map<rx::string, rx::string> bindings;

        /*!
         * \brief All the descriptor sets needed to bind everything used by this material to its pipeline
         *
         * All the material's resources get bound to its descriptor sets when the material is created. Updating
         * descriptor sets is allowed, although the result won't show up on screen for a couple frames because Nova
         * (will) copies its descriptor sets to each in-flight frame
         */
        rx::vector<VkDescriptorSet> descriptor_sets;

        VkPipelineLayout layout = VK_NULL_HANDLE;
    };

    struct MaterialData {
        rx::string name;
        rx::vector<MaterialPass> passes;
        rx::string geometry_filter;
    };

    /*!
     * \brief All the data that can be in a shaderpack
     */
    struct RenderpackData {
        rx::vector<PipelineCreateInfo> pipelines;

        /*!
         * \brief All the renderpasses that this shaderpack needs, in submission order
         */
        RendergraphData graph_data;

        rx::vector<MaterialData> materials;

        ShaderpackResourcesData resources;

        rx::string name;
    };

    // TODO: Wrap these in to_json/from_json thingies

    [[nodiscard]] PixelFormatEnum pixel_format_enum_from_string(const rx::string& str);
    [[nodiscard]] TextureDimensionTypeEnum texture_dimension_type_enum_from_string(const rx::string& str);
    [[nodiscard]] TextureFilterEnum texture_filter_enum_from_string(const rx::string& str);
    [[nodiscard]] WrapModeEnum wrap_mode_enum_from_string(const rx::string& str);
    [[nodiscard]] StencilOpEnum stencil_op_enum_from_string(const rx::string& str);
    [[nodiscard]] CompareOpEnum compare_op_enum_from_string(const rx::string& str);
    [[nodiscard]] MsaaSupportEnum msaa_support_enum_from_string(const rx::string& str);
    [[nodiscard]] PrimitiveTopologyEnum primitive_topology_enum_from_string(const rx::string& str);
    [[nodiscard]] BlendFactorEnum blend_factor_enum_from_string(const rx::string& str);
    [[nodiscard]] RenderQueueEnum render_queue_enum_from_string(const rx::string& str);
    [[nodiscard]] StateEnum state_enum_from_string(const rx::string& str);

    [[nodiscard]] rx::string to_string(PixelFormatEnum val);
    [[nodiscard]] rx::string to_string(TextureDimensionTypeEnum val);
    [[nodiscard]] rx::string to_string(TextureFilterEnum val);
    [[nodiscard]] rx::string to_string(WrapModeEnum val);
    [[nodiscard]] rx::string to_string(StencilOpEnum val);
    [[nodiscard]] rx::string to_string(CompareOpEnum val);
    [[nodiscard]] rx::string to_string(MsaaSupportEnum val);
    [[nodiscard]] rx::string to_string(PrimitiveTopologyEnum val);
    [[nodiscard]] rx::string to_string(BlendFactorEnum val);
    [[nodiscard]] rx::string to_string(RenderQueueEnum val);
    [[nodiscard]] rx::string to_string(StateEnum val);

    [[nodiscard]] uint32_t pixel_format_to_pixel_width(PixelFormatEnum format);

    [[nodiscard]] PixelFormatEnum to_pixel_format_enum(rhi::PixelFormat format);

    [[nodiscard]] rhi::PixelFormat to_rhi_pixel_format(PixelFormatEnum format);
} // namespace nova::renderer::shaderpack
