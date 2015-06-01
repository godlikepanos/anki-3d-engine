// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/FrustumComponent.h"
#include "anki/scene/Visibility.h"
#include "anki/util/Rtti.h"

namespace anki {

//==============================================================================
FrustumComponent::FrustumComponent(SceneNode* node, Frustum* frustum)
	: SceneComponent(Type::FRUSTUM, node)
	, m_frustum(frustum)
	, m_flags(0)
{
	// WARNING: Never touch m_frustum in constructor
	ANKI_ASSERT(frustum);
	markShapeForUpdate();
	markTransformForUpdate();

	setEnabledVisibilityTests(VisibilityTestFlag::TEST_ALL);
}

//==============================================================================
void FrustumComponent::setVisibilityTestResults(VisibilityTestResults* visible)
{
	ANKI_ASSERT(m_visible == nullptr);
	m_visible = visible;

	m_stats.m_renderablesCount = visible->getRenderablesCount();
	m_stats.m_lightsCount = visible->getLightsCount();
}

//==============================================================================
Error FrustumComponent::update(SceneNode& node, F32, F32, Bool& updated)
{
	m_visible = nullptr;

	updated = false;

	if(m_flags.bitsEnabled(SHAPE_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_pm = m_frustum->calculateProjectionMatrix();
		computeProjectionParams();
	}

	if(m_flags.bitsEnabled(TRANSFORM_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_vm = Mat4(m_frustum->getTransform().getInverse());
	}

	if(updated)
	{
		m_vpm = m_pm * m_vm;
		m_flags.disableBits(
			SHAPE_MARKED_FOR_UPDATE | TRANSFORM_MARKED_FOR_UPDATE);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void FrustumComponent::computeProjectionParams()
{
	const Mat4& m = m_pm;

	if(isa<PerspectiveFrustum>(m_frustum))
	{
		// First, z' = (m * Pv) / 2 + 0.5 where Pv is the view space position.
		// Solving that for Pv.z we get
		// Pv.z = A / (z' + B)
		// where A = (-m23 / 2) and B = (m22/2 - 0.5)
		// so we save the A and B in the projection params vector
		m_projParams.z() = -m(2, 3) * 0.5;
		m_projParams.w() = m(2, 2) * 0.5 - 0.5;

		// Using the same logic the Pv.x = x' * w / m00
		// so Pv.x = x' * Pv.z * (-1 / m00)
		m_projParams.x() = -1.0 / m(0, 0);

		// Same for y
		m_projParams.y() = -1.0 / m(1, 1);
	}
	else
	{
		ANKI_ASSERT(0 && "TODO");
	}
}

} // end namespace anki
