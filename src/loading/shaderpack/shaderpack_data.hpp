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
#include <nlohmann/json.hpp>
#include "../../util/filesystem.hpp"
#include "../../util/utils.hpp"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <unordered_set>

namespace nova {
    NOVA_EXCEPTION(validation_failure_exception);

    /*!
     * \brief Controls the rasterizer's state
     */
    enum class state_enum {
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

    enum class texture_filter_enum { TexelAA, Bilinear, Point };

    enum class wrap_mode_enum { Repeat, Clamp };

    /*!
     * \brief The kind of data in a vertex attribute
     */
    enum class vertex_field_enum {
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
    };

    /*!
     * \brief Where the texture comes from
     */
    enum class texture_location_enum {
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

    enum class msaa_support_enum { MSAA, Both, None };

    enum class stencil_op_enum { Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert };

    enum class compare_op_enum { Never, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual, Always };

    enum class primitive_topology_enum { Triangles, Lines };

    enum class blend_factor_enum { One, Zero, SrcColor, DstColor, OneMinusSrcColor, OneMinusDstColor, SrcAlpha, DstAlpha, OneMinusSrcAlpha, OneMinusDstAlpha };

    enum class render_queue_enum { Transparent, Opaque, Cutout };

    enum class pixel_format_enum {
        RGBA8,
        RGBA16F,
        RGBA32F,
        Depth,
        DepthStencil,
    };

    enum class texture_dimension_type_enum { ScreenRelative, Absolute };

    /*!
     * \brief Defines a sampler to use for a texture
     *
     * At the time of writing I'm not sure how this is corellated with a texture, but all well
     */
    struct sampler_state_data {
        std::string name;

        /*!
         * \brief What kind of texture filter to use
         *
         * texel_aa does something that I don't want to figure out right now. Bilinear is your regular bilinear filter,
         * and point is the point filter. Aniso isn't an option and I kinda hope it stays that way
         */
        texture_filter_enum filter;

        /*!
         * \brief How the texture should wrap at the edges
         */
        wrap_mode_enum wrap_mode;
    };

    struct stencil_op_state {
        stencil_op_enum fail_op;
        stencil_op_enum pass_op;
        stencil_op_enum depth_fail_op;
        compare_op_enum compare_op;
        uint32_t compare_mask;
        uint32_t write_mask;
    };

    struct shader_source {
        fs::path filename;
        std::vector<uint32_t> source;
    };

    struct vertex_field_data {
        std::string semantic_name;
        vertex_field_enum field;
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
        std::string pass;

        /*!
         * \brief All of the symbols in the shader that are defined by this state
         */
        std::vector<std::string> defines;

        /*!
         * \brief Defines the rasterizer state that's active for this pipeline
         */
        std::vector<state_enum> states;

        /*!
         * \brief Sets up the vertex fields that Nova will bind to this pipeline
         *
         * The index in the array is the attribute index that the vertex field is bound to
         */
        std::vector<vertex_field_data> vertex_fields;

        /*!
         * \brief The stencil buffer operations to perform on the front faces
         */
        std::optional<stencil_op_state> front_face;

        /*!
         * \brief The stencil buffer operations to perform on the back faces
         */
        std::optional<stencil_op_state> back_face;

        /*!
         * \brief The material to use if this one's shaders can't be found
         */
        std::optional<std::string> fallback;

        /*!
         * \brief A bias to apply to the depth
         */
        float depth_bias;

        /*!
         * \brief The depth bias, scaled by slope I guess?
         */
        float slope_scaled_depth_bias;

        /*!
         * \brief The reference value to use for the stencil test
         */
        uint32_t stencil_ref;

        /*!
         * \brief The mask to use when reading from the stencil buffer
         */
        uint32_t stencil_read_mask;

        /*!
         * \brief The mask to use when writing to the stencil buffer
         */
        uint32_t stencil_write_mask;

        /*!
         * \brief How to handle MSAA for this state
         */
        msaa_support_enum msaa_support;

        /*!
         * \brief
         */
        primitive_topology_enum primitive_mode;

        /*!
         * \brief Where to get the blending factor for the soource
         */
        blend_factor_enum source_blend_factor;

        /*!
         * \brief Where to get the blending factor for the destination
         */
        blend_factor_enum destination_blend_factor;

        /*!
         * \brief How to get the source alpha in a blend
         */
        blend_factor_enum alpha_src;

