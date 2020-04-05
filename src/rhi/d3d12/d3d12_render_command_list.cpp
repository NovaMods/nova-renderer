#include "d3d12_render_command_list.hpp"

#include <minitrace.h>
#include <rx/core/log.h>

#include "nova_renderer/camera.hpp"

#include "d3d12_resource_binder.hpp"
#include "d3d12_utils.hpp"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

namespace nova::renderer::rhi {
    RX_LOG("D3D12RenderCommandList", logger);

    D3D12RenderCommandList::D3D12RenderCommandList(rx::memory::allocator& allocator,
                                                   Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmds,
                                                   D3D12RenderDevice& device_in)
        : internal_allocator{&allocator}, device{&device_in}, command_list{rx::utility::move(cmds)} {
        MTR_SCOPE("D3D12RenderCommandList", "D3D12RenderCommandList");

        command_list->QueryInterface(command_list_4.GetAddressOf());
    }

    void D3D12RenderCommandList::set_debug_name(const rx::string& name) {
        MTR_SCOPE("D3D12RenderCommandList", "set_debug_name");

        set_object_name(command_list.Get(), name);
    }

    void D3D12RenderCommandList::set_checkpoint(const rx::string& /* checkpoint_name */) {
        // D3D12 is lame with this
        // You can use `ID3D12GraphicsCommandList2::WriteBufferImmediate` to write a specific value to a specific GPU virtual address. I
        // definitely could make a buffer that holds the values for each command list, and use WriteBufferImmediate to write values to
        // that buffer, and implement some machinery in D3D12RenderDevice to read back that buffer value... but I want to stay in
        // D3D12RenderCommandList tonight, so I wrote this comment instead
    }

    void D3D12RenderCommandList::bind_resources(RhiResourceBinder& binder) {
        MTR_SCOPE("D3D12RenderCommandList", "bind_resources");

        auto& d3d12_binder = static_cast<D3D12ResourceBinder&>(binder);

        command_list->SetGraphicsRootSignature(d3d12_binder.get_root_signature());
    }

    void D3D12RenderCommandList::resource_barriers(PipelineStage /* stages_before_barrier */,
                                                   PipelineStage /* stages_after_barrier */,
                                                   const rx::vector<RhiResourceBarrier>& barriers) {

        rx::vector<D3D12_RESOURCE_BARRIER> d3d12_barriers{*internal_allocator};
        d3d12_barriers.reserve(barriers.size());

        barriers.each_fwd([&](const RhiResourceBarrier& barrier) {
            auto* resource = [&] {
                if(barrier.resource_to_barrier->type == ResourceType::Image) {
                    const auto* image = static_cast<D3D12Image*>(barrier.resource_to_barrier);
                    return image->resource.Get();

                } else {
                    auto* buffer = static_cast<D3D12Buffer*>(barrier.resource_to_barrier);
                    return buffer->resource.Get();
                }
            }();

            const auto state_before = to_d3d12_resource_state(barrier.old_state, barrier.access_before_barrier);
            const auto states_after = to_d3d12_resource_state(barrier.new_state, barrier.access_after_barrier);

            const auto d3d12_barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, state_before, states_after);

            d3d12_barriers.push_back(rx::utility::must_move(d3d12_barrier));
        });

