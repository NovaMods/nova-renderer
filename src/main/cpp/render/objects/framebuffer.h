/*!
 * \author gold1
 * \date 01-Jun-17.
 */

#ifndef RENDERER_FRAMEBUFFER_H
#define RENDERER_FRAMEBUFFER_H

#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <set>
#include <vulkan/vulkan.hpp>

namespace nova {
    /*!
     * \brief A framebuffer object
     *
     * Framebuffers may have multiple attachments, and each attachment may be enabled for writing. All attachments are
     * enabled for reading all the time
     */
    class framebuffer {
        friend class framebuffer_builder;

    public:
        framebuffer(framebuffer &&other) noexcept;

        ~framebuffer();

        void bind();

        void generate_mipmaps();

        void enable_writing_to_attachment(unsigned int attachment);

        void reset_drawbuffers();

        void create_depth_buffer();

    private:
        GLuint framebuffer_id;

        /*!
         * \brief All the color attachments that this framebuffer has
         *
         * These attachments are stored in a map from the index in a shader (0, 1, 2...) to their OpenGL name. This allows
         * easy operations on the attachments (such as
         */
        std::unordered_map<int, GLuint> color_attachments_map;

        std::set<GLenum> drawbuffers;

        GLuint* color_attachments;
        bool has_depth_buffer = false;

        framebuffer(unsigned int width, unsigned int height, unsigned int num_color_attachments);

        vk::Format choose_supported_format(vk::Format* formats, int num_formats, vk::ImageTiling tiling, vk::FormatFeatureFlags flags);

        vk::Format depth_format;
    };

    /*!
     * \brief Creates a framebuffer
     *
     * Framebuffers in Nova are pretty complex. Shaders can enable or disable various color attachments, but dealing with
     * that at runtime is a bitch. \class framebuffer_builder exposes a fluent API which creates a framebuffer without any
     * fuss. Even better, a framebuffer_builder can be reused as a window changes size, requiring a minimum amount of
     * recalculation!
     */
    class framebuffer_builder {
    public:
        /*!
         * \brief Sets the framebuffer size to the specified value
         *
         * \param width The desired width of the framebuffer
         * \param height The desired height of the framebuffer
         * \return This framebuffer_builder object
         */
        framebuffer_builder &set_framebuffer_size(unsigned int width, unsigned int height);

        /*!
         * \brief Enables the specified color attachment so that a texture is created for it
         *
         * \param color_attachment The index of the color attachment to enable
         * \return  This framebuffer_builder object
         */
        framebuffer_builder &enable_color_attachment(unsigned int color_attachment);

        /*!
         * \brief Disables the specified color attachment so that no texture is created for it
         *
         * \param color_attachment The index of the color attachment to disable
         * \return  This framebuffer_builder object
         */
        framebuffer_builder &disable_color_attachment(unsigned int color_attachment);

        /*!
         * \brief Creates a framebuffer with the settings specified in this framebuffer_builder
         *
         * \return The newly constructed framebuffer object
         */
        framebuffer build();

        /*!
         * \brief Resets this framebuffer builder, clearing all the information stored within
         */
        void reset();

    private:
        std::set<unsigned int> enabled_color_attachments;
        unsigned int width;
        unsigned int height;
    };
}

#endif //RENDERER_FRAMEBUFFER_H
