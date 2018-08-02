//
// Created by jannis on 19.07.18.
//

#include "crash_handler.h"

#include <csignal>
#ifndef _WIN32
#include <execinfo.h>
#endif
#include <iostream>
#include <exception>
#include <typeinfo>

namespace nova {
    void crash_handler::install() {
        std::set_terminate(crash_handler::handle_terminate);
    }

    void crash_handler::handle_terminate() {
        const std::exception_ptr ptr = std::current_exception();
        if(!ptr) {
            std::cerr << "nova::crash_handler::handle_terminate() called without current exception!" << std::endl;
            print_stacktrace();
            std::cerr << "Exiting with code " << EXIT_FAILURE << std::endl;
            std::exit(EXIT_FAILURE);
        }

        const std::type_info *type = ptr.__cxa_exception_type();
        try {
            std::rethrow_exception(ptr);
        } catch (const std::exception &e) {
            std::cerr << "Unhandled exception: " << e.what() << std::endl;
        } catch (const std::string &msg) {
            std::cerr << "Unhandled exception: " << msg << std::endl;
        } catch (...) {
            const char *type_name;
            if(!type || !type->name()) {
                type_name = "<unknown>";
            } else {
                type_name = type->name();
            }
            std::cerr << "Unhandled exception of type " << type_name << std::endl;
        }
        print_stacktrace();

        std::cerr << "Exception handling complete, exiting with code " << EXIT_FAILURE << std::endl;
        std::exit(EXIT_FAILURE);
    }

    void crash_handler::print_stacktrace() {
#ifndef _WIN32
        std::cerr << "###########" << std::endl;
        std::cerr << "Stacktrace:" << std::endl;
        void *stack[100];
        int size = backtrace(stack, 100);
        char **function_names = backtrace_symbols(stack, size);
        for(int i = 0; i < size; i++) {
            std::cerr << "\t" << i << ": " << function_names[i] << std::endl;
        }
        std::cerr << "##########" << std::endl;
#endif
    }
}