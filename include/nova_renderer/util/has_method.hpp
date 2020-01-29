#pragma once

namespace nova {
    // SFINAE bullshit I'm going to hate
    // Copied from https://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
    template <typename T>
    struct has_from_json {
        template <typename U, rx_size (U::*)(const rx::json&) const>
        struct SFINAE {};

        template <typename U>
        static char Test(SFINAE<U, U::from_json>*);

        template <typename U>
        static int Test(...);

        static const bool Has = sizeof(Test<T>({})) == sizeof(char);
    };
} // namespace nova
