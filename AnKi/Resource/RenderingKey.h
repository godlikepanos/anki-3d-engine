// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/Common.h>
#include <AnKi/Util/Hash.h>

namespace anki {

/// The AnKi passes visible to materials.
enum class RenderingTechnique : U8
{
	kGBuffer = 0,
	kGBufferEarlyZ = 1,
	kShadow = 2,
	kForward = 3,
	kRtShadow = 4,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderingTechnique)

enum class RenderingTechniqueBit : U8
{
	kNone = 0,
	kGBuffer = 1 << 0,
	kGBufferEarlyZ = 1 << 1,
	kShadow = 1 << 2,
	kForward = 1 << 3,
	kRtShadow = 1 << 4,

	kAllRt = kRtShadow
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RenderingTechniqueBit)

/// A key that consistst of the rendering pass and the level of detail
class RenderingKey
{
public:
	RenderingKey(RenderingTechnique technique, U32 lod, U32 instanceCount, Bool skinned, Bool velocity)
		: m_technique(technique)
		, m_lod(U8(lod))
		, m_instanceCount(U8(instanceCount))
		, m_skinned(skinned)
		, m_velocity(velocity)
	{
		ANKI_ASSERT(instanceCount <= kMaxInstanceCount && instanceCount != 0);
		ANKI_ASSERT(lod <= kMaxLodCount);
	}

	RenderingKey()
		: RenderingKey(RenderingTechnique::kFirst, 0, 1, false, false)
	{
	}

	RenderingKey(const RenderingKey& b)
		: RenderingKey(b.m_technique, b.m_lod, b.m_instanceCount, b.m_skinned, b.m_velocity)
	{
	}

	RenderingKey& operator=(const RenderingKey& b)
	{
		memcpy(this, &b, sizeof(*this));
		return *this;
	}

	Bool operator==(const RenderingKey& b) const
	{
		return m_technique == b.m_technique && m_lod == b.m_lod && m_instanceCount == b.m_instanceCount
			   && m_skinned == b.m_skinned && m_velocity == b.m_velocity;
	}

	RenderingTechnique getRenderingTechnique() const
	{
		return m_technique;
	}

	void setRenderingTechnique(RenderingTechnique t)
	{
		m_technique = t;
	}

	U32 getLod() const
	{
		return m_lod;
	}

	void setLod(U32 lod)
	{
		ANKI_ASSERT(lod < kMaxLodCount);
		m_lod = U8(lod);
	}

	U32 getInstanceCount() const
	{
		return m_instanceCount;
	}

	void setInstanceCount(U32 instanceCount)
	{
		ANKI_ASSERT(instanceCount <= kMaxInstanceCount && instanceCount > 0);
		m_instanceCount = U8(instanceCount);
	}

	Bool getSkinned() const
	{
		return m_skinned;
	}

	void setSkinned(Bool is)
	{
		m_skinned = is;
	}

	Bool getVelocity() const
	{
		return m_velocity;
	}

	void setVelocity(Bool v)
	{
		m_velocity = v;
	}

private:
	RenderingTechnique m_technique;
	U8 m_lod;
	U8 m_instanceCount;
	Bool m_skinned;
	Bool m_velocity;
};

static_assert(sizeof(RenderingKey) == sizeof(U8) * 5, "RenderingKey needs to be packed because of hashing");

} // end namespace anki
