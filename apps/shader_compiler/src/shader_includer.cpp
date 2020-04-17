#include "shader_includer.hpp"

#include <utility>

namespace nova::renderer {
    constexpr const char* NOVA_SHADERS_DIRECTORY = "data/shaders/builtin";

    NovaDxcIncludeHandler::NovaDxcIncludeHandler(CComPtr<IDxcUtils> library_in) : library{std::move(library_in)} {
        library->CreateDefaultIncludeHandler(&default_includer);
    }

    HRESULT NovaDxcIncludeHandler::QueryInterface(REFIID class_id, void** output_object) {
        if(!output_object) {
            return E_INVALIDARG;
        }

        *output_object = nullptr;

        if(class_id == __uuidof(IDxcIncludeHandler)) {
            *output_object = reinterpret_cast<LPVOID>(this);
            AddRef();

            return 0;
        }

        return E_NOINTERFACE;
    }

#if _WIN32
    ULONG NovaDxcIncludeHandler::AddRef() {
        std::lock_guard l{mtx};
        num_refs++;

        return num_refs;
    }

    ULONG NovaDxcIncludeHandler::Release() {
        {
            std::lock_guard l{mtx};
            num_refs--;
        }
        const auto ref_count = num_refs;

        if(ref_count == 0) {
            // TODO: Figure out how to use a Rex allocator instead of forcing things to be on the heap
            delete this;
        }

        return ref_count;
    }
#endif

    HRESULT NovaDxcIncludeHandler::LoadSource(const LPCWSTR wide_filename, IDxcBlob** included_source) {
        std::string filename;
        filename.resize(1024);
        WideCharToMultiByte(CP_UTF8, 0, wide_filename, -1, filename.data(), filename.size(), nullptr, nullptr);

        if(filename.find("./nova/") == 0) {
            // The shader is trying to include a Nova shader file, load it from there
            const auto real_filename = NOVA_SHADERS_DIRECTORY + filename.substr(6);
            auto* real_wide_filename = new wchar_t[real_filename.length()];

            MultiByteToWideChar(CP_UTF8, 0, real_filename.c_str(), real_filename.length(), real_wide_filename, real_filename.length() * sizeof(wchar_t));

            printf("Trying to load Nova shader file '%S'", real_wide_filename);

            IDxcBlobEncoding* raw_file;
            const auto hr = library->LoadFile(real_wide_filename, nullptr, &raw_file);
            if(FAILED(hr)) {
                fprintf(stderr, "Could not open shader file %S\n", real_wide_filename);
                return hr;
            }

            *included_source = raw_file;

            delete[] real_wide_filename;

            return 0;

        } else {
            // Let the default includer handle things
            return default_includer->LoadSource(wide_filename, included_source);
        }
    }
} // namespace nova::renderer
