#pragma once

#include <d3d12.h>
#include <stdint.h>
#include <wrl/client.h>

#include "rx/core/vector.h"

namespace rx {
    namespace memory {
        struct allocator;
    }
} // namespace rx

namespace nova::renderer::rhi {
    /*!
     * \brief A class to make allocating descriptors from a descriptor heap ezpz
     */
    class DescriptorAllocator {
    public:
        DescriptorAllocator(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_in,
                            uint32_t descriptor_size_in,
                            rx::memory::allocator& allocator);

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE get_next_free_descriptor();

        void release_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

    private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
        uint32_t descriptor_size;

        INT next_unallocated_descriptor = 0;
        rx::vector<D3D12_CPU_DESCRIPTOR_HANDLE> available_descriptors;
    };
} // namespace nova::renderer::rhi
