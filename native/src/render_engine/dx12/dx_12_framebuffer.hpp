/*!
 * \author ddubois 
 * \date 02-Sep-18.
 */

#ifndef NOVA_RENDERER_DX_12_FRAMEBUFFER_HPP
#define NOVA_RENDERER_DX_12_FRAMEBUFFER_HPP

#include <d3d12.h>
#include <vector>

namespace nova {
    /*!
     * \brief A framebuffer class for DirectX, since it doesn't seemto have one
     */
    struct iframebuffer {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> color_attachments;
        D3D12_CPU_DESCRIPTOR_HANDLE* depth_stencil_descriptor = nullptr;
    };
}

#endif //NOVA_RENDERER_DX_12_FRAMEBUFFER_HPP
