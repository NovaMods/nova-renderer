#pragma once

#ifdef _DEBUG
#define DEBUG_ENABLED true
#else
#define DEBUG_ENABLED false
#endif

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <dxcapi.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

void ResultCheck(HRESULT Result, const wchar_t* ErrorMessage, const wchar_t* ErrorTitle) {
	if (Result != S_OK) {
		MessageBox(NULL, ErrorMessage, ErrorTitle, NULL);
		exit(-1);
	}
}

class UnknownInterface {

protected:

	IUnknown* pInterface;

	UnknownInterface() : pInterface{ nullptr } {

	}

	~UnknownInterface() {
		if (this->pInterface != nullptr) {
			this->pInterface->Release();
			this->pInterface = nullptr;
		}
	}

public:

	virtual IUnknown* GetInterface() = 0;

};

// DirectX Shader Compiler.

class DXCLibrary : UnknownInterface {

public:

	DXCLibrary() {
		HRESULT Result{ S_OK };

		Result = DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary),
			reinterpret_cast<void**>(&this->pInterface));

		ResultCheck(Result, L"DxcCreateInstance() failed.", L"DirectXStuff::DXCLibrary Error");
	}

	IDxcLibrary* GetInterface() {
		if (this->pInterface != nullptr) {
			return reinterpret_cast<IDxcLibrary*>(this->pInterface);
		}
		else {
			return nullptr;
		}
	}

};

class DXCCompiler : UnknownInterface {

public:

	DXCCompiler() {
		HRESULT Result{ S_OK };

		Result = DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler),
			reinterpret_cast<void**>(&this->pInterface));

		ResultCheck(Result, L"DxcCreateInstance() failed.", L"DirectXStuff::DXCCompiler Error");
	}

	IDxcCompiler2* GetInterface() {
		if (this->pInterface != nullptr) {
			return reinterpret_cast<IDxcCompiler2*>(this->pInterface);
		}
		else {
			return nullptr;
		}
	}

};

struct ShaderConfig {
	const wchar_t* ShaderFileName{ L"Shader.hlsl" };
	unsigned __int32 ShaderFileEncoding{ CP_UTF8 };
	const wchar_t* ShaderEntryPoint{ L"Main" };
	const wchar_t* TargetProfile{ L"cs_6_3" };
};

class Shader {

protected:

	IDxcBlobEncoding* pSourceBlob;
	IDxcOperationResult* pOperationResult;
	IDxcBlob* pCompiledShaderBlob;
	IDxcIncludeHandler* pIncludeHandler;

public:

	Shader(IDxcLibrary* pDXCLibrary, IDxcCompiler* pDXCCompiler, ShaderConfig Config) :
		pSourceBlob{ nullptr }, pOperationResult{ nullptr }, pCompiledShaderBlob{ nullptr },
		pIncludeHandler{ nullptr } {
		HRESULT Result{ S_OK };

		Result = pDXCLibrary->CreateBlobFromFile(Config.ShaderFileName, &Config.ShaderFileEncoding,
			&pSourceBlob);

		ResultCheck(Result, L"CreateBlobFromFile() failed.", L"DirectXStuff::Shader Error");

		Result = pDXCCompiler->Compile(pSourceBlob, Config.ShaderFileName, Config.ShaderEntryPoint,
			Config.TargetProfile, NULL, 0u, NULL, 0u, pIncludeHandler, &pOperationResult);

		ResultCheck(Result, L"Compile() failed.", L"DirectXStuff::Shader Error");

		pOperationResult->GetStatus(&Result);

		ResultCheck(Result, L"Compile() failed.", L"DirectXStuff::Shader Error");

		Result = pOperationResult->GetResult(&pCompiledShaderBlob);

		ResultCheck(Result, L"Compile() failed.", L"DirectXStuff::Shader Error");
	}

	unsigned __int32 GetShaderByteCodeSize() {
		if (pCompiledShaderBlob != nullptr) {
			return static_cast<unsigned __int32>(pCompiledShaderBlob->GetBufferSize());
		}
		else {
			return 0u;
		}
	}

	void* GetShaderByteCode() {
		if (pCompiledShaderBlob != nullptr) {
			return pCompiledShaderBlob->GetBufferPointer();
		}
		else {
			return nullptr;
		}
	}

	~Shader() {
		if (this->pIncludeHandler != nullptr) {
			this->pIncludeHandler->Release();
			this->pIncludeHandler = nullptr;
		}

		if (this->pCompiledShaderBlob != nullptr) {
			this->pCompiledShaderBlob->Release();
			this->pCompiledShaderBlob = nullptr;
		}

		if (this->pOperationResult != nullptr) {
			this->pOperationResult->Release();
			this->pOperationResult = nullptr;
		}

		if (this->pSourceBlob != nullptr) {
			this->pSourceBlob->Release();
			this->pSourceBlob = nullptr;
		}
	}

};
