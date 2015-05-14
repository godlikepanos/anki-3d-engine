// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Sector.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
SpatialComponent::SpatialComponent(
	SceneNode* node,
	const CollisionShape* shape,
	Flag flags)
:	SceneComponent(Type::SPATIAL, node),
	Bitset<Flag>(flags),
	m_shape(shape)
{
	ANKI_ASSERT(shape);
	markForUpdate();
}

//==============================================================================
SpatialComponent::~SpatialComponent()
{
	getSceneGraph().getSectorGroup().spatialDeleted(this);
}

//==============================================================================
Error SpatialComponent::update(SceneNode&, F32, F32, Bool& updated)
{
	updated = bitsEnabled(Flag::MARKED_FOR_UPDATE);
	if(updated)
	{
		m_shape->computeAabb(m_aabb);
		getSceneGraph().getSectorGroup().spatialUpdated(this);
		disableBits(Flag::MARKED_FOR_UPDATE);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void SpatialComponent::reset()
{
	disableBits(Flag::VISIBLE_ANY);
}

} // end namespace anki
