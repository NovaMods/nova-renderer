/*!
 * \brief Helper functions for converting resource barrier data from platform-independent to DX12
 *
 * \author ddubois 
 * \date 02-Sep-18.
 */

#ifndef NOVA_RENDERER_DX_12_RESOURCE_BARRIER_HELPERS_HPP
#define NOVA_RENDERER_DX_12_RESOURCE_BARRIER_HELPERS_HPP

#include "../../platform.hpp"

#if SUPPORT_DX12

#include <d3d12.h>
#include "../resource_barrier.hpp"

namespace nova {
    D3D12_RESOURCE_STATES to_dx12_resource_state(const resource_layout layout);
}

#endif

#endif //NOVA_RENDERER_DX_12_RESOURCE_BARRIER_HELPERS_HPP
