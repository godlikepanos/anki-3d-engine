// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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

/// Spatial component. It is used by scene nodes that need to be placed inside the visibility structures.
class SpatialComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::SPATIAL;

	SpatialComponent(SceneNode* node, const Obb* obb);

	SpatialComponent(SceneNode* node, const Aabb* aabb);

	SpatialComponent(SceneNode* node, const Sphere* sphere);

	SpatialComponent(SceneNode* node, const ConvexHullShape* hull);

	~SpatialComponent();

	template<typename T>
	const T& getCollisionShape() const
	{
		ANKI_ASSERT(T::CLASS_TYPE == m_collisionObjectType);
		return *reinterpret_cast<const T*>(m_shapePtr);
	}

	CollisionShapeType getCollisionShapeType() const
	{
		return m_collisionObjectType;
	}

	const Aabb& getAabb() const
	{
		return m_derivedAabb;
	}

	const SceneNode& getSceneNode() const
	{
		return *m_node;
	}

	SceneNode& getSceneNode()
	{
		return *m_node;
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

	/// Update the "actual scene bounds" of the octree or not.
	void setUpdateOctreeBounds(Bool update)
	{
		m_updateOctreeBounds = update;
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;
	/// @}

private:
	SceneNode* m_node;

	union
	{
		const Obb* m_obb;
		const Aabb* m_aabb;
		const Sphere* m_sphere;
		const ConvexHullShape* m_hull;
		const void* m_shapePtr;
	};

	CollisionShapeType m_collisionObjectType;
	Aabb m_derivedAabb; ///< A faster shape

	Vec4 m_origin = Vec4(MAX_F32, MAX_F32, MAX_F32, 0.0f);

	OctreePlaceable m_octreeInfo;

	Bool m_markedForUpdate = false;
	Bool m_placed = false;
	Bool m_updateOctreeBounds = true;
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
