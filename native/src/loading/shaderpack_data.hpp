/*!
 * \brief Holds all the structs that correspond to the data in a shaderpack
 *
 * \author ddubois 
 * \date 23-Aug-18.
 */

#ifndef NOVA_RENDERER_SHADERPACK_DATA_HPP
#define NOVA_RENDERER_SHADERPACK_DATA_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include <optional>

#include "../util/smart_enum.hpp"
#include <nlohmann/json.hpp>

namespace nova {
    /*!
     * \brief Controlls the rasterizer's state
     */
    SMART_ENUM(state_enum,
    /*!
     * \brief Enable blending for this material state
     */
            Blending,

    /*!
     * \brief Render backfaces and cull frontfaces
     */
            InvertCuling,

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
    )

    SMART_ENUM(texture_filter_enum,
            TexelAA,
            Bilinear,
            Point
    )

    SMART_ENUM(wrap_mode_enum,
            Repeat,
            Clamp
    )

    /*!
     * \brief Defines a sampler to use for a texture
     *
     * At the time of writing I'm not sure how this is corellated with a texture, but all well
     */
    struct sampler_state {
        /*!
         * \brief The index of the sampler. This might correspond directly with the texture but I hope not cause I don't
         * want to write a number of sampler blocks
         */
        std::optional<uint32_t> sampler_index;

        /*!
         * \brief What kind of texture filter to use
         *
         * texel_aa does something that I don't want to figure out right now. Bilinear is your regular bilinear filter,
         * and point is the point filter. Aniso isn't an option and I kinda hope it stays that way
         */
        std::optional<texture_filter_enum> filter;

        /*!
         * \brief How the texutre should wrap at the edges
         */
        std::optional<wrap_mode_enum> wrap_mode;
    };

    /*!
     * \brief The kind of data in a vertex attribute
     */
    SMART_ENUM(vertex_field_enum,
    /*!
     * \brief The vertex position
     *
     * 12 bytes
     */
            Position,

    /*!
     * \brief The vertex color
     *
     * 4 bytes
     */
            Color,

    /*!
     * \brief The UV coordinate of this object
     *
     * Except not really, because Nova's virtual textures means that the UVs for a block or entity or whatever
     * could change on the fly, so this is kinda more of a preprocessor define that replaces the UV with a lookup
     * in the UV table
     *
     * 8 bytes (might try 4)
     */
            UV0,

    /*!
     * \brief The UV coordinate in the lightmap texture
     *
     * This is a real UV and it doesn't change for no good reason
     *
     * 2 bytes
     */
            UV1,

    /*!
     * \brief Vertex normal
     *
     * 12 bytes
     */
            Normal,

    /*!
     * \brief Vertex tangents
     *
     * 12 bytes
     */
            Tangent,

    /*!
     * \brief The texture coordinate of the middle of the quad
     *
     * 8 bytes
     */
            MidTexCoord,

    /*!
     * \brief A uint32_t that's a unique identifier for the texture that this vertex uses
     *
     * This is generated at runtime by Nova, so it may change a lot depending on what resourcepacks are loaded and
     * if they use CTM or random detail textures or whatever
     *
     * 4 bytes
     */
            VirtualTextureId,

    /*!
     * \brief Some information about the current block/entity/whatever
     *
     * 12 bytes
     */
            McEntityId,

    /*!
     * \brief Useful if you want to skip a vertex attribute
     */
            Empty,
    )

    /*!
     * \brief Where the texture comes from
     */
    SMART_ENUM(texture_location_enum,
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
    )

    /*!
     * \brief A texture definition in a material file
     *
     * This class simple describes where to get the texture data from.
     */
    struct texture {
        /*!
         * \brief Where Nova should look for the texture at
         *
         * The texture location currently has two values: Dynamic and InUserPackage.
         *
         * A Dynamic texture is generated at runtime, often as part of the rendering pipeline. There are a number of
         * dynamic textures defined by Nova, which I don't feel like listing out here.
         *
         * A texture that's InUserPackage is not generated at runtime. Rather, it's supplied by the resourcepack. A
         * InUserPackage texture can have a name that's the path, relative to the resourcepack, of where to find the
         * texture, or it can refer to an atlas. Because of the way Nova handles megatextures there's actually always
         * three atlases: color, normals, and data. Something like `atlas.terrain` or `atlas.gui` refer to the color
         * atlas. Think of the texture name as more of a guideline then an actual rule
         */
        texture_location_enum texture_location;

        /*!
         * \brief The name of the texture
         *
         * If the texture name starts with `atlas` then the texture is one of the atlases. Nova sticks all the textures
         * it can into the virtual texture atlas, so it doesn't really care what atlas you request.
         */
        std::string texture_name;

        /*!
         * \brief If true, calculates mipmaps for this texture before the shaders is drawn
         */
        std::optional<bool> calculate_mipmaps;
    };

    SMART_ENUM(msaa_support_enum,
            MSAA,
            Both,
            None
    )

    SMART_ENUM(stencil_op_enum,
            Keep,
            Zero,
            Replace,
            Incr,
            IncrWrap,
            Decr,
            DecrWrap,
            Invert
    )

