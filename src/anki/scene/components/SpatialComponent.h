// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/scene/Octree.h>
#include <anki/Collision.h>
#include <anki/util/BitMask.h>
#include <anki/util/Enum.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Spatial component for scene nodes. It is used by scene nodes that need to be placed in the visibility structures.
class SpatialComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::SPATIAL;

	SpatialComponent(SceneNode* node, const CollisionShape* shape);

	~SpatialComponent();

	const CollisionShape& getSpatialCollisionShape() const
	{
		return *m_shape;
	}

	const Aabb& getAabb() const
	{
		return m_aabb;
	}

	const SceneNode& getSceneNode() const
	{
		return *m_node;
	}

	SceneNode& getSceneNode()
	{
		return *m_node;
	}

	/// Get optimal collision shape for visibility tests
	const CollisionShape& getVisibilityCollisionShape()
	{
		const CollisionShape& cs = getSpatialCollisionShape();
		if(cs.getType() == CollisionShapeType::SPHERE)
		{
			return cs;
		}
		else
		{
			return m_aabb;
		}
	}

	/// Used for sorting spatials. In most object the origin is the center of mass but for cameras the origin is the
	/// eye point.
	const Vec4& getSpatialOrigin() const
	{
		ANKI_ASSERT(m_origin.x() != MAX_F32);
		return m_origin;
	}

	void setSpatialOrigin(const Vec4& origin)
	{
		m_origin = origin;
	}

	/// The derived class has to manually call this method when the collision shape got updated.
	void markForUpdate()
	{
		m_markedForUpdate = true;
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;
	/// @}

private:
	SceneNode* m_node;
	const CollisionShape* m_shape;
	Aabb m_aabb; ///< A faster shape
	Vec4 m_origin = Vec4(MAX_F32, MAX_F32, MAX_F32, 0.0);

	Bool8 m_markedForUpdate = false;
	Bool8 m_placed = false;

	OctreePlaceable m_octreeInfo;
};

/// A class that holds spatial information and implements the SpatialComponent virtuals. You just need to update the
/// OBB manually
class ObbSpatialComponent : public SpatialComponent
{
public:
	Obb m_obb;

	ObbSpatialComponent(SceneNode* node)
		: SpatialComponent(node, &m_obb)
	{
	}
};
/// @}

} // end namespace anki
