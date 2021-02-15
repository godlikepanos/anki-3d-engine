// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Octree.h>
#include <AnKi/Collision.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/List.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Spatial component. It is used by scene nodes that need to be placed inside the visibility structures.
class SpatialComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SpatialComponent)

public:
	SpatialComponent(SceneNode* node);

	~SpatialComponent();

	void setObbWorldSpace(const Obb& obb)
	{
		m_obb = obb;
		m_collisionObjectType = obb.CLASS_TYPE;
		m_markedForUpdate = true;
	}

	void setAabbWorldSpace(const Aabb& aabb)
	{
		m_aabb = aabb;
		m_collisionObjectType = aabb.CLASS_TYPE;
		m_markedForUpdate = true;
	}

	void setSphereWorldSpace(const Sphere& sphere)
	{
		m_sphere = sphere;
		m_collisionObjectType = sphere.CLASS_TYPE;
		m_markedForUpdate = true;
	}

	void setConvexHullWorldSpace(const ConvexHullShape& hull);

	template<typename T>
	const T& getCollisionShape() const
	{
		ANKI_ASSERT(T::CLASS_TYPE == m_collisionObjectType);
		return *reinterpret_cast<const T*>(&m_anyShape);
	}

	CollisionShapeType getCollisionShapeType() const
	{
		return m_collisionObjectType;
	}

	const Aabb& getAabbWorldSpace() const
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
	const Vec3& getSpatialOrigin() const
	{
		ANKI_ASSERT(m_origin.x() != MAX_F32);
		return m_origin;
	}

	/// See getSpatialOrigin()
	void setSpatialOrigin(const Vec3& origin)
	{
		m_origin = origin;
	}

	/// Update the "actual scene bounds" of the octree or not.
	void setUpdateOctreeBounds(Bool update)
	{
		m_updateOctreeBounds = update;
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override;

private:
	SceneNode* m_node;

	union
	{
		Obb m_obb;
		Aabb m_aabb;
		Sphere m_sphere;
		ConvexHullShape m_hull;
		U8 m_anyShape;
	};

	DynamicArray<Vec4> m_convexHullPoints;

	CollisionShapeType m_collisionObjectType = CollisionShapeType::COUNT;
	Aabb m_derivedAabb; ///< A faster shape

	Vec3 m_origin = Vec3(MAX_F32);

	OctreePlaceable m_octreeInfo;

	Bool m_markedForUpdate : 1;
	Bool m_placed : 1;
	Bool m_updateOctreeBounds : 1;
};
/// @}

} // end namespace anki
