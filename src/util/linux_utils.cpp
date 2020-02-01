#include "linux_utils.hpp"

#include <exception>

#include <cxxabi.h>
#include <errno.h>
#include <execinfo.h>
#include <rx/core/array.h>
#include <rx/core/log.h>
#include <stdlib.h>
#include <string.h>

void nova_backtrace() {
    RX_LOG("LinuxUtil", logger);

    rx::array<void* [50]> array {};

    // get void*'s for all entries on the stack
    int size = backtrace(array.data(), 10);

    // print out all the frames to stderr
    logger(rx::log::level::k_error, "Stacktrace: ");
    char** data = backtrace_symbols(array.data(), size);

    for(int i = 0; i < size; i++) {
        rx::string str(data[i]);

        if(str.find_last_of("(") != rx::string::k_npos && str.find_last_of(")") != rx::string::k_npos) {
            try {
                rx::string path = str.substring(0, str.find_last_of("("));
                rx::string symbol = str.substring(str.find_last_of("(") + 1, str.find_last_of("+") - str.find_last_of("(") - 1);
                rx::string address = str.substring(str.find_last_of("+"), str.find_last_of(")") - str.find_last_of("+"));

                if(symbol.size() > 0) {
                    char* name = abi::__cxa_demangle(symbol.data(), nullptr, nullptr, nullptr);
                    str = rx::string::format("%s(%s%s)", path, name, address);

                    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
                    free(name);
                }
            }
            catch(const std::exception& e) {
                logger(rx::log::level::k_warning, "Demangle failed: %s", e.what());
            }
        }

        if(str[str.size() - 1] == '\n') {
            str = str.substring(0, str.size() - 1);
        }

        logger(rx::log::level::k_error, "\t%s", str);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
    free(data);
}

rx::string get_last_linux_error() {
    char* errstr = strerror(errno);
    if(errstr == nullptr) {
        return rx::string("unkown error");
    }
    errno = 0;

    return rx::string(errstr);
}
