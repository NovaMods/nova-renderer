#include <atlbase.h>
#include <d3d12shader.h>
#include <dxc/dxcapi.h>
#include <stdio.h>

#include "shader_includer.hpp"

// Mostly from https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll

constexpr int E_INVALID_ARGS = 1;
constexpr int E_INVALID_SHADER_STAGE = 2;
constexpr int E_COMPILATION_FAILED = 3;
constexpr int E_CANT_SAVE = 4;

static LPCWSTR to_wide_string(const char* string) {
    const auto string_length = strlen(string);
    const auto wide_string_length = string_length * 4;

    auto* wide_string = new wchar_t[wide_string_length];
    memset(wide_string, 0, wide_string_length * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, string, string_length, wide_string, wide_string_length);

    return wide_string;
}

static LPCWSTR get_shader_target(const char* filename) {
    // TODO: Update this as Nova supports more shader stages

    if(strstr(filename, ".vertex.hlsl")) {
        return L"vs_6_0";

    } else if(strstr(filename, ".pixel.hlsl")) {
        return L"ps_6_0";

    } else {
        fprintf(stderr, "Unsupported shader type for file %s\n", filename);
        return nullptr;
    }
}

int main(const int argc, char** argv) {
    const char* input_file_name_raw;
    const char* output_file_name;
    if(argc == 3) {
        input_file_name_raw = argv[1];
        output_file_name = argv[2];

    } else {
        fprintf(stderr, "Usage: shader-compiler.exe <HLSL file> <SPIR-V file>\n");

#ifdef NDEBUG
        return E_INVALID_ARG;
#else
        // Enable debugging with default parameters on debug builds
        input_file_name_raw = R"(E:\Documents\Nova\best-friend\external\nova-renderer\data\shaders\backbuffer_output.vertex.hlsl)";
        output_file_name = R"(E:\Documents\Nova\best-friend\data\shaders\builtin\backbuffer_output.vertex.spirv)";
#endif
    }

#ifndef NDEBUG
    printf("Compiling '%s' to '%s'\n", input_file_name_raw, output_file_name);
#endif

    const auto input_file_name = to_wide_string(input_file_name_raw);

    const auto shader_target = get_shader_target(input_file_name_raw);
    if(shader_target == nullptr) {
        fprintf(stderr, "Could not compile shader %s\n", input_file_name_raw);
        return E_INVALID_SHADER_STAGE;
    }

    CComPtr<IDxcUtils> utils;
    CComPtr<IDxcCompiler3> compiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    auto* include_handler = new nova::renderer::NovaDxcIncludeHandler(utils);

    LPCWSTR args[] = {L"-spirv", L"-fspv-target-env=vulkan1.1", L"-fspv-reflect", L"-E", L"main", L"-T", shader_target};

    CComPtr<IDxcBlobEncoding> source;
    const auto hr = utils->LoadFile(input_file_name, nullptr, &source);
    if(FAILED(hr)) {
        fprintf(stderr, "Could not open file %S\n", input_file_name);
        return E_INVALID_ARGS;
    }

    DxcBuffer source_buffer{};
    source_buffer.Ptr = source->GetBufferPointer();
    source_buffer.Size = source->GetBufferSize();
    source_buffer.Encoding = DXC_CP_ACP;

    CComPtr<IDxcResult> results;
    compiler->Compile(&source_buffer, args, _countof(args), include_handler, IID_PPV_ARGS(&results));

    // Check, for errors, exiting if any exist
    CComPtr<IDxcBlobUtf8> errors;
    results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if(errors && errors->GetStringLength() > 0) {
        fwprintf(stderr, L"Warnings and errors: %S\n", errors->GetStringPointer());
    }

    // Quit if compilation failed
    HRESULT compilation_status;
    results->GetStatus(&compilation_status);
    if(FAILED(compilation_status)) {
        fprintf(stderr, "Could not compile shader %s\n", input_file_name_raw);
        return E_COMPILATION_FAILED;
    }

    // Save the shader binary
    CComPtr<IDxcBlob> shader;
    results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);
    {
        auto* shader_file = fopen(output_file_name, "wb");
        if(shader_file == nullptr) {
            fprintf(stderr, "Could not open output file '%s'", output_file_name);
            return E_CANT_SAVE;
        }
        fwrite(shader->GetBufferPointer(), shader->GetBufferSize(), 1, shader_file);
        fclose(shader_file);

        printf("Saved shader to disk\n");
    }

    return 0;
}
