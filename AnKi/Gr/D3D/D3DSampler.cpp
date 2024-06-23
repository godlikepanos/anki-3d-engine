// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DSampler.h>

namespace anki {

Sampler* Sampler::newInstance(const SamplerInitInfo& init)
{
	SamplerImpl* impl = anki::newInstance<SamplerImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

SamplerImpl::~SamplerImpl()
{
	DescriptorFactory::getSingleton().freePersistent(m_handle);
}

Error SamplerImpl::init(const SamplerInitInfo& inf)
{
	m_handle = DescriptorFactory::getSingleton().allocatePersistent(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, false);

	D3D12_SAMPLER_DESC desc = {};
	desc.Filter = convertFilter(inf.m_minMagFilter, inf.m_mipmapFilter, inf.m_compareOperation, inf.m_anisotropyLevel);
	desc.AddressU = convertSamplingAddressing(inf.m_addressing);
	desc.AddressV = desc.AddressU;
	desc.AddressW = desc.AddressU;
	desc.MipLODBias = inf.m_lodBias;
	desc.MaxAnisotropy = inf.m_anisotropyLevel;
	desc.ComparisonFunc = D3D12_DECODE_IS_COMPARISON_FILTER(desc.Filter) ? convertComparisonFunc(inf.m_compareOperation) : D3D12_COMPARISON_FUNC_NONE;

	if(inf.m_addressing == SamplingAddressing::kBlack)
	{
		zeroMemory(desc.BorderColor);
	}
	else
	{
		desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 1.0f;
	}

	desc.MinLOD = inf.m_minLod;
	desc.MaxLOD = inf.m_maxLod;

	getDevice().CreateSampler(&desc, m_handle.getCpuOffset());

	return Error::kNone;
}

} // end namespace anki
