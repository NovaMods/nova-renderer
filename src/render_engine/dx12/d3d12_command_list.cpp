/*!
 * \author ddubois 
 * \date 30-Mar-19.
 */

#include "d3d12_command_list.hpp"
#include <utility>

namespace nova::renderer {
    d3d12_command_list::d3d12_command_list(ComPtr<ID3D12CommandList> cmds) : cmds(std::move(cmds)) {}

    d3d12_graphics_command_list::d3d12_graphics_command_list(ComPtr<ID3D12GraphicsCommandList> cmds) : cmds(std::move(cmds)) {}
}
