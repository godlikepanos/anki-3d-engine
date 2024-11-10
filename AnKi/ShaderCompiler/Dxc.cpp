// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Dxc.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/StringList.h>

#include <string>
#if ANKI_OS_WINDOWS
#	include <windows.h>
#	include <wrl/client.h>
#	include <ThirdParty/Dxc/d3d12shader.h>
#	define CComPtr Microsoft::WRL::ComPtr
#else
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#	pragma GCC diagnostic ignored "-Wambiguous-reversed-operator"
#	define __EMULATE_UUID
#	include <ThirdParty/Dxc/WinAdapter.h>
#	pragma GCC diagnostic pop
#endif
#include <ThirdParty/Dxc/dxcapi.h>

namespace anki {

static Atomic<U32> g_nextFileId = {1};

static HMODULE g_dxilLib = 0;
static HMODULE g_dxcLib = 0;
static DxcCreateInstanceProc g_DxcCreateInstance = nullptr;
static Mutex g_dxcLibMtx;

#define ANKI_DXC_CHECK(x) \
	do \
	{ \
		HRESULT rez; \
		if((rez = (x)) < 0) [[unlikely]] \
		{ \
			errorMessage.sprintf("DXC function failed (HRESULT: %d): %s", rez, #x); \
			return Error::kFunctionFailed; \
		} \
	} while(0)

static Error lazyDxcInit(ShaderCompilerString& errorMessage)
{
	LockGuard lock(g_dxcLibMtx);

	if(g_dxcLib == 0)
	{
		// Init DXC
#if ANKI_OS_WINDOWS
		g_dxilLib = LoadLibraryA(ANKI_SOURCE_DIRECTORY "/ThirdParty/Dxc/dxil.dll");
		if(g_dxilLib == 0)
		{
			errorMessage = "dxil.dll missing or wrong architecture";
			return Error::kFunctionFailed;
		}

		g_dxcLib = LoadLibraryA(ANKI_SOURCE_DIRECTORY "/ThirdParty/Dxc/dxcompiler.dll");
#else
		g_dxcLib = dlopen(ANKI_SOURCE_DIRECTORY "/ThirdParty/Dxc/libdxcompiler.so", RTLD_LAZY);
#endif
		if(g_dxcLib == 0)
		{
			errorMessage = "dxcompiler.dll/libdxcompiler.so missing or wrong architecture";
			return Error::kFunctionFailed;
		}

#if ANKI_OS_WINDOWS
		g_DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(g_dxcLib, "DxcCreateInstance"));
#else
		g_DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(dlsym(g_dxcLib, "DxcCreateInstance"));
#endif
		if(g_DxcCreateInstance == nullptr)
		{
			errorMessage = "DxcCreateInstance was not found in the dxcompiler.dll/libdxcompiler.so";
			return Error::kFunctionFailed;
		}
	}

	return Error::kNone;
}

static const WChar* profile(ShaderType shaderType)
{
	switch(shaderType)
	{
	case ShaderType::kVertex:
		return L"vs_6_8";
		break;
	case ShaderType::kPixel:
		return L"ps_6_8";
		break;
	case ShaderType::kDomain:
		return L"ds_6_8";
		break;
	case ShaderType::kHull:
		return L"ds_6_8";
		break;
	case ShaderType::kGeometry:
		return L"gs_6_8";
		break;
	case ShaderType::kAmplification:
		return L"as_6_8";
		break;
	case ShaderType::kMesh:
		return L"ms_6_8";
		break;
	case ShaderType::kCompute:
		return L"cs_6_8";
		break;
	case ShaderType::kRayGen:
	case ShaderType::kAnyHit:
	case ShaderType::kClosestHit:
	case ShaderType::kMiss:
	case ShaderType::kIntersection:
	case ShaderType::kCallable:
	case ShaderType::kWorkGraph:
		return L"lib_6_8";
		break;
	default:
		ANKI_ASSERT(0);
	};

	return L"";
}

static Error compileHlsl(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ConstWeakArray<CString> compilerArgs,
						 Bool spirv, ShaderCompilerDynamicArray<U8>& bin, ShaderCompilerString& errorMessage)
{
	ANKI_CHECK(lazyDxcInit(errorMessage));

	// Call DXC
	std::vector<std::wstring> dxcArgs;
	dxcArgs.push_back(L"-Fo");
	dxcArgs.push_back(L"-Wall");
	dxcArgs.push_back(L"-Wextra");
	dxcArgs.push_back(L"-Wno-conversion");
	dxcArgs.push_back(L"-Werror");
	dxcArgs.push_back(L"-Wfatal-errors");
	dxcArgs.push_back(L"-Wundef");
	dxcArgs.push_back(L"-Wno-unused-const-variable");
	dxcArgs.push_back(L"-Wno-unused-parameter");
	dxcArgs.push_back(L"-Wno-unneeded-internal-declaration");
	dxcArgs.push_back(L"-HV");
	dxcArgs.push_back(L"2021");
	dxcArgs.push_back(L"-E");
	dxcArgs.push_back(L"main");
	dxcArgs.push_back(L"-T");
	dxcArgs.push_back(profile(shaderType));

	if(ANKI_COMPILER_MSVC)
	{
		dxcArgs.push_back(L"-fdiagnostics-format=msvc"); // Make errors clickable in visual studio
	}

	if(spirv)
	{
		dxcArgs.push_back(L"-spirv");
		dxcArgs.push_back(L"-fspv-target-env=vulkan1.1spirv1.4");
		// dxcArgs.push_back(L"-fvk-support-nonzero-base-instance"); // Match DX12's behavior, SV_INSTANCEID starts from zero

		// Shift the bindings in order to identify the registers when doing reflection
		for(U32 ds = 0; ds < kMaxRegisterSpaces; ++ds)
		{
			dxcArgs.push_back(L"-fvk-b-shift");
			dxcArgs.push_back(std::to_wstring(kDxcVkBindingShifts[ds][HlslResourceType::kCbv]));
			dxcArgs.push_back(std::to_wstring(ds));

			dxcArgs.push_back(L"-fvk-t-shift");
			dxcArgs.push_back(std::to_wstring(kDxcVkBindingShifts[ds][HlslResourceType::kSrv]));
			dxcArgs.push_back(std::to_wstring(ds));

			dxcArgs.push_back(L"-fvk-u-shift");
			dxcArgs.push_back(std::to_wstring(kDxcVkBindingShifts[ds][HlslResourceType::kUav]));
			dxcArgs.push_back(std::to_wstring(ds));

			dxcArgs.push_back(L"-fvk-s-shift");
			dxcArgs.push_back(std::to_wstring(kDxcVkBindingShifts[ds][HlslResourceType::kSampler]));
			dxcArgs.push_back(std::to_wstring(ds));
		}
	}
	else
	{
		dxcArgs.push_back(L"-Wno-ignored-attributes"); // TODO remove that at some point
		dxcArgs.push_back(L"-Wno-inline-asm"); // Workaround a DXC bug
	}

	if(debugInfo)
	{
		dxcArgs.push_back(L"-Zi");
	}

	if(compileWith16bitTypes)
	{
		dxcArgs.push_back(L"-enable-16bit-types");
	}

	for(CString extraArg : compilerArgs)
	{
		WChar wstr[128];
		extraArg.toWideChars(wstr, 128);
		dxcArgs.push_back(wstr);
	}

	std::vector<const WChar*> dxcArgsRaw;
	dxcArgsRaw.reserve(dxcArgs.size());
	for(const auto& x : dxcArgs)
	{
		dxcArgsRaw.push_back(x.c_str());
	}

	// Compile
	CComPtr<IDxcCompiler3> dxcCompiler;
	ANKI_DXC_CHECK(g_DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

	const DxcBuffer buff = {src.getBegin(), src.getLength(), 0};
	CComPtr<IDxcResult> pResults;
	ANKI_DXC_CHECK(dxcCompiler->Compile(&buff, dxcArgsRaw.data(), U32(dxcArgsRaw.size()), nullptr, IID_PPV_ARGS(&pResults)));

	CComPtr<IDxcBlobUtf8> pErrors = nullptr;
	ANKI_DXC_CHECK(pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr));
	if(pErrors != nullptr && pErrors->GetStringLength() != 0)
	{
		errorMessage = pErrors->GetStringPointer();
	}

	HRESULT hrStatus;
	ANKI_DXC_CHECK(pResults->GetStatus(&hrStatus));
	if(FAILED(hrStatus))
	{
		return Error::kFunctionFailed;
	}

	CComPtr<IDxcBlob> pShader = nullptr;
	ANKI_DXC_CHECK(pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), nullptr));
	if(pShader != nullptr)
	{
		bin.resize(U32(pShader->GetBufferSize()));
		memcpy(bin.getBegin(), pShader->GetBufferPointer(), pShader->GetBufferSize());
	}

	return Error::kNone;
}

