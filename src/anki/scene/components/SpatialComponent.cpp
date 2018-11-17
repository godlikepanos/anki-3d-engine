// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

SpatialComponent::SpatialComponent(SceneNode* node, const CollisionShape* shape)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
	, m_shape(shape)
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(shape);
	markForUpdate();
	m_octreeInfo.m_userData = this;
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
		m_shape->computeAabb(m_aabb);
		m_markedForUpdate = false;

		m_node->getSceneGraph().getOctree().place(m_aabb, &m_octreeInfo);
		m_placed = true;
	}

	m_octreeInfo.reset();

	return Error::NONE;
}

} // end namespace anki
