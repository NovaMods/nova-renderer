/*! 
 * \author gold1 
 * \date 19-Jul-17.
 */

#include "direct_buffers.h"

namespace nova {
    void direct_buffers::create_buffer(std::string name, unsigned long num_bytes) {
        auto vec = std::vector<unsigned char>();
        vec.reserve(num_bytes);
        buffers[name] = vec;
    }

    std::vector<unsigned char> &direct_buffers::operator[](std::string buffer_name) {
        auto buffer_pos = buffers.find(buffer_name);
        if(buffer_pos != buffers.end()) {
            return buffers[buffer_name];
        }

        throw std::runtime_error("No buffer named " + buffer_name);
    }
}
