// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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

	COUNT,
	FIRST = 0
};

/// A key that consistst of the rendering pass and the level of detail
class RenderingKey
{
public:
	RenderingKey(Pass pass, U32 lod, U32 instanceCount, Bool skinned, Bool velocity)
		: m_pass(pass)
		, m_lod(U8(lod))
		, m_instanceCount(U8(instanceCount))
		, m_skinned(skinned)
		, m_velocity(velocity)
	{
		ANKI_ASSERT(instanceCount <= MAX_INSTANCES && instanceCount != 0);
		ANKI_ASSERT(lod <= MAX_LOD_COUNT);
	}

	RenderingKey()
		: RenderingKey(Pass::GB, 0, 1, false, false)
	{
	}

	RenderingKey(const RenderingKey& b)
		: RenderingKey(b.m_pass, b.m_lod, b.m_instanceCount, b.m_skinned, b.m_velocity)
	{
	}

	RenderingKey& operator=(const RenderingKey& b)
	{
		memcpy(this, &b, sizeof(*this));
		return *this;
	}

	Bool operator==(const RenderingKey& b) const
	{
		return m_pass == b.m_pass && m_lod == b.m_lod && m_instanceCount == b.m_instanceCount
			   && m_skinned == b.m_skinned && m_velocity == b.m_velocity;
	}

	Pass getPass() const
	{
		return m_pass;
	}

	void setPass(Pass p)
	{
		m_pass = p;
	}

	U32 getLod() const
	{
		return m_lod;
	}

	void setLod(U32 lod)
	{
		ANKI_ASSERT(lod < MAX_LOD_COUNT);
		m_lod = U8(lod);
	}

	U32 getInstanceCount() const
	{
		return m_instanceCount;
	}

	void setInstanceCount(U32 instanceCount)
	{
		ANKI_ASSERT(instanceCount <= MAX_INSTANCES && instanceCount > 0);
		m_instanceCount = U8(instanceCount);
	}

	Bool isSkinned() const
	{
		return m_skinned;
	}

	void setSkinned(Bool is)
	{
		m_skinned = is;
	}

	Bool hasVelocity() const
	{
		return m_velocity;
	}

	void setVelocity(Bool v)
	{
		m_velocity = v;
	}

private:
	Pass m_pass;
	U8 m_lod;
	U8 m_instanceCount;
	Bool m_skinned : 1;
	Bool m_velocity : 1;
};

template<>
constexpr Bool isPacked<RenderingKey>()
{
	return sizeof(RenderingKey) == 5;
}

} // end namespace anki
