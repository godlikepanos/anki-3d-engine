// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

SpatialComponent::SpatialComponent(SceneNode* node, const Obb* obb)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(obb);
	markForUpdate();
	m_octreeInfo.m_userData = this;
	m_obb = obb;
	m_collisionObjectType = obb->CLASS_TYPE;
}

SpatialComponent::SpatialComponent(SceneNode* node, const Aabb* aabb)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(aabb);
	markForUpdate();
	m_octreeInfo.m_userData = this;
	m_aabb = aabb;
	m_collisionObjectType = aabb->CLASS_TYPE;
}

SpatialComponent::SpatialComponent(SceneNode* node, const Sphere* sphere)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(sphere);
	markForUpdate();
	m_octreeInfo.m_userData = this;
	m_sphere = sphere;
	m_collisionObjectType = sphere->CLASS_TYPE;
}

SpatialComponent::SpatialComponent(SceneNode* node, const ConvexHullShape* hull)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(hull);
	markForUpdate();
	m_octreeInfo.m_userData = this;
	m_hull = hull;
	m_collisionObjectType = hull->CLASS_TYPE;
}

SpatialComponent::~SpatialComponent()
{
	if(m_placed)
	{
		m_node->getSceneGraph().getOctree().remove(m_octreeInfo);
	}
}

Error SpatialComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	ANKI_ASSERT(&node == m_node);

	updated = m_markedForUpdate;
	if(updated)
	{
		// Compute the AABB
		switch(m_collisionObjectType)
		{
		case CollisionShapeType::AABB:
			m_derivedAabb = *m_aabb;
			break;
		case CollisionShapeType::OBB:
			m_derivedAabb = computeAabb(*m_obb);
			break;
		case CollisionShapeType::SPHERE:
			m_derivedAabb = computeAabb(*m_sphere);
			break;
		case CollisionShapeType::CONVEX_HULL:
			m_derivedAabb = computeAabb(*m_hull);
			break;
		default:
			ANKI_ASSERT(0);
		}

		m_markedForUpdate = false;

		m_node->getSceneGraph().getOctree().place(m_derivedAabb, &m_octreeInfo, m_updateOctreeBounds);
		m_placed = true;
	}

	m_octreeInfo.reset();

	return Error::NONE;
}

} // end namespace anki
