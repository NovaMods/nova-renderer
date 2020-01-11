#include "dx12_command_list.hpp"

#include "../../../tests/src/general_test_setup.hpp"
#include "d3dx12.h"
#include "dx12_utils.hpp"

using namespace nova::mem;

namespace nova::renderer::rhi {
    using namespace Microsoft::WRL;

    Dx12CommandList::Dx12CommandList(ComPtr<ID3D12GraphicsCommandList> cmds) : cmds(std::move(cmds)) {}

    void Dx12CommandList::resource_barriers(PipelineStage /* stages_before_barrier */,
                                            PipelineStage /* stages_after_barrier */,
                                            const std::pmr::vector<ResourceBarrier>& barriers) {
        // D3D12 barriers don't use all the information in our `barriers` struct - specifically, they don't care much about image layouts,
        // nor about the pipeline stage flags. Thus, this method doesn't do anything with that data

        std::pmr::vector<D3D12_RESOURCE_BARRIER> dx12_barriers;
        dx12_barriers.reserve(barriers.size());

        for(const ResourceBarrier& barrier : barriers) {
            ID3D12Resource* resource_to_barrier = nullptr;
            switch(barrier.resource_to_barrier->type) {
                case ResourceType::Buffer: {
                    auto* d3d12_buffer = static_cast<DX12Buffer*>(barrier.resource_to_barrier);
                    resource_to_barrier = d3d12_buffer->resource.Get();
                } break;

                case ResourceType::Image: {
                    auto* d3d12_image = static_cast<DX12Image*>(barrier.resource_to_barrier);
                    resource_to_barrier = d3d12_image->resource.Get();
                } break;
            }

            const D3D12_RESOURCE_STATES initial_state = to_dx12_state(barrier.old_state);
            const D3D12_RESOURCE_STATES final_state = to_dx12_state(barrier.new_state);

            D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            // Do some guesswork. Frontend/backend refactor will add state tracking that makes this code obsolete

            if(barrier.new_state == ResourceState::CopySource || barrier.new_state == ResourceState::CopyDestination) {
                // If the resource is about to be involved in a copy operation, we need to end the a previous barrier
                flags |= D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;

            } else if(barrier.old_state == ResourceState::CopySource || barrier.old_state == ResourceState::CopyDestination) {
                // If the resource was just involved in a copy operation, we need to begin a barrier for a graphics queue to end
                flags |= D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            }

            dx12_barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(resource_to_barrier,
                                                                         initial_state,
                                                                         final_state,
                                                                         D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                                                                         flags));
        }

        cmds->ResourceBarrier(static_cast<UINT>(dx12_barriers.size()), dx12_barriers.data());
    }

    void Dx12CommandList::copy_buffer(Buffer* destination_buffer,
                                      const Bytes destination_offset,
                                      Buffer* source_buffer,
                                      const Bytes source_offset,
                                      const Bytes num_bytes) {
        auto* dst_buf = reinterpret_cast<DX12Buffer*>(destination_buffer);
        auto* src_buf = reinterpret_cast<DX12Buffer*>(source_buffer);

        cmds->CopyBufferRegion(dst_buf->resource.Get(),
                               destination_offset.b_count(),
                               src_buf->resource.Get(),
                               source_offset.b_count(),
                               num_bytes.b_count());
    }

    void Dx12CommandList::execute_command_lists(const std::pmr::vector<CommandList*>& lists) {
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

        // TODO: Actually begin/end renderpasses if the hardware supports ID3D12GraphicsCommandList4

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

    void Dx12CommandList::bind_descriptor_sets(const std::pmr::vector<DescriptorSet*>& descriptor_sets,
                                               const PipelineInterface* pipeline_interface) {
        const auto* dx_interface = static_cast<const DX12PipelineInterface*>(pipeline_interface);

        // Probably how this should work?
        for(uint32_t i = 0; i < descriptor_sets.size(); i++) {
            const auto* dx_set = static_cast<const DX12DescriptorSet*>(descriptor_sets.at(i));
            cmds->SetDescriptorHeaps(1, dx_set->heap.GetAddressOf());
            cmds->SetGraphicsRootDescriptorTable(i, dx_set->heap->GetGPUDescriptorHandleForHeapStart());
        }
    }

    void Dx12CommandList::bind_vertex_buffers(const std::pmr::vector<Buffer*>& buffers) {
        std::pmr::vector<D3D12_VERTEX_BUFFER_VIEW> views;
        views.reserve(buffers.size());

        for(const Buffer* buffer : buffers) {
            const auto* dx_buffer = static_cast<const DX12Buffer*>(buffer);

            views.emplace_back(D3D12_VERTEX_BUFFER_VIEW{dx_buffer->resource->GetGPUVirtualAddress(),
                                                        static_cast<UINT>(dx_buffer->size.b_count()),
                                                        sizeof(FullVertex)});
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

    void Dx12CommandList::draw_indexed_mesh(const uint32_t num_indices, const uint32_t offset, const uint32_t num_instances) {
        cmds->DrawInstanced(num_indices, num_instances, 0, offset);
    }

    void Dx12CommandList::set_scissor_rect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
        D3D12_RECT scissor_rect = {static_cast<LONG>(x), static_cast<LONG>(y), static_cast<LONG>(x + width), static_cast<LONG>(y + height)};
        cmds->RSSetScissorRects(1, &scissor_rect);
    }

    void Dx12CommandList::upload_data_to_image(
        Image* image, const size_t width, const size_t height, const size_t bytes_per_pixel, Buffer* staging_buffer, const void* data) {
        const auto* dx_image = static_cast<const DX12Image*>(image);
        const auto* dx_buffer = static_cast<const DX12Buffer*>(staging_buffer);

        D3D12_SUBRESOURCE_DATA subresource;
        subresource.pData = data;
        subresource.RowPitch = width * bytes_per_pixel;
        subresource.SlicePitch = width * height * bytes_per_pixel;

        UpdateSubresources(cmds.Get(), dx_image->resource.Get(), dx_buffer->resource.Get(), 0, 0, 1, &subresource);
    }
} // namespace nova::renderer::rhi
