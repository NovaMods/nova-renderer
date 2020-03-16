#include "nova_renderer/loading/shader_includer.hpp"

#include "rx/core/log.h"

namespace nova::renderer {
    RX_LOG("NovaDxcIncludeHandler", logger);

    NovaDxcIncludeHandler::NovaDxcIncludeHandler(rx::memory::allocator& allocator) : allocator{allocator} {}

    HRESULT NovaDxcIncludeHandler::QueryInterface(const IID& class_id, void** output_object) {
        if(!output_object) {
            return E_INVALIDARG;
        }

        *output_object = nullptr;

        if(class_id == IID_IUnknown || class_id == __uuidof(IDxcIncludeHandler)) {
            *output_object = reinterpret_cast<LPVOID>(this);
            AddRef();

            return NOERROR;
        }

        return E_NOINTERFACE;
    }

    ULONG NovaDxcIncludeHandler::AddRef() {
        InterlockedIncrement(&num_refs);

        return num_refs;
    }

    ULONG NovaDxcIncludeHandler::Release() {
        const auto ref_count = InterlockedDecrement(&num_refs);

        if(ref_count == 0) {
            // TODO: Figure out how to use a Rex allocator instead of forcing things to be on the heap
            delete this;
        }

        return ref_count;
    }

    HRESULT NovaDxcIncludeHandler::LoadSource(const LPCWSTR wide_filename, IDxcBlob** included_source) {
        const auto utf8_length = WideCharToMultiByte(CP_UTF8, 0, wide_filename, -1, nullptr, 0, nullptr, nullptr);

        rx::string filename {&allocator};
        filename.resize(utf8_length);

        WideCharToMultiByte(CP_UTF8, 0, wide_filename, -1, filename.data(), filename.size(), nullptr, nullptr);

        logger(rx::log::level::k_info, "Trying to include file (%s)", filename);

        return NOERROR;
    }
} // namespace nova::renderer