        /*!
         * \brief How to get the destination alpha in a blend
         */
        blend_factor_enum alpha_dst;

        /*!
         * \brief The function to use for the depth test
         */
        compare_op_enum depth_func;

        /*!
         * \brief The render queue that this pass belongs to
         *
         * This may or may not be removed depending on what is actually needed by Nova
         */
        render_queue_enum render_queue;

        shader_source vertex_shader;

        std::optional<shader_source> geometry_shader;
        std::optional<shader_source> tessellation_control_shader;
        std::optional<shader_source> tessellation_evaluation_shader;
        std::optional<shader_source> fragment_shader;

        /*!
         * \brief Merges this pipeline with the parent, returning the merged pipeline
         */
        pipeline_data merge_with_parent(const pipeline_data &parent_pipeline) const;
    };

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

        glm::uvec2 get_size_in_pixels(const glm::uvec2& screen_size) const;

        bool operator==(const texture_format &other) const;
        bool operator!=(const texture_format &other) const;
    };

    /*!
     * \brief A texture that a pass can use
     */
    struct texture_resource_data {
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

    struct shaderpack_resources_data {
        std::vector<texture_resource_data> textures;
        std::vector<sampler_state_data> samplers;
    };

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

        bool operator==(const texture_attachment& other) const;
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
    struct render_pass_data {
        /*!
         * \brief The name of this render pass
         */
        std::string name;

        /*!
         * \brief The materials that MUST execute before this one
         */
        std::vector<std::string> dependencies;

        /*!
         * \brief The textures that this pass will read from
         */
        std::vector<std::string> texture_inputs;
        /*!
         * \brief The textures that this pass will write to
         */
        std::vector<texture_attachment> texture_outputs;
        /*!
         * \brief The depth texture this pass will write to
         */
        std::optional<texture_attachment> depth_texture;

        /*!
         * \brief All the buffers that this renderpass reads from
         */
        std::vector<std::string> input_buffers;

        /*!
         * \brief All the buffers that this renderpass writes to
         */
        std::vector<std::string> output_buffers;

        render_pass_data() = default;
    };

    struct material_pass {
        std::string name;
        std::string material_name;
        std::string pipeline;
        std::unordered_map<std::string, std::string> bindings;
        std::vector<VkDescriptorSet> descriptor_sets;
    };

    struct material_data {
        std::string name;
        std::vector<material_pass> passes;
        std::string geometry_filter;
    };

    /*!
     * \brief All the data that can be in a shaderpack
     */
    struct shaderpack_data {
        std::vector<pipeline_data> pipelines;

        /*!
         * \brief All the renderpasses that this shaderpack needs, in submission order
         */
        std::vector<render_pass_data> passes;

        std::vector<material_data> materials;

        shaderpack_resources_data resources;
    };

    pixel_format_enum pixel_format_enum_from_string(const std::string &str);
    texture_dimension_type_enum texture_dimension_type_enum_from_string(const std::string &str);
    texture_filter_enum texture_filter_enum_from_string(const std::string &str);
    wrap_mode_enum wrap_mode_enum_from_string(const std::string &str);
    stencil_op_enum stencil_op_enum_from_string(const std::string &str);
    compare_op_enum compare_op_enum_from_string(const std::string &str);
    msaa_support_enum msaa_support_enum_from_string(const std::string &str);
    primitive_topology_enum primitive_topology_enum_from_string(const std::string &str);
    blend_factor_enum blend_factor_enum_from_string(const std::string &str);
    render_queue_enum render_queue_enum_from_string(const std::string &str);
    state_enum state_enum_from_string(const std::string &str);
    vertex_field_enum vertex_field_enum_from_string(const std::string &str);

    std::string to_string(pixel_format_enum val);
    std::string to_string(texture_dimension_type_enum val);
    std::string to_string(texture_filter_enum val);
    std::string to_string(wrap_mode_enum val);
    std::string to_string(stencil_op_enum val);
    std::string to_string(compare_op_enum val);
    std::string to_string(msaa_support_enum val);
    std::string to_string(primitive_topology_enum val);
    std::string to_string(blend_factor_enum val);
    std::string to_string(render_queue_enum val);
    std::string to_string(state_enum val);
    std::string to_string(vertex_field_enum val);
}  // namespace nova

#endif  // NOVA_RENDERER_SHADERPACK_DATA_HPP
