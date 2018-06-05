/*!
 * \author ddubois 
 * \date 29-Mar-18.
 */

#ifndef NOVA_RENDERER_RENDER_PASSES_H
#define NOVA_RENDERER_RENDER_PASSES_H

#include <json.hpp>
#include "../../../utils/smart_enum.h"
#include <optional.hpp>

using namespace std::experimental;

namespace nova {
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

        texture_resource() = default;

        explicit texture_resource(const nlohmann::json &json);
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
    };

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
        optional<std::vector<std::string>> dependencies;

        /*!
         * \brief The textures that this pass will read from
         */
        optional<input_textures> texture_inputs;
        /*!
         * \brief The textures that this pass will write to
         */
        optional<std::vector<texture_attachment>> texture_outputs;
        /*!
         * \brief The depth texture this pass will write to
         */
        optional<texture_attachment> depth_texture;

        /*!
         * \brief Parses the provided JSON into a render pass
         * \param pass_json The JSON object that holds this pass
         */
        explicit render_pass(const nlohmann::json &pass_json);

        render_pass() = default;
    };
}

#endif //NOVA_RENDERER_RENDER_PASSES_H
