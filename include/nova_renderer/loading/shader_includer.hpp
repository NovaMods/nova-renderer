#pragma once

#include "nova_renderer/util/platform.hpp"
#ifdef NOVA_WINDOWS
#include <comdef.h>
#endif

#include <dxc/dxcapi.h>
#include <rx/core/concurrency/mutex.h>
#include <rx/core/map.h>
#include <rx/core/string.h>

namespace nova {
    namespace filesystem {
        class FolderAccessorBase;
    }
} // namespace nova

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
        explicit NovaDxcIncludeHandler(rx::memory::allocator& allocator,
                                       IDxcLibrary& library,
                                       filesystem::FolderAccessorBase* folder_accessor);

        virtual ~NovaDxcIncludeHandler() = default;

        HRESULT QueryInterface(const REFIID class_id, void** output_object) override;

#if NOVA_WINDOWS
        ULONG AddRef() override;

        ULONG Release() override;
#endif

        HRESULT LoadSource(LPCWSTR wide_filename, IDxcBlob** included_source) override;

    private:
        rx::memory::allocator& allocator;

        IDxcLibrary& library;

        filesystem::FolderAccessorBase* folder_accessor;

        rx::map<rx::string, rx::string> builtin_files;

#if NOVA_WINDOWS
        rx::concurrency::mutex mtx;

        ULONG num_refs = 0;
#endif
    };
} // namespace nova::renderer
