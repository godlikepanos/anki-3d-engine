// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>

namespace anki
{

/// The AnKi passes visible to materials.
enum class Pass : U8
{
	MS_FS, ///< MS or FS
	SM,
	COUNT
};

/// A key that consistst of the rendering pass and the level of detail
class RenderingKey
{
public:
	Pass m_pass;
	U8 m_lod;
	Bool8 m_tessellation;
	U8 m_instanceCount;

	RenderingKey(Pass pass, U8 lod, Bool tessellation, U instanceCount)
		: m_pass(pass)
		, m_lod(lod)
		, m_tessellation(tessellation)
		, m_instanceCount(instanceCount)
	{
		ANKI_ASSERT(m_instanceCount <= MAX_INSTANCES && m_instanceCount != 0);
		ANKI_ASSERT(m_lod <= MAX_LODS);
	}

	RenderingKey()
		: RenderingKey(Pass::MS_FS, 0, false, 1)
	{
	}

	RenderingKey(const RenderingKey& b)
		: RenderingKey(b.m_pass, b.m_lod, b.m_tessellation, b.m_instanceCount)
	{
	}
};

/// The hash function
class RenderingKeyHasher
{
public:
	PtrSize operator()(const RenderingKey& key) const
	{
		return U8(key.m_pass) | (key.m_lod << 8) | (key.m_tessellation << 16) | (key.m_instanceCount << 24);
	}
};

/// The collision evaluation function
class RenderingKeyEqual
{
public:
	Bool operator()(const RenderingKey& a, const RenderingKey& b) const
	{
		return a.m_pass == b.m_pass && a.m_lod == b.m_lod && a.m_tessellation == b.m_tessellation
			&& a.m_instanceCount == b.m_instanceCount;
	}
};

} // end namespace anki