        command_list->ResourceBarrier(static_cast<UINT>(d3d12_barriers.size()), d3d12_barriers.data());
    }

    void D3D12RenderCommandList::copy_buffer(RhiBuffer& destination_buffer,
                                             const mem::Bytes destination_offset,
                                             RhiBuffer& source_buffer,
                                             const mem::Bytes source_offset,
                                             const mem::Bytes num_bytes) {
        auto& d3d12_dst_buffer = static_cast<D3D12Buffer&>(destination_buffer);
        auto& d3d12_src_buffer = static_cast<D3D12Buffer&>(source_buffer);

        command_list->CopyBufferRegion(d3d12_dst_buffer.resource.Get(),
                                       destination_offset.b_count(),
                                       d3d12_src_buffer.resource.Get(),
                                       source_offset.b_count(),
                                       num_bytes.b_count());
    }

    void D3D12RenderCommandList::execute_command_lists(const rx::vector<RhiRenderCommandList*>& lists) {
        lists.each_fwd([&](const RhiRenderCommandList* list) {
            const auto* d3d12_list = static_cast<const D3D12RenderCommandList*>(list);
            command_list->ExecuteBundle(d3d12_list->get_d3d12_list());
        });
    }

    void D3D12RenderCommandList::set_camera(const Camera& camera) { command_list->SetGraphicsRoot32BitConstant(0, camera.index, 0); }

    void D3D12RenderCommandList::begin_renderpass(RhiRenderpass& renderpass, RhiFramebuffer& framebuffer) {
        MTR_SCOPE("D3D12RenderCommandList", "begin_renderpass");

        auto& d3d12_renderpass = static_cast<D3D12Renderpass&>(renderpass);

        current_renderpass = &d3d12_renderpass;

        if(command_list_4) {
            auto* depth_stencil_descriptor = d3d12_renderpass.depth_stencil_description ? &(*d3d12_renderpass.depth_stencil_description) :
                                                                                          nullptr;

            command_list_4->BeginRenderPass(static_cast<UINT>(d3d12_renderpass.render_target_descriptions.size()),
                                            d3d12_renderpass.render_target_descriptions.data(),
                                            depth_stencil_descriptor,
                                            d3d12_renderpass.flags);

        } else {
            auto& d3d12_framebuffer = static_cast<D3D12Framebuffer&>(framebuffer);

            auto* depth_stencil_descriptor = d3d12_framebuffer.depth_stencil_descriptor ? &(*d3d12_framebuffer.depth_stencil_descriptor) :
                                                                                          nullptr;

            command_list->OMSetRenderTargets(static_cast<UINT>(d3d12_framebuffer.render_target_descriptors.size()),
                                             d3d12_framebuffer.render_target_descriptors.data(),
                                             0,
                                             depth_stencil_descriptor);
        }
    }

    void D3D12RenderCommandList::end_renderpass() {
        MTR_SCOPE("D3D12RenderCommandList", "end_renderpass");

        if(command_list_4) {
            command_list_4->EndRenderPass();
        }
    }

    void D3D12RenderCommandList::set_material_index(const uint32_t index) { command_list->SetGraphicsRoot32BitConstant(0, index, 1); }

    void D3D12RenderCommandList::set_pipeline(const RhiPipeline& pipeline) {
        MTR_SCOPE("D3D12RenderCommandList", "set_pipeline");

        if(current_renderpass == nullptr) {
            logger->error("Can not set a pipeline");
            return;
        }

        const auto& d3d12_pipeline = static_cast<const D3D12Pipeline&>(pipeline);

        if(auto* pso = current_renderpass->cached_pipelines.find(d3d12_pipeline.name)) {
            command_list->SetPipelineState(pso->Get());

        } else {
            const auto new_pso = device->compile_pso(d3d12_pipeline, *current_renderpass);
            current_renderpass->cached_pipelines.insert(d3d12_pipeline.name, new_pso);
            command_list->SetPipelineState(new_pso.Get());
        }

        if(d3d12_pipeline.create_info.stencil_state) {
            command_list->OMSetStencilRef(d3d12_pipeline.create_info.stencil_state->reference_value);
        }
    }

    void D3D12RenderCommandList::bind_vertex_buffers(const rx::vector<RhiBuffer*>& buffers) {
        MTR_SCOPE("D3D12RenderCommandList", "bind_vertex_buffers");

        rx::vector<D3D12_VERTEX_BUFFER_VIEW> views{*internal_allocator};
        views.reserve(buffers.size());

        // TODO: RhiRenderCommandList needs a better abstraction for binding meshes, that allows for abstraction away of strides like this
        // Ideal solution: `RhiMeshBuffers` that has vertex buffers, index buffers, and information about how to bind them
        buffers.each_fwd([&](const RhiBuffer* buffer) {
            const auto* d3d12_buffer = static_cast<const D3D12Buffer*>(buffer);
            views.emplace_back(d3d12_buffer->resource->GetGPUVirtualAddress(), static_cast<UINT>(d3d12_buffer->size.b_count()), 0);
        });

        command_list->IASetVertexBuffers(0, static_cast<UINT>(views.size()), views.data());
    }

    void D3D12RenderCommandList::bind_index_buffer(const RhiBuffer& buffer, const IndexType index_size) {
        MTR_SCOPE("D3D12RenderCommandList", "bind_index_buffer");

        const auto& d3d12_buffer = static_cast<const D3D12Buffer&>(buffer);

        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = d3d12_buffer.resource->GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(d3d12_buffer.size.b_count());
        switch(index_size) {
            case IndexType::Uint16:
                view.Format = DXGI_FORMAT_R16_UINT;
                break;
            case IndexType::Uint32:
                view.Format = DXGI_FORMAT_R32_UINT;
                break;
        }

        command_list->IASetIndexBuffer(&view);
    }

    void D3D12RenderCommandList::draw_indexed_mesh(const uint32_t num_indices, const uint32_t offset, const uint32_t num_instances) {
        command_list->DrawIndexedInstanced(num_indices, num_instances, offset, 0, 0);
    }

    void D3D12RenderCommandList::set_scissor_rect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height) {
        D3D12_RECT scissor_rect;
        scissor_rect.top = y;
        scissor_rect.left = x;
        scissor_rect.bottom = y + height;
        scissor_rect.right = x + width;

        command_list->RSSetScissorRects(1, &scissor_rect);
    }

    ID3D12GraphicsCommandList* D3D12RenderCommandList::get_d3d12_list() const {
        return command_list.Get();
    }
} // namespace nova::renderer::rhi
