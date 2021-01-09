#include "material_data_buffer.hpp"

#include <rx/core/utility/move.h>

namespace nova::renderer {
    MaterialDataBuffer::MaterialDataBuffer(size_t num_bytes) : buffer(std::move(buffer)) {}

    uint8_t* MaterialDataBuffer::data() const { return buffer.data; }
} // namespace nova::renderer
