// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>

namespace anki {

// Sampler initializer
class SamplerInitInfo : public GrBaseInitInfo
{
public:
	F32 m_minLod = -1000.0f;
	F32 m_maxLod = 1000.0f;
	F32 m_lodBias = 0.0f;
	SamplingFilter m_minMagFilter = SamplingFilter::kNearest;
	SamplingFilter m_mipmapFilter = SamplingFilter::kNearest;
	CompareOperation m_compareOperation = CompareOperation::kAlways;
	U8 m_anisotropyLevel = 0;
	SamplingAddressing m_addressing = SamplingAddressing::kRepeat;

	SamplerInitInfo() = default;

	SamplerInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	U64 computeHash() const
	{
		Array<U32, 8> arr;
		U32 count = 0;
		arr[count++] = asU32(m_minLod);
		arr[count++] = asU32(m_maxLod);
		arr[count++] = asU32(m_lodBias);
		arr[count++] = U32(m_minMagFilter);
		arr[count++] = U32(m_mipmapFilter);
		arr[count++] = U32(m_compareOperation);
		arr[count++] = m_anisotropyLevel;
		arr[count++] = U32(m_addressing);
		return computeObjectHash(arr);
	}
};

// GPU sampler
class Sampler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kSampler;

protected:
	Sampler(CString name)
		: GrObject(kClassType, name)
	{
	}

	~Sampler()
	{
	}

private:
	[[nodiscard]] static Sampler* newInstance(const SamplerInitInfo& init);
};

} // end namespace anki
