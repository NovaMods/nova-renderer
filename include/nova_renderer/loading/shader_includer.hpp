#pragma once

#include "nova_renderer/util/platform.hpp"
#ifdef NOVA_WINDOWS
#include <comdef.h>
#endif

#include <dxc/dxcapi.h>

#include "rx/core/concurrency/mutex.h"
#include "rx/core/map.h"
#include "rx/core/string.h"

namespace rx {
    namespace memory {
        struct allocator;
    }
} // namespace rx

namespace nova::renderer {
    /*!
     * \brief Include handler to let Nova shaders include other files
     */
    class NovaDxcIncludeHandler final : public IDxcIncludeHandler {
    public:
        explicit NovaDxcIncludeHandler(rx::memory::allocator& allocator, IDxcLibrary& library);

        virtual ~NovaDxcIncludeHandler() = default;

        HRESULT QueryInterface(const REFIID class_id, void** output_object) override;

        ULONG AddRef() override;

        ULONG Release() override;

        HRESULT LoadSource(LPCWSTR wide_filename, IDxcBlob** included_source) override;

    private:
        rx::memory::allocator& allocator;

        IDxcLibrary& library;

        rx::concurrency::mutex mtx;

        rx::map<rx::string, rx::string> builtin_files;

        ULONG num_refs = 0;
    };
} // namespace nova::renderer
