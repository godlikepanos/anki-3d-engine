// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/FrustumComponent.h>
#include <anki/scene/Visibility.h>

namespace anki
{

FrustumComponent::FrustumComponent(SceneNode* node, Frustum* frustum)
	: SceneComponent(CLASS_TYPE, node)
	, m_frustum(frustum)
	, m_flags(0)
{
	// WARNING: Never touch m_frustum in constructor
	ANKI_ASSERT(frustum);
	markShapeForUpdate();
	markTransformForUpdate();

	setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
}

Error FrustumComponent::update(SceneNode& node, Second, Second, Bool& updated)
{
	updated = false;

	if(m_flags.get(SHAPE_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_pm = m_frustum->calculateProjectionMatrix();
	}

	if(m_flags.get(TRANSFORM_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_vm = Mat4(m_frustum->getTransform().getInverse());
	}

	if(updated)
	{
		m_vpm = m_pm * m_vm;
		m_flags.unset(SHAPE_MARKED_FOR_UPDATE | TRANSFORM_MARKED_FOR_UPDATE);
	}

	return Error::NONE;
}

} // end namespace anki
