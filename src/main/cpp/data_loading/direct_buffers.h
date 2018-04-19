/*! 
 * \author gold1 
 * \date 19-Jul-17.
 */

#ifndef RENDERER_DIRECT_BUFFERS_H
#define RENDERER_DIRECT_BUFFERS_H

#include <string>
#include <vector>
#include <unordered_map>

namespace nova {
/*!
 * \brief Holds all the direct mapped buffers that allow Java nad C++ to talk directly to each other
 */
    class direct_buffers {
    public:
        /*!
         * \brief Creates a new buffer with the specified size
         *
         * \param name The name of the new buffer
         * \param num_bytes The size in bytes of the new buffer
         */
        void create_buffer(std::string name, unsigned long num_bytes);

        /*!
         * \brief Allows access to buffers by name
         *
         * If the buffer is not available, a runtime_error is thrown
         *
         * \param buffer_name The name of the buffer to access
         * \return A reference to the requested buffer
         */
        std::vector<unsigned char> &operator[](std::string buffer_name);

    private:
        std::unordered_map<std::string, std::vector<unsigned char>> buffers;
    };
}

#endif //RENDERER_DIRECT_BUFFERS_H
