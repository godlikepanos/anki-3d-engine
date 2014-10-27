// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
SpatialComponent::SpatialComponent(SceneNode* node, Flag flags)
:	SceneComponent(Type::SPATIAL, node), 
	Bitset<Flag>(flags)
{
	markForUpdate();
}

//==============================================================================
SpatialComponent::~SpatialComponent()
{}

//==============================================================================
Error SpatialComponent::update(SceneNode&, F32, F32, Bool& updated)
{
	updated = false;

	updated = bitsEnabled(Flag::MARKED_FOR_UPDATE);
	if(updated)
	{
		getSpatialCollisionShape().computeAabb(m_aabb);
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