    SMART_ENUM(compare_op_enum,
            Never,
            Less,
            LessEqual,
            Greater,
            GreaterEqual,
            Equal,
            NotEqual,
            Always
    )

    struct stencil_op_state {
        std::optional<stencil_op_enum> fail_op;
        std::optional<stencil_op_enum> pass_op;
        std::optional<stencil_op_enum> depth_fail_op;
        std::optional<compare_op_enum> compare_op;
        std::optional<uint32_t> compare_mask;
        std::optional<uint32_t> write_mask;
    };

    SMART_ENUM(pass_enum,
            Shadow,
            Gbuffer,
            Transparent,
            DeferredLight,
            Fullscreen
    )

    SMART_ENUM(primitive_topology_enum,
            Triangles,
            Lines
    )

    SMART_ENUM(blend_factor_enum,
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
    )

    SMART_ENUM(render_queue_enum,
            Transparent,
            Opaque,
            Cutout
    )

    struct bound_resource {
        /*!
         * \brief The name of the resource
         */
        std::string name;

        /*!
         * \brief Where to bind the resource
         *
         * For input textures, this is the texture binding unit to use
         * For input buffers, the is the uniform location of the buffer
         *
         * For output textures, this is the framebuffer attachment slot to use
         */
        uint32_t binding;
    };

    /*!
     * \brief All the data that Nova uses to build a pipeline
     */
    struct pipeline_data {
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
        std::optional<std::string> pass;

        /*!
         * \brief All of the symbols in the shader that are defined by this state
         */
        std::optional<std::vector<std::string>> defines;

        /*!
         * \brief Defines the rasterizer state that's active for this pipeline
         */
        std::optional<std::vector<state_enum>> states;

        /*!
         * \brief Sets up the vertex fields that Nova will bind to this pipeline
         *
         * The index in the array is the attribute index that the vertex field is bound to
         */
        std::optional<std::vector<vertex_field_enum>> vertex_fields;

        /*!
         * \brief The stencil buffer operations to perform on the front faces
         */
        std::optional<stencil_op_state> front_face;

        /*!
         * \brief The stencil buffer operations to perform on the back faces
         */
        std::optional<stencil_op_state> back_face;

        /*!
         * \brief All the textures that this material reads from
         */
        std::optional<std::vector<bound_resource>> input_textures;

        /*!
         * \brief All the textures that this material writes to
         */
        std::optional<std::vector<bound_resource>> output_textures;

        /*!
         * \brief The depth texture to use for this material
         */
        std::optional<bound_resource> depth_texture;

        /*!
         * \brief The filter string used to get data for this material
         */
        std::optional<std::string> filters;

        /*!
         * \brief The material to use if this one's shaders can't be found
         */
        std::optional<std::string> fallback;

        /*!
         * \brief A bias to apply to the depth
         */
        std::optional<float> depth_bias;

        /*!
         * \brief The depth bias, scaled by slope I guess?
         */
        std::optional<float> slope_scaled_depth_bias;

        /*!
         * \brief The reference value to use for the stencil test
         */
        std::optional<uint32_t> stencil_ref;

        /*!
         * \brief The mastk to use when reading from the stencil buffer
         */
        std::optional<uint32_t> stencil_read_mask;

        /*!
         * \brief The mask to use when writing to the stencil buffer
         */
        std::optional<uint32_t> stencil_write_mask;

        /*!
         * \brief How to handle MSAA for this state
         */
        std::optional<msaa_support_enum> msaa_support;

        /*!
         * \brief
         */
        std::optional<primitive_topology_enum> primitive_mode;

        /*!
         * \brief Where to get the blending factor for the soource
         */
        std::optional<blend_factor_enum> source_blend_factor;

        /*!
         * \brief Where to get the blending factor for the destination
         */
        std::optional<blend_factor_enum> destination_blend_factor;

        /*!
         * \brief How to get the source alpha in a blend
         */
        std::optional<blend_factor_enum> alpha_src;

        /*!
         * \brief How to get the destination alpha in a blend
         */
        std::optional<blend_factor_enum> alpha_dst;

        /*!
         * \brief The function to use for the depth test
         */
        std::optional<compare_op_enum> depth_func;

        /*!
         * \brief The render queue that this pass belongs to
         *
         * This may or may not be removed depending on what is actually needed by Nova
         */
        std::optional<render_queue_enum> render_queue;

        /*!
         * \brief Map from shader file name to compiled SPIR-V. All shaders are compiled to SPIR-V when they're loaded,
         * and each backend can do with them as it will
         */
        std::unordered_map<std::string, std::vector<uint32_t>> sources;

        /*!
         * \brief Constructs a new pipeline from the provided JSON
         *
         * This constructor simply reads in the data from the JSON object that represents it. It won't fill in any
         * fields that are missing from the JSON - that happens at a later time
         *
         * \param pass_name The name of this pipeline
         * \param parent_pass_name The name of the pipeline that this pipeline inherits from
         * \param pass_json The JSON that this pipeline will be created from
         */
        pipeline_data(const std::string& pass_name, const std::optional<std::string>& parent_pass_name, const nlohmann::json& pass_json);

