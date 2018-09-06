/*!
 * \author ddubois 
 * \date 02-Sep-18.
 */

#include "dx12_resource_barrier_helpers.hpp"
#include "../../util/logger.hpp"

#if SUPPORT_DX12

namespace nova {
    D3D12_RESOURCE_STATES to_dx12_resource_state(const image_layout layout) {
        switch(layout) {
            case image_layout::RENDER_TARGET:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case image_layout::PRESENT:
                return D3D12_RESOURCE_STATE_PRESENT;
        }
    }
}

#endif
