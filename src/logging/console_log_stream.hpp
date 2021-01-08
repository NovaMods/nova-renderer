#pragma once

#include <rx/core/stream.h>
#include <rx/core/string.h>

namespace nova {
    class StdoutStream final : public rx::stream {
    public:
        StdoutStream();
        
        virtual ~StdoutStream() = default;

        rx_u64 on_write(const rx_byte* data, rx_u64 size) override;

        bool on_flush() override;

        const std::string& name() const& override;

    private:
        std::string my_name = "NovaConsoleLogStream";
    };
} // namespace nova
