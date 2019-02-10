//
// Created by jannis on 22.12.18.
//

#include <execinfo.h>
#include <cxxabi.h>
#include <cstring>
#include <fmt/format.h>
#include "logger.hpp"
#include "linux_utils.hpp"

void nova_backtrace() {
    std::array<void*, 50> array;

    // get void*'s for all entries on the stack
    int size = backtrace(array.data(), 10);

    // print out all the frames to stderr
    NOVA_LOG(ERROR) << "Stacktrace: ";
    char **data = backtrace_symbols(array.data(), size);

    for(int i = 0; i < size; i++) {
        std::string str(data[i]);

        if(str.find_last_of('(') != std::string::npos && str.find_last_of(')') != std::string::npos) {
            try {
                std::string path = str.substr(0, str.find_last_of('('));
                std::string symbol = str.substr(str.find_last_of('(') + 1,
                                                str.find_last_of('+') - str.find_last_of('(') - 1);
                std::string address = str.substr(str.find_last_of('+'), str.find_last_of(')') - str.find_last_of('+'));

                if (symbol.length() > 0) {
                    char *name = abi::__cxa_demangle(symbol.c_str(), nullptr, nullptr, nullptr);
                    str = format(fmt("{:s}({:s}{:s})"), path, name, address);
                    
                    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
                    free(name);
                }
            } catch (const std::exception &e) {
                NOVA_LOG(WARN) << "Demangle failed: " << e.what() << std::endl;
            }
        }

        if(str.at(str.length() - 1) == '\n') {
            str = str.substr(0, str.length() - 1);
        }

        NOVA_LOG(ERROR) << "\t" << str;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    free(data);
}

std::string get_last_linux_error() {
    char *errstr = strerror(errno);
    if(!errstr) {
        return std::string("unkown error");
    }
    errno = 0;

    return std::string(errstr);
}
