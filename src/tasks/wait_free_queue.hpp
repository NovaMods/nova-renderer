/**
 * FiberTaskingLib - A tasking library that uses fibers for efficient task switching
 *
 * This library was created as a proof of concept of the ideas presented by
 * Christian Gyrling in his 2015 GDC Talk 'Parallelizing the Naughty Dog Engine Using Fibers'
 *
 * http://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
 *
 * FiberTaskingLib is the legal property of Adrian Astley
 * Copyright Adrian Astley 2015 - 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * This is an implementation of 'Correct and Efficient Work-Stealing for Weak Memory Models' by Le et. al [2013]
 *
 * https://hal.inria.fr/hal-00802885
 */

#pragma once

#include <atomic>
#include <cassert>
#include <memory>
#include <vector>

#define CACHE_LINE_SIZE 64

namespace nova {

    template <typename T>
    class wait_free_queue {
    public:
        wait_free_queue()
            : m_top(1),    // m_top and m_bottom must start at 1
              m_bottom(1), // Otherwise, the first Pop on an empty queue will underflow m_bottom
              m_array(new circular_array(32)) {
        }
        ~wait_free_queue() {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete m_array.load(std::memory_order_relaxed);
        }

    private:
        class circular_array {
        public:
            explicit circular_array(std::size_t n) : items(n) {
                assert(n != 0 && !(n & (n - 1)) && "n must be a power of 2");
            }

        private:
            std::vector<T> items;
            std::unique_ptr<circular_array> previous;

        public:
            [[nodiscard]] std::size_t size() const {
                return items.size();
            }

            T get(std::size_t index) {
                return items[index & (size() - 1)];
            }

            void put(std::size_t index, T x) {
                items[index & (size() - 1)] = x;
            }

            // Growing the array returns a new circular_array object and keeps a
            // linked list of all previous arrays. This is done because other threads
            // could still be accessing elements from the smaller arrays.
            circular_array *grow(std::size_t top, std::size_t bottom) {
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                auto *new_array = new circular_array(size() * 2);
                new_array->previous.reset(this);
                for(std::size_t i = top; i != bottom; i++) {
                    new_array->put(i, get(i));
                }
                return new_array;
            }
        };

        alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> m_top;
        alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> m_bottom;
        alignas(CACHE_LINE_SIZE) std::atomic<circular_array *> m_array;

    public:
        void push(T value) {
            uint64_t b = m_bottom.load(std::memory_order_relaxed);
            uint64_t t = m_top.load(std::memory_order_acquire);
            circular_array *array = m_array.load(std::memory_order_relaxed);

            if(b - t > array->size() - 1) {
                /* Full queue. */
                array = array->grow(t, b);
                m_array.store(array, std::memory_order_release);
            }
            array->put(b, value);

#if defined(FTL_STRONG_MEMORY_MODEL)
            std::atomic_signal_fence(std::memory_order_release);
#else
            std::atomic_thread_fence(std::memory_order_release);
#endif

            m_bottom.store(b + 1, std::memory_order_relaxed);
        }

        bool pop(T *value) {
            uint64_t b = m_bottom.load(std::memory_order_relaxed) - 1;
            circular_array *array = m_array.load(std::memory_order_relaxed);
            m_bottom.store(b, std::memory_order_relaxed);

            std::atomic_thread_fence(std::memory_order_seq_cst);

            uint64_t t = m_top.load(std::memory_order_relaxed);
            bool result = true;
            if(t <= b) {
                /* Non-empty queue. */
                *value = array->get(b);
                if(t == b) {
                    /* Single last element in queue. */
                    if(!std::atomic_compare_exchange_strong_explicit(&m_top,
                                                                     &t,
                                                                     t + 1,
                                                                     std::memory_order_seq_cst,
                                                                     std::memory_order_relaxed)) {
                        /* Failed race. */
                        result = false;
                    }
                    m_bottom.store(b + 1, std::memory_order_relaxed);
                }
            }
            else {
                /* Empty queue. */
                result = false;
                m_bottom.store(b + 1, std::memory_order_relaxed);
            }

            return result;
        }

        bool steal(T *value) {
            uint64_t t = m_top.load(std::memory_order_acquire);

#if defined(FTL_STRONG_MEMORY_MODEL)
            std::atomic_signal_fence(std::memory_order_seq_cst);
#else
            std::atomic_thread_fence(std::memory_order_seq_cst);
#endif

            uint64_t b = m_bottom.load(std::memory_order_acquire);
            if(t < b) {
                /* Non-empty queue. */
                circular_array *array = m_array.load(std::memory_order_consume);
                *value = array->get(t);

                return std::atomic_compare_exchange_strong_explicit(&m_top,
                                                                    &t,
                                                                    t + 1,
                                                                    std::memory_order_seq_cst,
                                                                    std::memory_order_relaxed);
            }

            return false;
        }

        size_t size() {
            return m_array.load(std::memory_order_relaxed)->size();
        }
    };

} // namespace nova
