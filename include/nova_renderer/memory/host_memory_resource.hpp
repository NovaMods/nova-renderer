#pragma once

#include <memory_resource>
#include <unordered_map>

#include "nova_renderer/memory/allocation_structs.hpp"

#if NOVA_DEBUG
#include "nova_renderer/util/logger.hpp"
#endif

namespace nova::mem {

    template <typename AllocationStrategy>
    class HostMemoryResource final : public std::pmr::memory_resource {
    public:
        HostMemoryResource(std::byte* mem, size_t size, AllocationStrategy* strategy);

        HostMemoryResource(const HostMemoryResource& other) = default;
        HostMemoryResource& operator=(const HostMemoryResource& other) = default;

        HostMemoryResource(HostMemoryResource&& old) noexcept = default;
        HostMemoryResource& operator=(HostMemoryResource&& old) noexcept = default;

        ~HostMemoryResource() noexcept override = default;

    private:
        std::byte* mem;
        size_t size;

        AllocationStrategy* strategy;

        std::unordered_map<void*, AllocationInfo> allocations;

        void* do_allocate(size_t bytes, size_t align) override;

        void do_deallocate(void* ptr, size_t bytes, size_t align) override;

        [[nodiscard]] bool do_is_equal(const memory_resource& that) const noexcept override;
    };

    template <typename AllocationStrategy>
    HostMemoryResource<AllocationStrategy>::HostMemoryResource(std::byte* mem, const size_t size, AllocationStrategy* strategy)
        : mem(mem), size(size), strategy(strategy) {}

    template <typename AllocationStrategy>
    void* HostMemoryResource<AllocationStrategy>::do_allocate(const size_t bytes, size_t /* align */) {
        AllocationInfo info;
        if(strategy->allocate(Bytes(bytes), info)) {
            auto* ptr = mem + info.offset;
            allocations.emplace(ptr, info);
            return ptr;

        } else {
            return nullptr;
        }
    }

    template <typename AllocationStrategy>
    void HostMemoryResource<AllocationStrategy>::do_deallocate(void* ptr, size_t /* bytes */, size_t /* align */) {
        if(const auto itr = allocations.find(ptr); itr != allocations.end()) {
            strategy->free(itr->second);
        }

#if NOVA_DEBUG
        else {
            NOVA_LOG(ERROR) << "Bad deallocation request for pointer " << ptr;
        }
#endif
    }

    template <typename AllocationStrategy>
    bool HostMemoryResource<AllocationStrategy>::do_is_equal(const memory_resource& that) const noexcept {
        return false; // TODO: Figure this method out
    }
} // namespace nova::mem
