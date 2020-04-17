#pragma once

#include <mutex>

#include <atlbase.h>
#include <comdef.h>
#include <dxc/dxcapi.h>

namespace nova::renderer {
    /*!
     * \brief Include handler to let Nova shaders include other files
     */
    class NovaDxcIncludeHandler final : public IDxcIncludeHandler {
    public:
        explicit NovaDxcIncludeHandler(CComPtr<IDxcUtils> library_in);

        virtual ~NovaDxcIncludeHandler() = default;

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID class_id, void** output_object) override;

        // In the Linux support library, these are implemented in IUnknown
        // However, I ran into an issue where the linker couldn't find definitions for these methods. I added the definitions to the
        // WinAdapter.h header in DXC, which seems to work for now
#if _WIN32
        ULONG STDMETHODCALLTYPE AddRef() override;

        ULONG STDMETHODCALLTYPE Release() override;
#endif

        HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR wide_filename, IDxcBlob** included_source) override;

    private:
        CComPtr<IDxcUtils> library;

        CComPtr<IDxcIncludeHandler> default_includer;

#if _WIN32
        std::mutex mtx;

        ULONG num_refs = 0;
#endif
    };
} // namespace nova::renderer
