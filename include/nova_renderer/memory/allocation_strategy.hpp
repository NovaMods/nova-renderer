#pragma once

#include "nova_renderer/memory/bytes.hpp"

namespace bvestl::polyalloc {
    struct AllocationInfo;

    /*!
     * \brief Interface for an allocation strategy
     */
    class AllocationStrategy {
    public:
        virtual ~AllocationStrategy() = default;

        virtual bool allocate(Bytes size, AllocationInfo& allocation) = 0;

        virtual void free(const AllocationInfo& alloc) = 0;
    };
} // namespace bvestl::polyalloc
