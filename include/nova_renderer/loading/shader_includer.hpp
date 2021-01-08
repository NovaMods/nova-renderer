#pragma once

#include "nova_renderer/util/platform.hpp"
#ifdef NOVA_WINDOWS
#include <comdef.h>
#endif

#include <dxc/dxcapi.h>
#include <rx/core/concurrency/mutex.h>
#include <unordered_map>
#include <string>

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

        // In the Linux support library, these are implemented in IUnknown
        // However, I ran into an issue where the linker couldn't find definitions for these methods. I added the definitions to the
        // WinAdapter.h header in DXC, which seems to work for now
#if NOVA_WINDOWS
        ULONG AddRef() override;

        ULONG Release() override;
#endif

        HRESULT LoadSource(LPCWSTR wide_filename, IDxcBlob** included_source) override;

    private:
        rx::memory::allocator& allocator;

        IDxcLibrary& library;

        filesystem::FolderAccessorBase* folder_accessor;

        std::unordered_map<std::string, std::string> builtin_files;

#if NOVA_WINDOWS
        rx::concurrency::mutex mtx;

        ULONG num_refs = 0;
#endif
    };
} // namespace nova::renderer
