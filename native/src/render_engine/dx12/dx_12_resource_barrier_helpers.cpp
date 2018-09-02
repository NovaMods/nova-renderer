/*!
 * \author ddubois 
 * \date 02-Sep-18.
 */

#include "dx_12_resource_barrier_helpers.hpp"
#include "../../util/logger.hpp"

#if SUPPORT_DX12

namespace nova {
    D3D12_RESOURCE_STATES to_dx12_resource_state(const resource_layout layout) {
        switch(layout) {
            case resource_layout::RENDER_TARGET:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case resource_layout::PRESENT:
                return D3D12_RESOURCE_STATE_PRESENT;

            default:
                NOVA_LOG(ERROR) << "Unrecognized resource state " << layout.to_string();
                return D3D12_RESOURCE_STATE_COMMON;
        }
    }
}

#endif
