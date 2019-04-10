#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace nova::ttl {
    /*!
     * \brief An atomic counter that can be waited on
     *
     * Brought to you by std::condition_variable, std::mutex, and uint32_t
     *
     * Internal value starts at 0
     */
    class condition_counter {
    public:
        explicit condition_counter(uint32_t initial_value = 0);

        /*!
         * \brief Atomically adds `num` to this boi
         */
        void add(uint32_t num);

        /*!
         * \brief Atomically subtracts `num` from this boi. If the result would have been negative, the result is 0
         */
        void sub(uint32_t num);

        /*!
         * \brief Waits for the value of this condition_counter to become equal to `val`
         */
        void wait_for_value(uint32_t val);

    private:
        std::mutex mut;
        std::condition_variable cv;

        uint32_t counter = 0;
        uint32_t wait_val = 0;
    };
} // namespace nova::ttl
