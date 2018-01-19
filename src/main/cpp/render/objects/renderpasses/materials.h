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

using namespace std::experimental;

namespace nova {
#define GET_FIELD(mat, field) [&]() {\
    if(mat.field) { \
        return mat.field; \
    } else if(mat.parent) { \
        return mat.parent.field; \
    } else { \
        return mat.field; \
    }();

    /*!
     * \brief Controlls the rasterizer's state
     */
    enum class state_enum {
        /*!
         * \brief Enable blending for this material state
         */
        blending,

        /*!
         * \brief Render backfaces and cull frontfaces
         */
        invert_culing,

        /*!
         * \brief Don't cull backfaces or frontfaces
         */
        disable_culling,

        /*!
         * \brief Don't write to the depth buffer
         */
        disable_depth_write,

        /*!
         * \brief Don't perform a depth test
         */
        disable_depth_test,

        /*!
         * \brief Perform the stencil test
         */
        enable_stencil_test,

        /*!
         * \brief Write to the stencil buffer
         */
        stencil_write,

        /*!
         * \brief Don't write to the color buffer
         */
        disable_color_write,

        /*!
         * \brief Enable alpha to coverage
         */
        enable_alpha_to_coverage,

        /*!
         * \brief Don't write alpha
         */
        disable_alpha_write,
    };

    enum class texture_filter_enum {
        texel_aa,
        bilinear,
        point
    };

    enum class wrap_mode_enum {
        repeat,
        clamp
    };

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
    enum class vertex_field_enum {
        /*!
         * \brief The vertex position
         *
         * 12 bytes
         */
        position,

        /*!
         * \brief The vertex color
         *
         * 4 bytes
         */
        color,

        /*!
         * \brief The UV coordinate of this object
         *
         * Except not really, because Nova's virtual textures means that the UVs for a block or entity or whatever
         * could change on the fly, so this is kinda more of a preprocessor define that replaces the UV with a lookup
         * in the UV table
         *
         * 8 bytes (might try 4)
         */
        main_uv,

        /*!
         * \brief The UV coordinate in the lightmap texture
         *
         * This is a real UV and it doesn't change for no good reason
         *
         * 2 bytes
         */
        lightmap_uv,

        /*!
         * \brief Vertex normal
         *
         * 12 bytes
         */
        normal,

        /*!
         * \brief Vertex tangents
         *
         * 12 bytes
         */
        tangent,

        /*!
         * \brief The texture coordinate of the middle of the quad
         *
         * 8 bytes
         */
        mid_tex_coord,

        /*!
         * \brief A uint32_t that's a unique identifier for the texture that this vertex uses
         *
         * This is generated at runtime by Nova, so it may change a lot depending on what resourcepacks are loaded and
         * if they use CTM or random detail textures or whatever
         *
         * 4 bytes
         */
        virtual_texture_id,

        /*!
         * \brief Some information about the current block/entity/whatever
         *
         * 12 bytes
         */
        mc_entity_id,

        /*!
         * \brief Useful if you want to skip a vertex attribute
         */
        empty,
    };

    enum class texture_location_enum {
        dynamic,
        in_user_package,
        in_app_package
    };

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

    enum class msaa_support_enum {
        msaa,
        both,
        none
    };

    struct output_info {
        uint8_t index;
        bool blending;
    };

    struct stencil_op_state {
        optional<vk::StencilOp> fail_op;
        optional<vk::StencilOp> pass_op;
        optional<vk::StencilOp> depth_fail_op;
        optional<vk::CompareOp> compare_op;
        optional<uint32_t> compare_mask;
        optional<uint32_t> write_mask;

        vk::StencilOpState to_vk_stencil_op_state() const;
    };

    /*!
     * \brief Represents the configuration for a single pipeline
     */
    struct material_state {
        /*!
         * \brief The name of this material_state
         */
        std::string name;

        /*!
         * \brief The material_state that this material_state inherits from
         *
         * I may or may not make this a pointer to another material_state. Depends on how the code ends up being
         */
        optional<std::string> parent_name;

        /*!
         * \brief The actual parent material
         */
        optional<material_state*> parent;

        /*!
         * \brief All of the symbols in the shader that are defined by this state
         */
        optional<std::vector<std::string>> defines;

        /*!
         * \brief Defines the rasterizer state that's active for this material state
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
         * \brief Sets up the vertex fields that Nova will bind to this shader
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
         * \brief All the sampler states that are defined for this material_state. Still not sure how they work though
         */
        optional<std::vector<sampler_state>> sampler_states;

        /*!
         * \brief All the textures that this material state uses
         */
        optional<std::vector<texture>> textures;

        /*!
         * \brief The filter string used to get data for this material_state
         */
        optional<std::string> filters;

        /*!
         * \brief The material_state to use if this one's shaders can't be found
         */
        optional<std::string> fallback;

