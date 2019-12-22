#pragma once

#include "nova_renderer/memory/allocation_strategy.hpp"
#include "nova_renderer/memory/bytes.hpp"

namespace bvestl::polyalloc {
    struct AllocationInfo;

    /*!
     * \brief Allocates memory linearly from a single pool. Memory must be freed all at once, there's no support for freeing individual
     * allocations
     */
    class BumpPointAllocationStrategy final : public AllocationStrategy {
    public:
        struct Allocation {
            Bytes offset{0};
            Bytes size{0};
        };

        explicit BumpPointAllocationStrategy(Bytes size_in, Bytes alignment_in = Bytes(0));

        bool allocate(Bytes size, AllocationInfo& allocation) override;

        void free(const AllocationInfo&) override;

    private:
        Bytes memory_size;
        Bytes alignment;

        Bytes allocated_bytes{0};
    };
} // namespace bvestl::polyalloc
