/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_COMMAND_BUFFER_HPP
#define NOVA_RENDERER_COMMAND_BUFFER_HPP

#include <functional>
#include <glm/glm.hpp>
#include "../util/smart_enum.hpp"
#include "resource_barrier.hpp"

namespace nova {
    struct iframebuffer;
    struct iresource;
    struct iqueue;

    typedef struct iframebuffer* framebuffer_ptr;

    /*!
     * \brief The type of command list we want to create. This is important for DX12 but I don't think it's important
     * for Vulkan - but it needs to be part of the DX12 interface, so here we are
     */
    SMART_ENUM(command_buffer_type,
        /*!
         * \brief Non-specialized command buffer. Can be used for any tasks, although a more specialized command buffer
         * might be better for certain tasks
         */
        GENERIC,

        /*!
         * \brief Command buffer will be used for copying data from one buffer to another (e.g. from a staging buffer
         * to a texture)
         */
        COPY,

        /*!
         * \brief Command buffer will be used for compute tasks
         */
        COMPUTE
    );

    /*!
     * \brief Platform-independent interface for a command buffer
     */
    class command_buffer_base {
    public:
        explicit command_buffer_base(command_buffer_type type);

        /*!
         * \brief Executes a provided function when this command buffer has finished executing
         * \param completion_handler The function to execute when this command buffer has finished executing
         */
        virtual void on_completion(std::function<void(void)> completion_handler) = 0;

        /*!
         * \brief Ends recording for this command buffer, letting the underlying graphics API know that we're done
         */
        virtual void end_recording() = 0;

        /*!
         * \brief Resets this command buffer, making its resources availabe for future use
         */
        virtual void reset() = 0;

        /*!
         * \brief Retrieves the type of this command buffer
         *
         * This seems to be mainly useful for Vulkan knowing which queue to submit this command buffer to, but it'll
         * also be at least a little useful for debugging
         *
         * \return The type of this command buffer
         */
        command_buffer_type get_type() const;

    private:
        command_buffer_type type;
    };

    /*!
     * \brief Abstraction for a command buffer that can do graphics work
     */
    class graphics_command_buffer_base : public virtual command_buffer_base {
    public:
        graphics_command_buffer_base();

        /*!
         * \brief Adds one or more resource barriers
         * \param barriers The barriers to add
         */
        virtual void resource_barrier(const std::vector<resource_barrier_data>& barriers) = 0;

        /*!
         * \brief Clears the current render target to the provided clear color
         * \param clear_color The color to clear to
         */
        virtual void clear_render_target(std::shared_ptr<iframebuffer> framebuffer_to_clear, glm::vec4& clear_color) = 0;

        /*!
         * \brief Sets the render target to render to
         *
         * \param render_target A pointer to the render target to render to
         */
        virtual void set_render_target(std::shared_ptr<iframebuffer> render_target) = 0;
    };
}

#endif //NOVA_RENDERER_COMMAND_BUFFER_HPP
