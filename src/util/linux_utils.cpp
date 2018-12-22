//
// Created by jannis on 22.12.18.
//

#include <execinfo.h>
#include <cxxabi.h>
#include "logger.hpp"
#include "linux_utils.hpp"

void nova_backtrace() {
    void *array[50];
    int size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    NOVA_LOG(ERROR) << "Stacktrace: ";
    char **data = backtrace_symbols(array, size);

    for(int i = 0; i < size; i++) {
        std::string str(data[i]);

        if(str.find_last_of('(') != std::string::npos && str.find_last_of(')') != std::string::npos) {
            std::string path = str.substr(0, str.find_last_of('('));
            std::string symbol = str.substr(str.find_last_of('(') + 1, str.find_last_of('+') - str.find_last_of('(') - 1);
            std::string address = str.substr(str.find_last_of('+'), str.find_last_of(')') - str.find_last_of('+'));

            if(symbol.length() > 0) {
                char *name = abi::__cxa_demangle(symbol.c_str(), nullptr, nullptr, nullptr);
                symbol = std::string(name);
                free(name);

                str = "";
                str.append(path).append("(").append(symbol).append(address).append(")");
            }
        }

        if(str.at(str.length() - 1) == '\n') {
            str = str.substr(0, str.length() - 1);
        }

        NOVA_LOG(ERROR) << "\t" << str;
    }
    free(data);
}