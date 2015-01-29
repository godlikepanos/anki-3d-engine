// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/LightComponent.h"

namespace anki {

//==============================================================================
LightComponent::LightComponent(SceneNode* node, LightType type)
:	SceneComponent(Type::LIGHT, node),
	m_type(type)
{
	setInnerAngle(toRad(45.0));
	setOuterAngle(toRad(30.0));
	m_radius = 1.0;
}

//==============================================================================
Error LightComponent::update(SceneNode&, F32, F32, Bool& updated)
{
	if(m_dirty)
	{
		updated = true;
		m_dirty = false;
	}

	return ErrorCode::NONE;
}

} // end namespace anki

