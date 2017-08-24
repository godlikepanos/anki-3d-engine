// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class SamplerInitInfo : public GrBaseInitInfo
{
public:
	F32 m_minLod = -1000.0;
	F32 m_maxLod = 1000.0;
	SamplingFilter m_minMagFilter = SamplingFilter::NEAREST;
	SamplingFilter m_mipmapFilter = SamplingFilter::BASE;
	CompareOperation m_compareOperation = CompareOperation::ALWAYS;
	I8 m_anisotropyLevel = 0;
	Bool8 m_repeat = true; ///< Repeat or clamp.

	SamplerInitInfo() = default;

	SamplerInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	U64 computeHash() const
	{
		const U8* start = reinterpret_cast<const U8*>(&m_minLod);
		const U8* end = reinterpret_cast<const U8*>(&m_repeat) + sizeof(m_repeat);
		ANKI_ASSERT((end - start == 13) && "Class needs to be tightly packed since we hash it");
		return anki::computeHash(start, end - start);
	}
};

/// Texture initializer.
class TextureInitInfo : public GrBaseInitInfo
{
public:
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 1; //< Relevant only for 3D textures.
	U32 m_layerCount = 1; ///< Relevant only for texture arrays.

	TextureType m_type = TextureType::_2D;

	TextureUsageBit m_usage = TextureUsageBit::NONE; ///< How the texture will be used.
	TextureUsageBit m_initialUsage = TextureUsageBit::NONE; ///< It's initial usage.

	/// It's usual usage. That way you won't need to call CommandBuffer::informTextureXXXCurrentUsage() all the time.
	TextureUsageBit m_usageWhenEncountered = TextureUsageBit::NONE;

	U8 m_mipmapsCount = 1;

	PixelFormat m_format;
	U8 m_samples = 1;

	SamplerInitInfo m_sampling;

	TextureInitInfo() = default;

	TextureInitInfo(CString name)
		: GrBaseInitInfo(name)
		, m_sampling(name)
	{
	}

	U64 computeHash() const
	{
		const U8* start = reinterpret_cast<const U8*>(&m_width);
		const U8* end = reinterpret_cast<const U8*>(&m_samples) + sizeof(m_samples);
		ANKI_ASSERT((end - start == 24) && "Class needs to be tightly packed since we hash it");

		const U8* starts = reinterpret_cast<const U8*>(&m_sampling.m_minLod);
		const U8* ends = reinterpret_cast<const U8*>(&m_sampling.m_repeat) + sizeof(m_sampling.m_repeat);

		return appendHash(starts, ends - starts, anki::computeHash(start, end - start));
	}
};

/// GPU texture
class Texture final : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<TextureImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::TEXTURE;

	/// Construct.
	Texture(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~Texture();

	/// Create it.
	void init(const TextureInitInfo& init);
};
/// @}

} // end namespace anki
