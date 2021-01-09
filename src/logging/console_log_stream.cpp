#include "console_log_stream.hpp"

#include <stdio.h>

namespace nova {
    StdoutStream::StdoutStream() : rx::stream(k_flush | k_write) {}

    rx_u64 StdoutStream::on_write(const uint8_t* data, const rx_u64 size) {
        fwrite(data, size, 1, stdout);
        return size;
    }

    bool StdoutStream::on_flush() {
        fflush(stdout);
        return true;
    }

    const std::string& StdoutStream::name() const& { return my_name; }
} // namespace nova