Error compileHlslToSpirv(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ConstWeakArray<CString> compilerArgs,
						 ShaderCompilerDynamicArray<U8>& spirv, ShaderCompilerString& errorMessage)
{
	return compileHlsl(src, shaderType, compileWith16bitTypes, debugInfo, compilerArgs, true, spirv, errorMessage);
}

Error compileHlslToDxil(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ConstWeakArray<CString> compilerArgs,
						ShaderCompilerDynamicArray<U8>& dxil, ShaderCompilerString& errorMessage)
{
	return compileHlsl(src, shaderType, compileWith16bitTypes, debugInfo, compilerArgs, false, dxil, errorMessage);
}

#if ANKI_OS_WINDOWS
Error doReflectionDxil(ConstWeakArray<U8> dxil, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorMessage)
{
	using Microsoft::WRL::ComPtr;

	// Lazyly load the DXC DLL
	{
		LockGuard lock(g_dxcLibMtx);

		if(g_dxcLib == 0)
		{
			// Init DXC
			g_dxcLib = LoadLibraryA(ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/dxcompiler.dll");
			if(g_dxcLib == 0)
			{
				ANKI_SHADER_COMPILER_LOGE("dxcompiler.dll missing or wrong architecture");
				return Error::kFunctionFailed;
			}

			g_DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(g_dxcLib, "DxcCreateInstance"));
			if(g_DxcCreateInstance == nullptr)
			{
				ANKI_SHADER_COMPILER_LOGE("DxcCreateInstance was not found in the dxcompiler.dll");
				return Error::kFunctionFailed;
			}
		}
	}

	const Bool isLib = (type >= ShaderType::kFirstRayTracing && type <= ShaderType::kLastRayTracing) || type == ShaderType::kWorkGraph;

	ComPtr<IDxcUtils> utils;
	ANKI_DXC_CHECK(g_DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));

	ComPtr<ID3D12ShaderReflection> dxRefl;
	ComPtr<ID3D12LibraryReflection> libRefl;
	ShaderCompilerDynamicArray<ID3D12FunctionReflection*> funcReflections;
	D3D12_SHADER_DESC shaderDesc = {};

	if(isLib)
	{
		const DxcBuffer buff = {dxil.getBegin(), dxil.getSizeInBytes(), 0};
		ANKI_DXC_CHECK(utils->CreateReflection(&buff, IID_PPV_ARGS(&libRefl)));

		D3D12_LIBRARY_DESC libDesc = {};
		libRefl->GetDesc(&libDesc);

		if(libDesc.FunctionCount == 0)
		{
			errorMessage.sprintf("Expecting at least 1 in D3D12_LIBRARY_DESC::FunctionCount");
			return Error::kUserData;
		}

		funcReflections.resize(libDesc.FunctionCount);
		for(U32 i = 0; i < libDesc.FunctionCount; ++i)
		{

			funcReflections[i] = libRefl->GetFunctionByIndex(i);
		}
	}
	else
	{
		const DxcBuffer buff = {dxil.getBegin(), dxil.getSizeInBytes(), 0};
		ANKI_DXC_CHECK(utils->CreateReflection(&buff, IID_PPV_ARGS(&dxRefl)));

		ANKI_DXC_CHECK(dxRefl->GetDesc(&shaderDesc));
	}

	for(U32 ifunc = 0; ifunc < ((isLib) ? funcReflections.getSize() : 1); ++ifunc)
	{
		U32 bindingCount;
		if(isLib)
		{
			D3D12_FUNCTION_DESC funcDesc;
			ANKI_DXC_CHECK(funcReflections[ifunc]->GetDesc(&funcDesc));
			bindingCount = funcDesc.BoundResources;
		}
		else
		{
			bindingCount = shaderDesc.BoundResources;
		}

		for(U32 i = 0; i < bindingCount; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			if(isLib)
			{
				ANKI_DXC_CHECK(funcReflections[ifunc]->GetResourceBindingDesc(i, &bindDesc));
			}
			else
			{
				ANKI_DXC_CHECK(dxRefl->GetResourceBindingDesc(i, &bindDesc));
			}

			ShaderReflectionBinding akBinding;

			akBinding.m_type = DescriptorType::kCount;
			akBinding.m_arraySize = U16(bindDesc.BindCount);
			akBinding.m_registerBindingPoint = bindDesc.BindPoint;

			if(bindDesc.Type == D3D_SIT_CBUFFER)
			{
				// ConstantBuffer

				if(bindDesc.Space == 3000 && bindDesc.BindPoint == 0)
				{
					// It's push/root constants

					ID3D12ShaderReflectionConstantBuffer* cbuffer =
						(isLib) ? funcReflections[ifunc]->GetConstantBufferByName(bindDesc.Name) : dxRefl->GetConstantBufferByName(bindDesc.Name);
					D3D12_SHADER_BUFFER_DESC desc;
					ANKI_DXC_CHECK(cbuffer->GetDesc(&desc));
					refl.m_descriptor.m_fastConstantsSize = desc.Size;

					continue;
				}

				akBinding.m_type = DescriptorType::kConstantBuffer;
			}
			else if(bindDesc.Type == D3D_SIT_TEXTURE && bindDesc.Dimension != D3D_SRV_DIMENSION_BUFFER)
			{
				// Texture2D etc
				akBinding.m_type = DescriptorType::kSrvTexture;
			}
			else if(bindDesc.Type == D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
			{
				// Buffer
				akBinding.m_type = DescriptorType::kSrvTexelBuffer;
			}
			else if(bindDesc.Type == D3D_SIT_SAMPLER)
			{
				// SamplerState
				akBinding.m_type = DescriptorType::kSampler;
			}
			else if(bindDesc.Type == D3D_SIT_UAV_RWTYPED && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
			{
				// RWBuffer
				akBinding.m_type = DescriptorType::kUavTexelBuffer;
			}
			else if(bindDesc.Type == D3D_SIT_UAV_RWTYPED && bindDesc.Dimension != D3D_SRV_DIMENSION_BUFFER)
			{
				// RWTexture2D etc
				akBinding.m_type = DescriptorType::kUavTexture;
			}
			else if(bindDesc.Type == D3D_SIT_BYTEADDRESS)
			{
				// ByteAddressBuffer
				akBinding.m_type = DescriptorType::kSrvByteAddressBuffer;
				akBinding.m_d3dStructuredBufferStride = sizeof(U32);
			}
			else if(bindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
			{
				// RWByteAddressBuffer
				akBinding.m_type = DescriptorType::kUavByteAddressBuffer;
				akBinding.m_d3dStructuredBufferStride = sizeof(U32);
			}
			else if(bindDesc.Type == D3D_SIT_RTACCELERATIONSTRUCTURE)
			{
				// RaytracingAccelerationStructure
				akBinding.m_type = DescriptorType::kAccelerationStructure;
			}
			else if(bindDesc.Type == D3D_SIT_STRUCTURED)
			{
				// StructuredBuffer
				akBinding.m_type = DescriptorType::kSrvStructuredBuffer;
				akBinding.m_d3dStructuredBufferStride = U16(bindDesc.NumSamples);
			}
			else if(bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
			{
				// RWStructuredBuffer
				akBinding.m_type = DescriptorType::kUavStructuredBuffer;
				akBinding.m_d3dStructuredBufferStride = U16(bindDesc.NumSamples);
			}
			else
			{
				errorMessage.sprintf("Unrecognized type for binding: %s", bindDesc.Name);
				return Error::kUserData;
			}

			Bool skip = false;
			if(isLib)
			{
				// Search if the binding exists because it may repeat
				for(U32 i = 0; i < refl.m_descriptor.m_bindingCounts[bindDesc.Space]; ++i)
				{
					if(refl.m_descriptor.m_bindings[bindDesc.Space][i] == akBinding)
					{
						skip = true;
						break;
					}
				}
			}

			if(!skip)
			{
				refl.m_descriptor.m_bindings[bindDesc.Space][refl.m_descriptor.m_bindingCounts[bindDesc.Space]] = akBinding;
				++refl.m_descriptor.m_bindingCounts[bindDesc.Space];
			}
		}
	}

	for(U32 i = 0; i < kMaxRegisterSpaces; ++i)
	{
		std::sort(refl.m_descriptor.m_bindings[i].getBegin(), refl.m_descriptor.m_bindings[i].getBegin() + refl.m_descriptor.m_bindingCounts[i]);
	}

	if(type == ShaderType::kVertex)
	{
		for(U32 i = 0; i < shaderDesc.InputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC in;
			ANKI_DXC_CHECK(dxRefl->GetInputParameterDesc(i, &in));

			VertexAttributeSemantic a = VertexAttributeSemantic::kCount;
#	define ANKI_ATTRIB_NAME(x, idx) CString(in.SemanticName) == #    x&& in.SemanticIndex == idx
			if(ANKI_ATTRIB_NAME(POSITION, 0))
			{
				a = VertexAttributeSemantic::kPosition;
			}
			else if(ANKI_ATTRIB_NAME(NORMAL, 0))
			{
				a = VertexAttributeSemantic::kNormal;
			}
			else if(ANKI_ATTRIB_NAME(TEXCOORD, 0))
			{
				a = VertexAttributeSemantic::kTexCoord;
			}
			else if(ANKI_ATTRIB_NAME(COLOR, 0))
			{
				a = VertexAttributeSemantic::kColor;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 0))
			{
				a = VertexAttributeSemantic::kMisc0;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 1))
			{
				a = VertexAttributeSemantic::kMisc1;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 2))
			{
				a = VertexAttributeSemantic::kMisc2;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 3))
			{
				a = VertexAttributeSemantic::kMisc3;
			}
			else if(ANKI_ATTRIB_NAME(SV_VERTEXID, 0) || ANKI_ATTRIB_NAME(SV_INSTANCEID, 0))
			{
				// Ignore
				continue;
			}
			else
			{
				errorMessage.sprintf("Unexpected attribute name: %s", in.SemanticName);
				return Error::kUserData;
			}
#	undef ANKI_ATTRIB_NAME

			refl.m_vertex.m_vertexAttributeMask |= VertexAttributeSemanticBit(1 << a);
			refl.m_vertex.m_vkVertexAttributeLocations[a] = U8(i); // Just set something
		}
	}

	if(type == ShaderType::kPixel)
	{
		for(U32 i = 0; i < shaderDesc.OutputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC desc;
			ANKI_DXC_CHECK(dxRefl->GetOutputParameterDesc(i, &desc));

			if(CString(desc.SemanticName) == "SV_TARGET")
			{
				refl.m_pixel.m_colorRenderTargetWritemask.set(desc.SemanticIndex);
			}
		}
	}

	return Error::kNone;
}
#endif

} // end namespace anki
