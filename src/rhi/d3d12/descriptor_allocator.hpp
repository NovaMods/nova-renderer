#pragma once

#include <d3d12.h>
#include <stdint.h>
#include <wrl/client.h>

#include "rx/core/vector.h"

// Fix WinAPI cause Rex broke it
#define interface struct

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
                            UINT descriptor_size_in,
                            rx::memory::allocator& allocator);

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE get_next_free_descriptor();

        [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE reserve_space_for_descriptor_table(uint32_t num_descriptors);

        void release_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        [[nodiscard]] UINT get_descriptor_size() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
        UINT descriptor_size;

        INT next_unallocated_descriptor = 0;
        rx::vector<D3D12_CPU_DESCRIPTOR_HANDLE> available_descriptors;
    };
} // namespace nova::renderer::rhi
