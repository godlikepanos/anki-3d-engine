// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Frustum.h>
#include <AnKi/Collision/Functions.h>

namespace anki {

Array<Mat3x4, 6> Frustum::m_omnidirectionalRotations = {Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, -kPi / 2.0f, 0.0f))), // +x
														Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, kPi / 2.0f, 0.0f))), // -x
														Mat3x4(Vec3(0.0f), Mat3(Euler(kPi / 2.0f, 0.0f, 0.0f))), // +y
														Mat3x4(Vec3(0.0f), Mat3(Euler(-kPi / 2.0f, 0.0f, 0.0f))), // -y
														Mat3x4(Vec3(0.0f), Mat3::getIdentity()), // -z
														Mat3x4(Vec3(0.0f), Mat3(Euler(0.0f, kPi, 0.0f)))}; // +z

Frustum::Frustum()
{
	// Set some default values
	init(FrustumType::kPerspective);
	update();
}

Frustum::~Frustum()
{
}

void Frustum::init(FrustumType type)
{
	ANKI_ASSERT(type < FrustumType::kCount);
	m_frustumType = type;
	setNear(kDefaultNear);
	setFar(kDefaultFar);
	if(m_frustumType == FrustumType::kPerspective)
	{
		setFovX(kDefaultFovAngle);
		setFovY(kDefaultFovAngle);
	}
	else
	{
		setLeft(-5.0f);
		setRight(5.0f);
		setBottom(-1.0f);
		setTop(1.0f);
	}
}

Bool Frustum::update()
{
	Bool updated = false;

	// Update the shape
	if(m_shapeDirty)
	{
		updated = true;

		if(m_frustumType == FrustumType::kPerspective)
		{
			m_projMat =
				Mat4::calculatePerspectiveProjectionMatrix(m_perspective.m_fovX, m_perspective.m_fovY, m_perspective.m_near, m_perspective.m_far);
		}
		else
		{
			m_projMat = Mat4::calculateOrthographicProjectionMatrix(m_ortho.m_right, m_ortho.m_left, m_ortho.m_top, m_ortho.m_bottom, m_ortho.m_near,
																	m_ortho.m_far);
		}
	}

	// Update transform related things
	if(m_worldTransformDirty)
	{
		updated = true;
		m_viewMat = Mat3x4(m_worldTransform.invert());
	}

	// Updates that are affected by transform & shape updates
	if(updated)
	{
		m_shapeDirty = false;
		m_worldTransformDirty = false;

		m_viewProjMat = m_projMat * Mat4(m_viewMat, Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	return updated;
}

} // end namespace anki
