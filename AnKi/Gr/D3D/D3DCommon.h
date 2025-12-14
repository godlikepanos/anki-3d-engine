// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Gr/BackendCommon/Common.h>
#include <AnKi/Util/System.h>
#include <string>
#include <locale>
#include <codecvt>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <d3d12.h>
#include <d3dx12/d3dx12_pipeline_state_stream.h>
#include <d3dx12/d3dx12_state_object.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <pix3.h>
#include <atlcomcli.h>
#include <AnKi/Util/CleanupWindows.h>

using Microsoft::WRL::ComPtr;

namespace anki {

/// @addtogroup directx
/// @{

#define ANKI_D3D_LOGI(...) ANKI_LOG("D3D", kNormal, __VA_ARGS__)
#define ANKI_D3D_LOGE(...) ANKI_LOG("D3D", kError, __VA_ARGS__)
#define ANKI_D3D_LOGW(...) ANKI_LOG("D3D", kWarning, __VA_ARGS__)
#define ANKI_D3D_LOGF(...) ANKI_LOG("D3D", kFatal, __VA_ARGS__)
#define ANKI_D3D_LOGV(...) ANKI_LOG("D3D", kVerbose, __VA_ARGS__)

#define ANKI_D3D_SELF(class_) class_& self = *static_cast<class_*>(this)
#define ANKI_D3D_SELF_CONST(class_) const class_& self = *static_cast<const class_*>(this)

void invokeDred();

#define ANKI_D3D_CHECKF(x) \
	do \
	{ \
		HRESULT rez; \
		if((rez = (x)) < 0) [[unlikely]] \
		{ \
			if(rez == DXGI_ERROR_DEVICE_REMOVED) \
			{ \
				invokeDred(); \
			} \
			ANKI_D3D_LOGF("D3D function failed (HRESULT: 0x%X message: %s): %s", rez, errorMessageToString(GetLastError()).cstr(), #x); \
		} \
	} while(0)

#define ANKI_D3D_CHECK(x) \
	do \
	{ \
		HRESULT rez; \
		if((rez = (x)) < 0) [[unlikely]] \
		{ \
			ANKI_D3D_LOGE("D3D function failed (HRESULT: 0x%X message: %s): %s", rez, errorMessageToString(GetLastError()).cstr(), #x); \
			if(rez == DXGI_ERROR_DEVICE_REMOVED) \
			{ \
				invokeDred(); \
			} \
			return Error::kFunctionFailed; \
		} \
	} while(0)

// Globaly define the versions of some D3D objects
using D3D12GraphicsCommandListX = ID3D12GraphicsCommandList10;
using ID3D12DeviceX = ID3D12Device14;

enum class D3DTextureViewType : U8
{
	kSrv,
	kRtv,
	kDsv,
	kUav,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(D3DTextureViewType)

enum class D3DBufferViewType : U8
{
	kCbv,
	kSrv,
	kUav,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(D3DBufferViewType)

inline std::string ws2s(const std::wstring& wstr)
{
	const int slength = int(wstr.length());
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, 0, 0, 0, 0);
	std::string r(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, &r[0], len, 0, 0);
	return r;
}

inline std::wstring s2ws(const std::string& str)
{
	std::wstring wstr;
	size_t size;
	wstr.resize(str.length());
	mbstowcs_s(&size, &wstr[0], wstr.size() + 1, str.c_str(), str.size());
	return wstr;
}

template<typename T>
void safeRelease(T*& p)
{
	if(p)
	{
		p->Release();
		p = nullptr;
	}
}

ID3D12DeviceX& getDevice();

GrManagerImpl& getGrManagerImpl();

inline [[nodiscard]] D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE convertLoadOp(RenderTargetLoadOperation ak)
{
	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE out;
	switch(ak)
	{
	case RenderTargetLoadOperation::kClear:
		out = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		break;
	case RenderTargetLoadOperation::kDontCare:
		out = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		break;
	default:
		ANKI_ASSERT(ak == RenderTargetLoadOperation::kLoad);
		out = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		break;
	}
	return out;
}

inline [[nodiscard]] D3D12_RENDER_PASS_ENDING_ACCESS_TYPE convertStoreOp(RenderTargetStoreOperation ak)
{
	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE out;
	switch(ak)
	{
	case RenderTargetStoreOperation::kDontCare:
		out = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		break;
	default:
		ANKI_ASSERT(ak == RenderTargetStoreOperation::kStore);
		out = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		break;
	}
	return out;
}

[[nodiscard]] D3D12_FILTER convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter, CompareOperation compareOp, U32 anisotropy);

inline [[nodiscard]] D3D12_TEXTURE_ADDRESS_MODE convertSamplingAddressing(SamplingAddressing addr)
{
	D3D12_TEXTURE_ADDRESS_MODE out = {};
	switch(addr)
	{
	case SamplingAddressing::kClamp:
		out = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		break;
	case SamplingAddressing::kRepeat:
		out = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		break;
	case SamplingAddressing::kBlack:
	case SamplingAddressing::kWhite:
		out = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_COMPARISON_FUNC convertComparisonFunc(CompareOperation comp)
{
	D3D12_COMPARISON_FUNC out = {};

	switch(comp)
	{
	case CompareOperation::kAlways:
		out = D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case CompareOperation::kLess:
		out = D3D12_COMPARISON_FUNC_LESS;
		break;
	case CompareOperation::kEqual:
		out = D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case CompareOperation::kLessEqual:
		out = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case CompareOperation::kGreater:
		out = D3D12_COMPARISON_FUNC_GREATER;
		break;
	case CompareOperation::kGreaterEqual:
		out = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	case CompareOperation::kNotEqual:
		out = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		break;
	case CompareOperation::kNever:
		out = D3D12_COMPARISON_FUNC_NEVER;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_PRIMITIVE_TOPOLOGY_TYPE convertPrimitiveTopology(PrimitiveTopology top)
{
	D3D12_PRIMITIVE_TOPOLOGY_TYPE out = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;

	switch(top)
	{
	case PrimitiveTopology::kPoints:
		out = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		break;
	case PrimitiveTopology::kLines:
		out = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		break;
	case PrimitiveTopology::kTriangles:
		out = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D_PRIMITIVE_TOPOLOGY convertPrimitiveTopology2(PrimitiveTopology top)
{
	D3D_PRIMITIVE_TOPOLOGY out = {};

	switch(top)
	{
	case PrimitiveTopology::kPoints:
		out = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case PrimitiveTopology::kLines:
		out = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case PrimitiveTopology::kLineStip:
		out = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case PrimitiveTopology::kTriangles:
		out = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case PrimitiveTopology::kTriangleStrip:
		out = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	case PrimitiveTopology::kPatches:
		ANKI_ASSERT(!"TODO");
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_BLEND convertBlendFactor(BlendFactor f)
{
	D3D12_BLEND out = {};

	switch(f)
	{
	case BlendFactor::kZero:
		out = D3D12_BLEND_ZERO;
		break;
	case BlendFactor::kOne:
		out = D3D12_BLEND_ONE;
		break;
	case BlendFactor::kSrcColor:
		out = D3D12_BLEND_SRC_COLOR;
		break;
	case BlendFactor::kOneMinusSrcColor:
		out = D3D12_BLEND_INV_SRC_COLOR;
		break;
	case BlendFactor::kDstColor:
		out = D3D12_BLEND_DEST_COLOR;
		break;
	case BlendFactor::kOneMinusDstColor:
		out = D3D12_BLEND_INV_DEST_COLOR;
		break;
	case BlendFactor::kSrcAlpha:
		out = D3D12_BLEND_SRC_ALPHA;
		break;
	case BlendFactor::kOneMinusSrcAlpha:
		out = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case BlendFactor::kDstAlpha:
		out = D3D12_BLEND_DEST_ALPHA;
		break;
	case BlendFactor::kOneMinusDstAlpha:
		out = D3D12_BLEND_INV_DEST_ALPHA;
		break;
	case BlendFactor::kConstantColor:
		out = D3D12_BLEND_BLEND_FACTOR;
		break;
	case BlendFactor::kOneMinusConstantColor:
		out = D3D12_BLEND_INV_BLEND_FACTOR;
		break;
	case BlendFactor::kConstantAlpha:
		out = D3D12_BLEND_ALPHA_FACTOR;
		break;
	case BlendFactor::kOneMinusConstantAlpha:
		out = D3D12_BLEND_INV_ALPHA_FACTOR;
		break;
	case BlendFactor::kSrcAlphaSaturate:
		out = D3D12_BLEND_SRC_ALPHA_SAT;
		break;
	case BlendFactor::kSrc1Color:
		out = D3D12_BLEND_SRC1_COLOR;
		break;
	case BlendFactor::kOneMinusSrc1Color:
		out = D3D12_BLEND_INV_SRC1_COLOR;
		break;
	case BlendFactor::kSrc1Alpha:
		out = D3D12_BLEND_SRC1_ALPHA;
		break;
	case BlendFactor::kOneMinusSrc1Alpha:
		out = D3D12_BLEND_INV_SRC1_ALPHA;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_BLEND_OP convertBlendOperation(BlendOperation b)
{
	D3D12_BLEND_OP out = {};

	switch(b)
	{
	case BlendOperation::kAdd:
		out = D3D12_BLEND_OP_ADD;
		break;
	case BlendOperation::kSubtract:
		out = D3D12_BLEND_OP_SUBTRACT;
		break;
	case BlendOperation::kReverseSubtract:
		out = D3D12_BLEND_OP_REV_SUBTRACT;
		break;
	case BlendOperation::kMin:
		out = D3D12_BLEND_OP_MIN;
		break;
	case BlendOperation::kMax:
		out = D3D12_BLEND_OP_MAX;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_COMPARISON_FUNC convertCompareOperation(CompareOperation c)
{
	D3D12_COMPARISON_FUNC out = {};

	switch(c)
	{
	case CompareOperation::kAlways:
		out = D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	case CompareOperation::kLess:
		out = D3D12_COMPARISON_FUNC_LESS;
		break;
	case CompareOperation::kEqual:
		out = D3D12_COMPARISON_FUNC_EQUAL;
		break;
	case CompareOperation::kLessEqual:
		out = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		break;
	case CompareOperation::kGreater:
		out = D3D12_COMPARISON_FUNC_GREATER;
		break;
	case CompareOperation::kGreaterEqual:
		out = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		break;
	case CompareOperation::kNotEqual:
		out = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		break;
	case CompareOperation::kNever:
		out = D3D12_COMPARISON_FUNC_NEVER;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_STENCIL_OP convertStencilOperation(StencilOperation o)
{
	D3D12_STENCIL_OP out = {};

	switch(o)
	{
	case StencilOperation::kKeep:
		out = D3D12_STENCIL_OP_KEEP;
		break;
	case StencilOperation::kZero:
		out = D3D12_STENCIL_OP_ZERO;
		break;
	case StencilOperation::kReplace:
		out = D3D12_STENCIL_OP_REPLACE;
		break;
	case StencilOperation::kIncrementAndClamp:
		out = D3D12_STENCIL_OP_INCR_SAT;
		break;
	case StencilOperation::kDecrementAndClamp:
		out = D3D12_STENCIL_OP_DECR_SAT;
		break;
	case StencilOperation::kInvert:
		out = D3D12_STENCIL_OP_INVERT;
		break;
	case StencilOperation::kIncrementAndWrap:
		out = D3D12_STENCIL_OP_INCR;
		break;
	case StencilOperation::kDecrementAndWrap:
		out = D3D12_STENCIL_OP_DECR;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

inline [[nodiscard]] D3D12_FILL_MODE convertFillMode(FillMode f)
{
	D3D12_FILL_MODE out = {};

	switch(f)
	{
	case FillMode::kWireframe:
		out = D3D12_FILL_MODE_WIREFRAME;
		break;
	case FillMode::kSolid:
		out = D3D12_FILL_MODE_SOLID;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return out;
}

[[nodiscard]] inline D3D12_CULL_MODE convertCullMode(FaceSelectionBit c)
{
	ANKI_ASSERT(c != FaceSelectionBit::kFrontAndBack);
	D3D12_CULL_MODE out = {};

	if(!!(c & FaceSelectionBit::kFront))
	{
		out = D3D12_CULL_MODE_FRONT;
	}
	else if(!!(c & FaceSelectionBit::kBack))
	{
		out = D3D12_CULL_MODE_BACK;
	}
	else
	{
		out = D3D12_CULL_MODE_NONE;
	}

	return out;
}

[[nodiscard]] DXGI_FORMAT convertFormat(Format fmt);

[[nodiscard]] inline DXGI_FORMAT convertIndexType(IndexType ak)
{
	DXGI_FORMAT out;
	switch(ak)
	{
	case IndexType::kU16:
		out = DXGI_FORMAT_R16_UINT;
		break;
	case IndexType::kU32:
		out = DXGI_FORMAT_R32_UINT;
		break;
	default:
		ANKI_ASSERT(0);
		out = DXGI_FORMAT_UNKNOWN;
	}

	return out;
}
/// @}

} // end namespace anki
