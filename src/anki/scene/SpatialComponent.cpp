// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SpatialComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Sector.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

SpatialComponent::SpatialComponent(SceneNode* node, const CollisionShape* shape)
	: SceneComponent(CLASS_TYPE, node)
	, m_shape(shape)
{
	ANKI_ASSERT(shape);
	markForUpdate();
}

SpatialComponent::~SpatialComponent()
{
	getSceneGraph().getSectorGroup().spatialDeleted(this);
}

Error SpatialComponent::update(SceneNode&, F32, F32, Bool& updated)
{
	m_flags.unset(Flag::VISIBLE_ANY);

	updated = m_flags.get(Flag::MARKED_FOR_UPDATE);
	if(updated)
	{
		m_shape->computeAabb(m_aabb);
		getSceneGraph().getSectorGroup().spatialUpdated(this);
		m_flags.unset(Flag::MARKED_FOR_UPDATE);
	}

	return ErrorCode::NONE;
}

} // end namespace anki
