#include "descriptor_allocator.hpp"

#include "d3dx12.h"

namespace nova::renderer::rhi {
    DescriptorAllocator::DescriptorAllocator(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptor_heap_in,
                                             const UINT descriptor_size_in,
                                             rx::memory::allocator& allocator)
        : heap{rx::utility::move(descriptor_heap_in)}, descriptor_size{descriptor_size_in}, available_descriptors{allocator} {}

    D3D12Descriptor DescriptorAllocator::get_descriptor(const uint32_t array_size) {
        rx::optional<rx_size> best_match_idx = rx::nullopt;

        for(rx_size i = 0; i < available_descriptors.size(); i++) {
            if(available_descriptors[i].array_size == array_size) {
                // We found the exact descriptor we want!

                const auto descriptor = available_descriptors[i];
                available_descriptors.erase(i, i);
                return descriptor;

            } else if(available_descriptors[i].array_size > array_size) {
                if(!best_match_idx) {
                    best_match_idx = i;

                } else {
                    if(available_descriptors[i].array_size < available_descriptors[*best_match_idx].array_size) {
                        // If the current descriptor has less array space than the previous best-match descriptor, but it still large enough
                        // to fit the whole array, it's the new best-match descriptor
                        best_match_idx = i;
                    }
                }
            }
        }

        if(best_match_idx) {
            const auto idx = *best_match_idx;
            const auto descriptor = available_descriptors[idx];
            available_descriptors.erase(idx, idx);
            return descriptor;

        } else {
            const auto descriptor_idx = next_unallocated_descriptor;
            next_unallocated_descriptor += array_size;

            const auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE{heap->GetCPUDescriptorHandleForHeapStart(),
                                                                  descriptor_idx,
                                                                  descriptor_size};
            const auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE{heap->GetGPUDescriptorHandleForHeapStart(),
                                                                  descriptor_idx,
                                                                  descriptor_size};

            D3D12Descriptor descriptor{};
            descriptor.cpu_handle = cpu_handle;
            descriptor.gpu_handle = gpu_handle;
            descriptor.array_size = array_size;

            return descriptor;
        }
    }

    void DescriptorAllocator::release_descriptor(const D3D12Descriptor& descriptor) { available_descriptors.push_back(descriptor); }

    UINT DescriptorAllocator::get_descriptor_size() const { return descriptor_size; }
} // namespace nova::renderer::rhi
