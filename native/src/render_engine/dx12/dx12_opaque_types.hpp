/*!
 * \brief Contains definitions for all the opaque types used in Nova
 *
 * \author ddubois 
 * \date 03-Sep-18.
 */

#ifndef NOVA_RENDERER_OPAQUE_TYPES_HPP
#define NOVA_RENDERER_OPAQUE_TYPES_HPP

#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace nova {
    struct iresource {
        ComPtr<ID3D12Resource> descriptor;
    };

    struct iframebuffer {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> color_attachments;
        D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil_descriptor = nullptr;
    };
}

#endif //NOVA_RENDERER_OPAQUE_TYPES_HPP
