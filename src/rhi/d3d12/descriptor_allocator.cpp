#include "descriptor_allocator.hpp"

#include "d3dx12.h"

namespace nova::renderer::rhi {
    DescriptorAllocator::DescriptorAllocator(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_in,
                                             const UINT descriptor_size_in,
                                             rx::memory::allocator& allocator)
        : heap{rx::utility::move(descriptor_heap_in)}, descriptor_size{descriptor_size_in}, available_descriptors{allocator} {}

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::get_next_free_descriptor() {
        if(available_descriptors.is_empty()) {
            const auto descriptor_idx = next_unallocated_descriptor;
            next_unallocated_descriptor++;

            return CD3DX12_CPU_DESCRIPTOR_HANDLE{heap->GetCPUDescriptorHandleForHeapStart(), descriptor_idx, descriptor_size};

        } else {
            const auto descriptor = available_descriptors.last();
            available_descriptors.pop_back();

            return descriptor;
        }
    }

    void DescriptorAllocator::release_descriptor(const D3D12_CPU_DESCRIPTOR_HANDLE descriptor) { available_descriptors.push_back(descriptor); }

    UINT DescriptorAllocator::get_descriptor_size() const {
        return descriptor_size;
    }
} // namespace nova::renderer::rhi
