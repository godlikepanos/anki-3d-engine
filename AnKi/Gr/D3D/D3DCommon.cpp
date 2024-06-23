// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

namespace anki {

ID3D12DeviceX& getDevice()
{
	return static_cast<GrManagerImpl&>(GrManager::getSingleton()).getDevice();
}

GrManagerImpl& getGrManagerImpl()
{
	return static_cast<GrManagerImpl&>(GrManager::getSingleton());
}

D3D12_FILTER convertFilter(SamplingFilter minMagFilter, SamplingFilter mipFilter, CompareOperation compareOp, U32 anisotropy)
{
	D3D12_FILTER out = {};

	const Bool anisoEnabled = anisotropy > 0;
	const Bool compareEnabled = compareOp != CompareOperation::kAlways;

	const D3D12_FILTER_TYPE d3dMinMagFilter = (minMagFilter == SamplingFilter::kNearest) ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;
	const D3D12_FILTER_TYPE d3dMipFilter = (mipFilter == SamplingFilter::kNearest) ? D3D12_FILTER_TYPE_POINT : D3D12_FILTER_TYPE_LINEAR;

	D3D12_FILTER_REDUCTION_TYPE d3dReduction;
	if(compareEnabled)
	{
		d3dReduction = D3D12_FILTER_REDUCTION_TYPE_COMPARISON;
	}
	else if(minMagFilter == SamplingFilter::kMin || mipFilter == SamplingFilter::kMin)
	{
		d3dReduction = D3D12_FILTER_REDUCTION_TYPE_MINIMUM;
	}
	else if(minMagFilter == SamplingFilter::kMax || mipFilter == SamplingFilter::kMax)
	{
		d3dReduction = D3D12_FILTER_REDUCTION_TYPE_MAXIMUM;
	}
	else
	{
		d3dReduction = D3D12_FILTER_REDUCTION_TYPE_STANDARD;
	}

	if(!anisoEnabled)
	{
		out = D3D12_ENCODE_BASIC_FILTER(d3dMinMagFilter, d3dMinMagFilter, d3dMipFilter, d3dReduction);
	}
	else if(d3dMinMagFilter == D3D12_FILTER_TYPE_LINEAR && d3dMipFilter == D3D12_FILTER_TYPE_POINT)
	{
		out = D3D12_ENCODE_MIN_MAG_ANISOTROPIC_MIP_POINT_FILTER(d3dReduction);
	}
	else
	{
		ANKI_ASSERT(d3dMinMagFilter == D3D12_FILTER_TYPE_LINEAR && d3dMipFilter == D3D12_FILTER_TYPE_LINEAR);
		out = D3D12_ENCODE_ANISOTROPIC_FILTER(d3dReduction);
	}

	return out;
}

void invokeDred()
{
	getGrManagerImpl().invokeDred();
}

DXGI_FORMAT convertFormat(Format fmt)
{
	DXGI_FORMAT out = DXGI_FORMAT_UNKNOWN;

	switch(fmt)
	{
	case Format::kNone:
		out = DXGI_FORMAT_UNKNOWN;
		break;

#define ANKI_FORMAT_DEF(type, vk, d3d, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, depthStencil) \
	case Format::k##type: \
		out = DXGI_FORMAT(d3d); \
		break;
#include <AnKi/Gr/BackendCommon/Format.def.h>
#undef ANKI_FORMAT_DEF

	default:
		ANKI_ASSERT(0);
	}

	ANKI_ASSERT(out < DXGI_FORMAT(1000));
	return out;
}

} // end namespace anki
