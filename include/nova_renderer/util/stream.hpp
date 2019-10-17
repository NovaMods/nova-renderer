#pragma once

#include <functional>

//! Streams and stream accessories!
//!
//! A stream provides a functional interface to operate on everything in a collection. Streams allow you to

namespace ntl {
    /*! \brief Provides a functional interface for data processing */
    template <typename DataType>
    class Stream {
    public:
        template <typename OutputType>
        Stream<OutputType>& map(std::function<OutputType(const DataType&)> map_func);
    };
} // namespace ntl