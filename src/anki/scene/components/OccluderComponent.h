// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/Math.h>
#include <anki/collision/Aabb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Occluder component.
class OccluderComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::OCCLUDER;

	/// @note The component won't own the triangles.
	OccluderComponent()
		: SceneComponent(CLASS_TYPE)
	{
	}

	/// Get the vertex positions and other info.
	void getVertices(const Vec3*& begin, U32& count, U32& stride) const
	{
		ANKI_ASSERT(m_begin && m_count && m_stride);
		begin = m_begin;
		count = m_count;
		stride = m_stride;
	}

	/// Point the component to the vertex positions in world space. You are not supposed to call this often.
	void setVertices(const Vec3* begin, U count, U stride);

	const Aabb& getBoundingVolume() const
	{
		return m_aabb;
	}

private:
	const Vec3* m_begin = nullptr;
	U32 m_count = 0;
	U32 m_stride = 0;
	Aabb m_aabb;
};
/// @}

} // end namespace anki
