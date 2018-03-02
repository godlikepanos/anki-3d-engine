// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Sampler initializer.
class alignas(4) SamplerInitInfo : public GrBaseInitInfo
{
public:
	F32 m_minLod = -1000.0;
	F32 m_maxLod = 1000.0;
	SamplingFilter m_minMagFilter = SamplingFilter::NEAREST;
	SamplingFilter m_mipmapFilter = SamplingFilter::BASE;
	CompareOperation m_compareOperation = CompareOperation::ALWAYS;
	I8 m_anisotropyLevel = 0;
	Bool8 m_repeat = true; ///< Repeat or clamp.
	U8 _m_padding[3] = {0, 0, 0};

	SamplerInitInfo() = default;

	SamplerInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	U64 computeHash() const
	{
		const U8* const first = reinterpret_cast<const U8* const>(&m_minLod);
		const U8* const last = reinterpret_cast<const U8* const>(&m_repeat) + sizeof(m_repeat);
		const U size = last - first;
		ANKI_ASSERT(
			size
			== sizeof(F32) * 2 + sizeof(SamplingFilter) * 2 + sizeof(CompareOperation) + sizeof(I8) + sizeof(Bool8));
		return anki::computeHash(first, size);
	}
};

/// GPU sampler.
class Sampler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::SAMPLER;

protected:
	/// Construct.
	Sampler(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~Sampler()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT Sampler* newInstance(GrManager* manager, const SamplerInitInfo& init);
};
/// @}

} // end namespace anki
