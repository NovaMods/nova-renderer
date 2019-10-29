/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#include "dx12_command_list.hpp"
#include "../../../tests/src/general_test_setup.hpp"
#include "d3dx12.h"
#include "dx12_utils.hpp"

namespace nova::renderer::rhi {
    using namespace Microsoft::WRL;

    Dx12CommandList::Dx12CommandList(ComPtr<ID3D12GraphicsCommandList> cmds) : cmds(std::move(cmds)) {}

    void Dx12CommandList::resource_barriers([[maybe_unused]] PipelineStageFlags stages_before_barrier,
                                            [[maybe_unused]] PipelineStageFlags stages_after_barrier,
                                            const std::vector<ResourceBarrier>& barriers) {
        std::vector<D3D12_RESOURCE_BARRIER> dx12_barriers;
        dx12_barriers.reserve(barriers.size());

        for(const ResourceBarrier& barrier : barriers) {
            ID3D12Resource* resource_to_barrier;
            switch(barrier.resource_to_barrier->type) {
                case ResourceType::Buffer: {
                    auto* d3d12_buffer = static_cast<DX12Buffer*>(barrier.resource_to_barrier);
                    resource_to_barrier = d3d12_buffer->resource.Get();
                } break;

                case ResourceType::Image: {
                    auto* d3d12_image = static_cast<DX12Image*>(barrier.resource_to_barrier);
                    resource_to_barrier = d3d12_image->resource.Get();
                } break;
                default:;
            }

            if(barrier.access_after_barrier == ResourceAccessFlags::NoFlags) {
                const D3D12_RESOURCE_STATES initial_state = to_dx12_state(barrier.initial_state);
                const D3D12_RESOURCE_STATES final_state = to_dx12_state(barrier.final_state);
                // ReSharper disable once CppLocalVariableMightNotBeInitialized
                dx12_barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(resource_to_barrier, initial_state, final_state));

            } else {
                // ReSharper disable once CppLocalVariableMightNotBeInitialized
                const CD3DX12_RESOURCE_BARRIER sync = CD3DX12_RESOURCE_BARRIER::UAV(resource_to_barrier);
                dx12_barriers.push_back(sync);
            }
        }

        cmds->ResourceBarrier(static_cast<UINT>(dx12_barriers.size()), dx12_barriers.data());
    }

    void Dx12CommandList::copy_buffer(Buffer* destination_buffer,
                                      const uint64_t destination_offset,
                                      Buffer* source_buffer,
                                      const uint64_t source_offset,
                                      const uint64_t num_bytes) {
        auto* dst_buf = reinterpret_cast<DX12Buffer*>(destination_buffer);
        auto* src_buf = reinterpret_cast<DX12Buffer*>(source_buffer);

        cmds->CopyBufferRegion(dst_buf->resource.Get(), destination_offset, src_buf->resource.Get(), source_offset, num_bytes);
    }

    void Dx12CommandList::execute_command_lists(const std::vector<CommandList*>& lists) {
        // Apparently D3D12 can only execute bundles from another command list, meaning that the strategy I use to
        // record command buffers in Vulkan won't work here...
        //
        // OR WILL IT??????!?!?!?!??!?!?!??!?!?!??!!?
        //

        for(CommandList* list : lists) {
            auto* d3d12_list = dynamic_cast<Dx12CommandList*>(list);
            cmds->ExecuteBundle(d3d12_list->cmds.Get());
        }
    }

    void Dx12CommandList::begin_renderpass(Renderpass* /* renderpass */, Framebuffer* framebuffer) {
        auto* d3d12_framebuffer = reinterpret_cast<DX12Framebuffer*>(framebuffer);

        D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil = nullptr;
        if(d3d12_framebuffer->dsv_descriptor) {
            depth_stencil = &*d3d12_framebuffer->dsv_descriptor;
        }

        cmds->OMSetRenderTargets(static_cast<UINT>(d3d12_framebuffer->rtv_descriptors.size()),
                                 d3d12_framebuffer->rtv_descriptors.data(),
                                 false,
                                 depth_stencil);
    }

    void Dx12CommandList::end_renderpass() {}

    void Dx12CommandList::bind_pipeline(const Pipeline* pipeline) {
        const auto* dx_pipeline = static_cast<const DX12Pipeline*>(pipeline);
        cmds->SetPipelineState(dx_pipeline->pso.Get());
    }

    void Dx12CommandList::bind_descriptor_sets(const std::vector<DescriptorSet*>& descriptor_sets,
                                               const PipelineInterface* pipeline_interface) {
        const auto* dx_interface = static_cast<const DX12PipelineInterface*>(pipeline_interface);

        // Probably how this should work?
        for(uint32_t i = 0; i < descriptor_sets.size(); i++) {
            const auto* dx_set = static_cast<const DX12DescriptorSet*>(descriptor_sets.at(i));
            cmds->SetDescriptorHeaps(1, dx_set->heap.GetAddressOf());
            cmds->SetGraphicsRootDescriptorTable(i, dx_set->heap->GetGPUDescriptorHandleForHeapStart());
        }
    }

    void Dx12CommandList::bind_vertex_buffers(const std::vector<Buffer*>& buffers) {
        std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
        views.reserve(buffers.size());

        for(const Buffer* buffer : buffers) {
            const auto* dx_buffer = static_cast<const DX12Buffer*>(buffer);

            views.emplace_back(
                D3D12_VERTEX_BUFFER_VIEW{dx_buffer->resource->GetGPUVirtualAddress(), static_cast<UINT>(dx_buffer->size.b_count()), sizeof(FullVertex)});
        }

        cmds->IASetVertexBuffers(0, static_cast<UINT>(views.size()), views.data());
    }

    void Dx12CommandList::bind_index_buffer(const Buffer* buffer) {
        const auto* dx12_buffer = static_cast<const DX12Buffer*>(buffer);

        D3D12_INDEX_BUFFER_VIEW view = {dx12_buffer->resource->GetGPUVirtualAddress(),
                                        static_cast<UINT>(dx12_buffer->size.b_count()),
                                        DXGI_FORMAT_R32_UINT};

        cmds->IASetIndexBuffer(&view);
    }

    void Dx12CommandList::draw_indexed_mesh(const uint32_t num_indices, const uint32_t num_instances) {
        cmds->DrawInstanced(num_indices, num_instances, 0, 0);
    }
} // namespace nova::renderer::rhi
