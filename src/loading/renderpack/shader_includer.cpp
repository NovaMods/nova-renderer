#include "nova_renderer/loading/shader_includer.hpp"

#include "rx/core/log.h"

namespace nova::renderer {
    RX_LOG("NovaDxcIncludeHandler", logger);

    constexpr const char* STANDARD_PIPELINE_LAYOUT_FILE_NAME = "./nova/standard_pipeline_layout.hlsl";

    NovaDxcIncludeHandler::NovaDxcIncludeHandler(rx::memory::allocator& allocator, IDxcLibrary& library)
        : allocator{allocator}, library{library}, builtin_files{&allocator} {
        const auto standard_pipeline_layout_hlsl = R"(
struct Camera {
    float4x4 view;
    float4x4 projection;
};

/*!
 * \brief All the push constants that are available to a shader that uses the standard pipeline layout
 */
[[vk::push_constant]]
struct StandardPushConstants {
    /*!
     * \brief Index of the camera that will render this draw
     */
    uint camera_index;

    /*!
     * \brief Index of the material data for the current draw
     */
    uint material_index;
} constants;

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(0, 0)]]
StructuredBuffer<Camera> cameras : register (t0);

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(1, 0)]]
StructuredBuffer<MaterialData> material_buffer : register (t1);

/*!
 * \brief Point sampler you can use to sample any texture
 */
[[vk::binding(2, 0)]]
SamplerState point_sampler : register(s0);

/*!
 * \brief Bilinear sampler you can use to sample any texture
 */
[[vk::binding(3, 0)]]
SamplerState bilinear_filter : register(s1);

/*!
 * \brief Trilinear sampler you can use to sample any texture
 */
[[vk::binding(4, 0)]]
SamplerState trilinear_filter : register(s3);

/*!
 * \brief Array of all the textures that are available for a shader to sample from
 */
[[vk::binding(5, 0)]]
Texture2D textures[] : register(t3);
        )";

        builtin_files.insert(STANDARD_PIPELINE_LAYOUT_FILE_NAME, standard_pipeline_layout_hlsl);
    }

    HRESULT NovaDxcIncludeHandler::QueryInterface(const REFIID class_id, void** output_object) {
        if(!output_object) {
            return E_INVALIDARG;
        }

        *output_object = nullptr;

        if(class_id == IID_IUnknown || class_id == __uuidof(IDxcIncludeHandler)) {
            *output_object = reinterpret_cast<LPVOID>(this);
            AddRef();

            return 0;
        }

        return E_NOINTERFACE;
    }

    ULONG NovaDxcIncludeHandler::AddRef() {
        rx::concurrency::scope_lock l{mtx};
        num_refs++;

        return num_refs;
    }

    ULONG NovaDxcIncludeHandler::Release() {
        rx::concurrency::scope_lock l{mtx};
        num_refs--;

        if(num_refs == 0) {
            // TODO: Figure out how to use a Rex allocator instead of forcing things to be on the heap
            delete this;
        }

        return num_refs;
    }

    HRESULT NovaDxcIncludeHandler::LoadSource(const LPCWSTR wide_filename, IDxcBlob** included_source) {
        const rx::wide_string wide_filename_str{&allocator, reinterpret_cast<const rx_u16*>(wide_filename)};
        const auto filename = wide_filename_str.to_utf8();

        logger(rx::log::level::k_verbose, "Trying to include file (%s)", filename);

        if(const auto* file = builtin_files.find(filename)) {
            IDxcBlobEncoding* encoding;
            library.CreateBlobWithEncodingFromPinned(file->data(), static_cast<uint32_t>(file->size()), CP_UTF8, &encoding);
            *included_source = encoding;

            logger(rx::log::level::k_verbose, "Included file");

            return 0;
        }

        return ERROR_FILE_NOT_FOUND;
    }
} // namespace nova::renderer
