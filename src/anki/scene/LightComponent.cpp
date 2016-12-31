// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/LightComponent.h>

namespace anki
{

LightComponent::LightComponent(SceneNode* node, LightComponentType type)
	: SceneComponent(CLASS_TYPE, node)
	, m_type(type)
{
	setInnerAngle(toRad(45.0));
	setOuterAngle(toRad(30.0));
	m_radius = 1.0;
}

Error LightComponent::update(SceneNode&, F32, F32, Bool& updated)
{
	if(m_dirty)
	{
		updated = true;
		m_dirty = false;
	}
	else
	{
		updated = false;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
