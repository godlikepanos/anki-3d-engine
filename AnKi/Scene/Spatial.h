// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Octree.h>
#include <AnKi/Collision.h>


namespace anki {

/// @addtogroup scene
/// @{

/// XXX
class Spatial
{
public:
	Spatial(SceneComponent* owner)
	{
		ANKI_ASSERT(owner);
		m_octreeInfo.m_userData = owner;
	}

	Spatial(const Spatial&) = delete;

	~Spatial()
	{
		ANKI_ASSERT(!m_placed && "Forgot to call removeFromOctree");
	}

	Spatial& operator=(const Spatial&) = delete;

	const Aabb& getAabbWorldSpace() const
	{
		ANKI_ASSERT(!m_alwaysVisible || !m_dirty);
		return m_aabb;
	}

	/// Update the "actual scene bounds" of the octree or not.
	void setUpdatesOctreeBounds(Bool update)
	{
		m_updatesOctreeBounds = update;
	}

	/// Make it or not always visible.
	void setAlwaysVisible(Bool alwaysVisible)
	{
		m_alwaysVisible = alwaysVisible;
	}

	/// Check if it's always visible or not.
	Bool getAlwaysVisible() const
	{
		return m_alwaysVisible;
	}

	template<typename TCollisionShape>
	void update(Octree& octree, const TCollisionShape& shape)
	{
		m_aabb = computeAabb(shape);
		updateCommon(octree);
	}

	template<typename TVec>
	void update(Octree& octree, ConstWeakArray<TVec> points)
	{
		ANKI_ASSERT(pointCount > 0);
		TVec min(kMaxF32), max(kMinF32);
		for(const TVec& point : points)
		{
			min = min.min(point);
			max = max.max(point);
		}
		m_aabb.setMin(min.xyz());
		m_aabb.setMax(max.xyz());
		updateCommon(octree);
	}

	void removeFromOctree(Octree& octree)
	{
		if(m_placed)
		{
			octree.remove(m_octreeInfo);
			m_placed = false;
		}
	}

private:
	Aabb m_aabb = Aabb(Vec3(-1.0f), Vec3(1.0f)); ///< A faster shape.

	OctreePlaceable m_octreeInfo;

	Bool m_placed : 1 = false;
	Bool m_updatesOctreeBounds : 1 = true;
	Bool m_alwaysVisible : 1 = false;

	void updateCommon(Octree& octree)
	{
		if(!m_alwaysVisible) [[likely]]
		{
			octree.place(m_aabb, &m_octreeInfo, m_updatesOctreeBounds);
		}
		else
		{
			octree.placeAlwaysVisible(&m_octreeInfo);
		}

		m_placed = true;
		m_octreeInfo.reset();
	}
};

template<>
void Spatial::update<Aabb>(Octree& octree, const Aabb& shape)
{
	m_aabb = shape;
	updateCommon(octree);
}
/// @}

} // end namespace anki
