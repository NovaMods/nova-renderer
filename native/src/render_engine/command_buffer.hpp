/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_COMMAND_BUFFER_HPP
#define NOVA_RENDERER_COMMAND_BUFFER_HPP

#include <functional>

namespace nova {
    /*!
     * \brief Platform-independent interface for a command buffer
     */
    class command_buffer {
        /*!
         * \brief Executes a provided function when this command buffer has finished executing
         * \param completion_handler The function to execute when this command buffer has finished executing
         */
        virtual void on_completion(std::function<void(void)> completion_handler) = 0;
    };
}

#endif //NOVA_RENDERER_COMMAND_BUFFER_HPP
