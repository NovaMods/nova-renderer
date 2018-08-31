/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include "../../src/nova_renderer.hpp"

#include "../../src/platform.hpp"
#if SUPPORT_DX12
#include "../../src/render_engine/dx12/dx_12_render_engine.hpp"
#define RenderEngineType nova::dx12_render_engine
#endif

void main(int num_args, const char** args) {
    nova::nova_renderer<RenderEngineType>::initialize();
}