        /*!
         * \brief When this material state will be drawn
         *
         * Lower pass indices are drawn earlier, and larger pass indices are drawn later. If multiple material states
         * have the same pass index then Nova makes no guarantees about when they will be drawn relative to each other.
         * Pass indices to not have to be continuous
         */
        optional<uint32_t> pass_index;

        /*!
         * \brief The framebuffer attachments that this material pass outputs to
         *
         * The index in this array is the location of the output in the shader, and the index member of the
         * frameuffer_output struct is the index in the framebuffer. For example, a framebuffer_output at index 2 in
         * this array with an index member of 4 tells Nova that when the shader associated with this material state
         * outputs to location 2, that data should be written to colortex4. Alteriately, you can think of it as telling
         * Nova to bind colortex4 to shader output 2
         */
        optional<std::vector<output_info>> outputs;

        /*!
         * \brief The width of the output texture we're rendering to
         *
         * If this is not set by the .material file, then its value comes from the framebuffer that it renders to. I
         * mostly put this member in this struct as a convenient way to pass it into a shader creation
         */
        optional<uint32_t> output_width;

        /*!
         * \brief The height of the output texture we're rendering to
         *
         * If this is not set by the .material file, then its value comes from the framebuffer that it renders to. I
         * mostly put this member in this struct as a convenient way to pass it into a shader creation
         */
        optional<uint32_t> output_height;

        /*!
         * \brief A bias to apply to the depth
         */
        optional<float> depth_bias;

        /*!
         * \brief The depth bias, scaled by slope I guess?
         */
        optional<float> slope_scaled_depth_bias;

        /*!
         * \brief A reference to a stencil somehow?
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
        optional<vk::PrimitiveTopology> primitive_mode;

        /*!
         * \brief Where to get the blending factor for the soource
         */
        optional<vk::BlendFactor> source_blend_factor;

        /*!
         * \brief Where to get the blending factor for the destination
         */
        optional<vk::BlendFactor> destination_blend_factor;

        /*!
         * \brief How to get the source alpha in a blend
         */
        optional<vk::BlendFactor> alpha_src;

        /*!
         * \brief How to get the destination alpha in a blend
         */
        optional<vk::BlendFactor> alpha_dst;

        /*!
         * \brief The function to use for the depth test
         */
        optional<vk::CompareOp> depth_func;

        /*!
         * \brief Tells Nova if this state handles transparent objects
         */
        optional<bool> has_transparency;

        /*!
         * \brief Tells Nova is this state handles cutout objects
         */
        optional<bool> has_cutout;
    };

    material_state create_material_from_json(const std::string& material_state_name, const optional<std::string>& parent_state_name, const nlohmann::json& material_json);

    /*!
     * \brief Translates a string from a material file to a state_enum value
     * \param state_to_decode The string to translate to a state
     * \return The decoded state
     */
    state_enum decode_state(const std::string& state_to_decode);

    /*!
     * \brief Translates a JSON object into a sampler_state object
     * \param json The JSON object to translate
     * \return The translated sampler_state
     */
    sampler_state decode_sampler_state(const nlohmann::json& json);

    /*!
     * \brief Translates a string from a material file to a texture_filter_enum value
     * \param texture_filter_enum_str The string to translate into a texture_filter_enum
     * \return The translated texture_filter_enum
     */
    texture_filter_enum decode_texture_filter_enum(const std::string &texture_filter_enum_str);

    /*!
     * \brief Translates a string from a material file into a texture_wrap_mode_enum value
     * \param wrap_mode The string to translate into a texture_wrap_mode_enum
     * \return The translated texture_wrap_mode_enum
     */
    wrap_mode_enum decode_wrap_mode_enum(const std::string &wrap_mode);

    /*!
     * \brief Translates a JSON object into a member of the vertex field enum
     * \param wrap_mode The string to translate into a vertex_field_enum
     * \return The translated vertex_field_enum
     */
    vertex_field_enum decode_vertex_field(const nlohmann::json &vertex_field_json);

    vk::CompareOp decode_comparison_func_enum(const std::string& comparison_func);

    vk::StencilOp decode_stencil_op_enum(const std::string &op);

    stencil_op_state decode_stencil_buffer_state(const nlohmann::json &json);

    msaa_support_enum decode_msaa_support_enum(const std::string& msaa_support_str);

    vk::PrimitiveTopology decode_primitive_mode_enum(const std::string& primitive_mode_str);

    vk::BlendFactor decode_blend_source_enum(const std::string& blend_source_str);

    texture decode_texture(const nlohmann::json& texture_json);

    texture_location_enum decode_texture_location_enum(const std::string& texture_location_str);

    output_info decode_outputs(const nlohmann::json& output_info_json);
}

#endif //RENDERER_MATERIALS_H
