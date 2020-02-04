#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

#include <cxxabi.h>
#include <execinfo.h>

void nova_backtrace() {
    std::array<void*, 50> array{};

    // get void*'s for all entries on the stack
    int size = backtrace(array.data(), array.size());

    // print out all the frames to stderr
    std::cerr << "Stacktrace: " << std::endl;
    char** data = backtrace_symbols(array.data(), size);

    for(int i = 0; i < size; i++) {
        std::string str(data[i]);

        if(str.find_last_of('(') != std::string::npos && str.find_last_of(')') != std::string::npos) {
            try {
                std::string path = str.substr(0, str.find_last_of('('));
                std::string symbol = str.substr(str.find_last_of('(') + 1, str.find_last_of('+') - str.find_last_of('(') - 1);
                std::string address = str.substr(str.find_last_of('+'), str.find_last_of(')') - str.find_last_of('+'));
                std::string load_address = str.substr(str.find_last_of('[') + 1, str.find_last_of(']') - str.find_last_of('['));

                if(!symbol.empty()) {
                    char* name = abi::__cxa_demangle(symbol.data(), nullptr, nullptr, nullptr);

                    str = std::string(path) + "(" + std::string(name ? name : symbol.c_str()) + std::string(address) + ") [" + load_address + "]";

                    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
                    free(name);
                }
            }
            catch(const std::exception& e) {
                std::cerr << "Demangle failed: " << e.what() << std::endl;
            }
        }

        if(str[str.size() - 1] == '\n') {
            str = str.substr(0, str.size() - 1);
        }

        std::cerr << "\t" << str << std::endl;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory)
    free(data);
}
