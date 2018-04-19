/*!
 * \brief Contains struct to represent .material files
 * \author ddubois 
 * \date 01-Nov-17.
 */

#ifndef RENDERER_MATERIALS_H
#define RENDERER_MATERIALS_H

#include <string>
#include <vector>
#include <json.hpp>
#include <optional.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include "../../../utils/smart_enum.h"
#include "../../../data_loading/loaders/shader_source_structs.h"

using namespace std::experimental;

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
        optional<uint32_t> sampler_index;

        /*!
         * \brief What kind of texture filter to use
         *
         * texel_aa does something that I don't want to figure out right now. Bilinear is your regular bilinear filter,
         * and point is the point filter. Aniso isn't an option and I kinda hope it stays that way
         */
        optional<texture_filter_enum> filter;

        /*!
         * \brief How the texutre should wrap at the edges
         */
        optional<wrap_mode_enum> wrap_mode;
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

    SMART_ENUM(texture_location_enum,
               Dynamic,
               InUserPackage,
               InAppPackage
    )

    /*!
     * \brief A texture definition in a material file
     *
     * This class simple describes where to get the texture data from.
     */
    struct texture {
        /*!
         * \brief The index of the texture
         *
         * In pure Bedrock mode this is part of the texture name, e.g. setting the index to 0 means the texture will be
         * bound to texture name TEXTURE0. If you don't name your textures according to this format, then the index is
         * the binding point of the texture, so an index of 0 would put this texture at binding point 0
         */
        uint32_t index;

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
        optional<bool> calculate_mipmaps;
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
        optional<stencil_op_enum> fail_op;
        optional<stencil_op_enum> pass_op;
        optional<stencil_op_enum> depth_fail_op;
        optional<compare_op_enum> compare_op;
        optional<uint32_t> compare_mask;
        optional<uint32_t> write_mask;

        vk::StencilOpState to_vk_stencil_op_state() const;
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
     * \brief A pipeline that can render certain groups of geometry
     *
     * A pipeline has a few things:
     * - A geometry_filter that determines what this pass renders
     * - Rasterizer state, like how to perform the depth and stencil test
     * - Blending state
     * - What shaders to use
     *
     * When a pipeline has its values filled in from a parent, each field that is not present in the child pipeline is
     * taken straight from the parent, and each field that is present in the child is not changed. That is to say,
     * vectors are NOT combined between the child and parent
     */
    struct pipeline {
        /*!
         * \brief The name of this pipeline
         */
        std::string name;

        /*!
         * \brief The pipeline that this pipeline inherits from
         *
         * I may or may not make this a pointer to another pipeline. Depends on how the code ends up being
         */
        optional<std::string> parent_name;

        /*!
         * \brief The name of the pass that this pipeline belongs to
         */
        optional<std::string> pass;

        /*!
         * \brief All of the symbols in the shader that are defined by this state
         */
        optional<std::vector<std::string>> defines;

        /*!
         * \brief Defines the rasterizer state that's active for this pipeline
         */
        optional<std::vector<state_enum>> states;

        /*!
         * \brief The path from the resourcepack or shaderpack root to the vertex shader
         *
         * Except not really, cause if you leave off the extension then Nova will try using the `.vert` and `.vsh`
         * extensions. This is kinda just a hint
         */
        optional<std::string> vertex_shader;

        /*!
         * \brief The path from the resourcepack or shaderpack root to the fragment shader
         *
         * Except not really, cause if you leave off the extension then Nova will try using the `.frag` and `.fsh`
         * extensions. This is kinda just a hint
         */
        optional<std::string> fragment_shader;

        /*!
         * \brief The path from the resourcepack or shaderpack root to the geometry shader
         *
         * Except not really, cause if you leave off the extension then Nova will try using the `.geom` and `.geo`
         * extensions. This is kinda just a hint
         */
        optional<std::string> geometry_shader;

        /*!
         * \brief The path from the resourcepack or shaderpack root to the tessellation evaluation shader
         *
         * Except not really, cause if you leave off the extension then Nova will try using the `.test` and `.tse`
         * extensions. This is kinda just a hint
         */
        optional<std::string> tessellation_evaluation_shader;

        /*!
         * \brief The path from the resourcepack or shaderpack root to the tessellation control shader
         *
         * Except not really, cause if you leave off the extension then Nova will try using the `.tesc` and `.tsc`
         * extensions. This is kinda just a hint
         */
        optional<std::string> tessellation_control_shader;

        /*!
         * \brief Sets up the vertex fields that Nova will bind to this pipeline
         *
         * The index in the array is the attribute index that the vertex field is bound to
         */
        optional<std::vector<vertex_field_enum>> vertex_fields;

        /*!
         * \brief The stencil buffer operations to perform on the front faces
         */
        optional<stencil_op_state> front_face;

        /*!
         * \brief The stencil buffer operations to perform on the back faces
         */
        optional<stencil_op_state> back_face;

        /*!
         * \brief All the textures that this material reads from
         */
        optional<std::vector<bound_resource>> input_textures;

        /*!
         * \brief All the textures that this material writes to
         */
        optional<std::vector<bound_resource>> output_textures;

        /*!
         * \brief The depth texture to use for this material
         */
        optional<bound_resource> depth_texture;

        /*!
         * \brief The filter string used to get data for this material
         */
        optional<std::string> filters;

        /*!
         * \brief The material to use if this one's shaders can't be found
         */
        optional<std::string> fallback;

        /*!
         * \brief A bias to apply to the depth
         */
        optional<float> depth_bias;

        /*!
         * \brief The depth bias, scaled by slope I guess?
         */
        optional<float> slope_scaled_depth_bias;

        /*!
         * \brief The reference value to use for the stencil test
         */
        optional<uint32_t> stencil_ref;

        /*!
         * \brief The mastk to use when reading from the stencil buffer
         */
        optional<uint32_t> stencil_read_mask;

        /*!
         * \brief The mask to use when writing to the stencil buffer
         */
        optional<uint32_t> stencil_write_mask;

        /*!
         * \brief How to handle MSAA for this state
         */
        optional<msaa_support_enum> msaa_support;

        /*!
         * \brief
         */
        optional<primitive_topology_enum> primitive_mode;

        /*!
         * \brief Where to get the blending factor for the soource
         */
        optional<blend_factor_enum> source_blend_factor;

        /*!
         * \brief Where to get the blending factor for the destination
         */
        optional<blend_factor_enum> destination_blend_factor;

        /*!
         * \brief How to get the source alpha in a blend
         */
        optional<blend_factor_enum> alpha_src;

        /*!
         * \brief How to get the destination alpha in a blend
         */
        optional<blend_factor_enum> alpha_dst;

        /*!
         * \brief The function to use for the depth test
         */
        optional<compare_op_enum> depth_func;

        /*!
         * \brief The render queue that this pass belongs to
         *
         * This may or may not be removed depending on what is actually needed by Nova
         */
        optional<render_queue_enum> render_queue;

        /*!
         * \brief The source of each shader, read from disk
         */
        shader_definition shader_sources;

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
        pipeline(const std::string& pass_name, const optional<std::string>& parent_pass_name, const nlohmann::json& pass_json);

        pipeline() = default;
    };

    struct material_pass {
        std::string material_name;
        std::string pipeline;
        std::unordered_map<std::string, std::string> bindings;

        explicit material_pass(const nlohmann::json& json);
    };

    struct material {
        explicit material(const nlohmann::json& json);

        std::string name;
        std::vector<material_pass> passes;
        std::string geometry_filter;
    };

    /*!
     * \brief Translates a JSON object into a sampler_state object
     * \param json The JSON object to translate
     * \return The translated sampler_state
     */
    sampler_state decode_sampler_state(const nlohmann::json& json);

    stencil_op_state decode_stencil_buffer_state(const nlohmann::json &json);

    texture decode_texture(const nlohmann::json& texture_json);

    bound_resource decode_bound_texture(const nlohmann::json& json);

    vk::PrimitiveTopology to_vk_topology(primitive_topology_enum tolopogy);

    vk::CompareOp to_vk_compare_op(compare_op_enum op);

    vk::StencilOp to_vk_stencil_op(stencil_op_enum op);

    vk::BlendFactor to_vk_blend_factor(blend_factor_enum fac);
}

#endif //RENDERER_MATERIALS_H
