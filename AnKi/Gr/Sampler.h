// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Sampler initializer.
class alignas(4) SamplerInitInfo : public GrBaseInitInfo
{
public:
	F32 m_minLod = -1000.0f;
	F32 m_maxLod = 1000.0f;
	F32 m_lodBias = 0.0f;
	SamplingFilter m_minMagFilter = SamplingFilter::kNearest;
	SamplingFilter m_mipmapFilter = SamplingFilter::kBase;
	CompareOperation m_compareOperation = CompareOperation::kAlways;
	U8 m_anisotropyLevel = 0;
	SamplingAddressing m_addressing = SamplingAddressing::kRepeat;
	U8 _m_padding[3] = {0, 0, 0};

	SamplerInitInfo() = default;

	SamplerInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	U64 computeHash() const
	{
		const U8* first = reinterpret_cast<const U8*>(&m_minLod);
		const U8* last = reinterpret_cast<const U8*>(&m_addressing) + sizeof(m_addressing);
		const U32 size = U32(last - first);
		ANKI_ASSERT(size
					== sizeof(F32) * 3 + sizeof(SamplingFilter) * 2 + sizeof(CompareOperation) + sizeof(I8)
						   + sizeof(SamplingAddressing));
		return anki::computeHash(first, size);
	}
};

/// GPU sampler.
class Sampler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kSampler;

protected:
	/// Construct.
	Sampler(GrManager* manager, CString name)
		: GrObject(manager, kClassType, name)
	{
	}

	/// Destroy.
	~Sampler()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static Sampler* newInstance(GrManager* manager, const SamplerInitInfo& init);
};
/// @}

} // end namespace anki
