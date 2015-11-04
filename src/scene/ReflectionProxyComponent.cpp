// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProxyComponent.h>

namespace anki {

//==============================================================================
Error ReflectionProxyComponent::update(SceneNode& node, F32 prevTime,
	F32 crntTime, Bool& updated)
{
	if(m_dirty)
	{
		m_dirty = false;
		updated = true;

		// Update the plane
		m_plane.setFrom3Points(m_quad[0], m_quad[1], m_quad[2]);

#if ANKI_ASSERTIONS == 1
		// Make sure that all points are co-planar
		Vec4 n0 = (m_quad[1] - m_quad[0]).cross(m_quad[2] - m_quad[1]);
		Vec4 n1 = (m_quad[2] - m_quad[1]).cross(m_quad[3] - m_quad[2]);
		Vec4 n2 = (m_quad[3] - m_quad[2]).cross(m_quad[0] - m_quad[3]);
		n0.normalize();
		n1.normalize();
		n2.normalize();
		ANKI_ASSERT(isZero(n0.dot(n1) - n1.dot(n2))
			&& isZero(n0.dot(n2) - n0.dot(n1)));
#endif

		updated = false;
		return ErrorCode::NONE;
	}
	else
	{
		updated = false;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
