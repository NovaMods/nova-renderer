#include "material_data_buffer.hpp"

#include <rx/core/utility/move.h>

namespace nova::renderer {
    MaterialDataBuffer::MaterialDataBuffer(rx::memory::view buffer) : buffer(rx::utility::move(buffer)) {}

    rx_byte* MaterialDataBuffer::data() const { return buffer.data; }
} // namespace nova::renderer