        pipeline_data() = default;
    };

    SMART_ENUM(pixel_format_enum,
               RGB8,
               RGBA8,
               RGB16F,
               RGBA16F,
               RGB32F,
               RGBA32F,
               Depth,
               DepthStencil,
    )

    SMART_ENUM(texture_dimension_type_enum,
               ScreenRelative,
               Absolute
    )

    struct texture_format {
        /*!
         * \brief The format of the texture
         */
        pixel_format_enum pixel_format;

        /*!
         * \brief How to interpret the dimensions of this texture
         */
        texture_dimension_type_enum dimension_type;

        /*!
         * \brief The width, in pixels, of the texture
         */
        float width;
        /*!
         * \brief The height, in pixels, of the texture
         */
        float height;
    };

    void from_json(const nlohmann::json& j, texture_format& format);

    bool operator==(const texture_format &rhs, const texture_format &lhs);

    bool operator!=(const texture_format &rhs, const texture_format &lhs);

    /*!
     * \brief A texture that a pass can use
     */
    struct texture_resource {
        /*!
         * \brief The name of the texture
         *
         * Nova implicitly defines a few textures for you to use:
         * - ColorVirtualTexture
         *      - Virtual texture atlas that holds color textures
         *      - Textures which have the exact name as requested by Minecraft are in this atlas
         *      - Things without a color texture get a pure white texture
         *      - Always has a format of R8G8B8A8
         *      - Can only be used as a pass's input
         * - NormalVirtualTexture
         *      - Virtual texture atlas that holds normal textures
         *      - Textures which have `_n` after the name requested by Minecraft are in this atlas
         *      - If no normal texture exists for a given object, a texture with RGBA of (0, 0, 1, 1) is used
         *      - Always has a format of R8G8B8A8
         *      - Can only be used as a pass's input
         * - DataVirtualTexture
         *      - Virtual texture atlas that holds data textures
         *      - Textures which have a `_s` after the name requested by Minecraft are in this atlas
         *      - If no data texture exists for a given object, a texture with an RGBA of (0, 0, 0, 0) is used
         *      - Always has a format of R8G8B8A8
         *      - Can only be used as a pass's input
         * - Lightmap
         *      - Lightmap, loaded from the current resourcepack
         *      - Format of RGB8
         *      - Can only be used as an input
         * - Backbuffer
         *      - The texture that gets presented to the screen
         *      - Always has a format of RGB8
         *      - Can only be used as a pass's output
         *
         * If you use one of the virtual textures, then all fields except the binding are ignored
         * If you use `Backbuffer`, then all fields are ignored since the backbuffer is always bound to output location 0
         */
        std::string name;

        texture_format format;
    };

    void from_json(const nlohmann::json& j, texture_resource& tex);

    struct shaderpack_resources {
        std::vector<texture_resource> textures;
    };

    void from_json(const nlohmann::json& j, shaderpack_resources& res);

    /*!
     * \brief A description of a texture that a render pass outputs to
     */
    struct texture_attachment {
        /*!
         * \brief The name of the texture
         */
        std::string name;

        /*!
         * \brief Whether to clear it
         *
         * If the texture is a depth buffer, it gets cleared to 1
         * If the texture is a stencil buffer, it gets cleared to 0xFFFFFFFF
         * If the texture is a color buffer, it gets cleared to (0, 0, 0, 0)
         */
        bool clear = false;
    };

    void from_json(const nlohmann::json& j, texture_attachment& tex);

    /*!
     * \brief Holds all the lists of textures that this pass needs as input
     */
    struct input_textures {
        /*!
         * \brief The textures that are bound to a pipeline that a shader may access anywhere. May be any size
         */
        std::vector<std::string> bound_textures;

        /*!
         * \brief The textures bound to this pass as color attachments. These may only be sampled at the current pixel
         * and must be the same size as the output textures
         */
        std::vector<std::string> color_attachments;
    };

    void from_json(const nlohmann::json& j, input_textures& inputs);

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
    struct render_pass {
        /*!
         * \brief The name of thie render pass
         */
        std::string name;

        /*!
         * \brief The materials that MUST execute before this one
         */
        std::vector<std::string> dependencies;

        /*!
         * \brief The textures that this pass will read from
         */
        std::optional<input_textures> texture_inputs;
        /*!
         * \brief The textures that this pass will write to
         */
        std::vector<texture_attachment> texture_outputs;
        /*!
         * \brief The depth texture this pass will write to
         */
        std::optional<texture_attachment> depth_texture;

        render_pass() = default;
    };

    void from_json(const nlohmann::json& j, render_pass& pass);

    /*!
     * \brief All the data that can be in a shaderpack
     */
    struct shaderpack_data {

        std::unordered_map<std::string, pipeline_data> pipelines;

        //std::unordered_map<std::string, material_data> materials;

        //std::unordered_map<std::string, texture_data> textures;
    };
}

#endif //NOVA_RENDERER_SHADERPACK_DATA_HPP
