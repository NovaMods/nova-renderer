#pragma once

#include <d3d12.h>
#include <stdint.h>
#include <wrl/client.h>

#include "d3d12_resource_binder.hpp"
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

        /*!
         * \brief Gets a handle to one or more descriptors from the descriptor pool
         *
         * \param array_size The size of the array that this descriptor represents. Must be greater than 0!
         */
        [[nodiscard]] D3D12Descriptor get_descriptor(uint32_t array_size = 1);

        void release_descriptor(const D3D12Descriptor& descriptor);

        [[nodiscard]] UINT get_descriptor_size() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
        UINT descriptor_size;

        INT next_unallocated_descriptor = 0;
        rx::vector<D3D12Descriptor> available_descriptors;
    };
} // namespace nova::renderer::rhi
