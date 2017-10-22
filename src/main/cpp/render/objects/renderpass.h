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
     * \brief A renderpass object
     *
     * I've made this wrapper to better manage adding and accessing framebuffer attachments,
     */
    class renderpass {
        friend class renderpass_builder;

    public:
        explicit renderpass(vk::RenderPassCreateInfo& create_info);

        renderpass(renderpass &&other) noexcept;

        ~renderpass();

        void bind();

        void generate_mipmaps();

        void enable_writing_to_attachment(unsigned int attachment);

        void reset_drawbuffers();

        void create_depth_buffer();

    private:
        GLuint framebuffer_id;

        std::set<GLenum> drawbuffers;

        GLuint* color_attachments;
        bool has_depth_buffer = false;

        vk::RenderPass vk_renderpass;
    };

    /*!
     * \brief A builder for renderbuffers
     */
    class renderpass_builder {
    public:
        /*!
         * \brief Sets the framebuffer size to the specified value
         *
         * \param width The desired width of the framebuffer
         * \param height The desired height of the framebuffer
         * \return This framebuffer_builder object
         */
        renderpass_builder &set_framebuffer_size(unsigned int width, unsigned int height);

        /*!
         * \brief Adds a color buffer to the renderpass
         *
         * Insertion order is maintained
         */
        renderpass_builder &add_color_buffer();

        /*!
         * \brief Creates a framebuffer with the settings specified in this framebuffer_builder
         *
         * \return The newly constructed framebuffer object
         */
        std::unique_ptr<renderpass> build();

        /*!
         * \brief Resets this framebuffer builder, clearing all the information stored within
         */
        void reset();

    private:
        unsigned int width;
        unsigned int height;

        std::vector<vk::AttachmentDescription> attachments;
        std::vector<vk::AttachmentReference> color_refs;
    };
}

#endif //RENDERER_FRAMEBUFFER_H
