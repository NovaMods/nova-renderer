//
// Created by jannis on 29.08.18.
//

#ifndef NOVA_RENDERER_MACROS_HPP
#define NOVA_RENDERER_MACROS_HPP

#include <exception>
#include <string>

/*!
 * \brief Makes an simple named error class
 */
#define NOVA_EXCEPTION(_namespace, name) \
    namespace _namespace { \
        class name : public std::exception { \
        private: \
            std::string _what; \
        public: \
            explicit name(std::string what) : _what(std::move(what)) {} \
            const char *what() const noexcept override { return _what.c_str(); } \
        }; \
    }

#endif //NOVA_RENDERER_MACROS_HPP
