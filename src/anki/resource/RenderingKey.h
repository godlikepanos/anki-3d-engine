// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>
#include <anki/util/Hash.h>

namespace anki
{

/// The AnKi passes visible to materials.
enum class Pass : U8
{
	GB, ///< GBuffer.
	FS, ///< Forward shading.
	SM, ///< Shadow mapping.
	EZ, ///< Early Z.
	COUNT
};

/// A key that consistst of the rendering pass and the level of detail
class RenderingKey
{
public:
	Pass m_pass;
	U8 m_lod;
	U8 m_instanceCount;
	Bool8 m_skinned;
	Bool8 m_velocity;

	RenderingKey(Pass pass, U8 lod, U instanceCount, Bool8 skinned, Bool8 velocity)
		: m_pass(pass)
		, m_lod(lod)
		, m_instanceCount(instanceCount)
		, m_skinned(skinned)
		, m_velocity(velocity)
	{
		ANKI_ASSERT(m_instanceCount <= MAX_INSTANCES && m_instanceCount != 0);
		ANKI_ASSERT(m_lod <= MAX_LOD_COUNT);
	}

	RenderingKey()
		: RenderingKey(Pass::GB, 0, 1, false, false)
	{
	}

	RenderingKey(const RenderingKey& b)
		: RenderingKey(b.m_pass, b.m_lod, b.m_instanceCount, b.m_skinned, b.m_velocity)
	{
	}

	Bool operator==(const RenderingKey& b) const
	{
		return m_pass == b.m_pass && m_lod == b.m_lod && m_instanceCount == b.m_instanceCount
			   && m_skinned == b.m_skinned && m_velocity == b.m_velocity;
	}
};

template<>
constexpr Bool isPacked<RenderingKey>()
{
	return sizeof(RenderingKey) == 5;
}

/// The hash function
class RenderingKeyHasher
{
public:
	U64 operator()(const RenderingKey& key) const
	{
		return computeHash(&key, sizeof(key));
	}
};

/// The collision evaluation function
class RenderingKeyEqual
{
public:
	Bool operator()(const RenderingKey& a, const RenderingKey& b) const
	{
		return a == b;
	}
};

} // end namespace anki
