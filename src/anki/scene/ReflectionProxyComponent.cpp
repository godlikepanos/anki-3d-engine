// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProxyComponent.h>

namespace anki
{

void ReflectionProxyComponent::setQuad(U index, const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d)
{
	m_dirty = true;

	m_faces[index].m_vertices[0] = a;
	m_faces[index].m_vertices[1] = b;
	m_faces[index].m_vertices[2] = c;
	m_faces[index].m_vertices[3] = d;
}

Error ReflectionProxyComponent::update(SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated)
{
	if(m_dirty)
	{
		m_dirty = false;
		updated = true;

		for(Face& face : m_faces)
		{
			const Vec4& a = face.m_vertices[0];
			const Vec4& b = face.m_vertices[1];
			const Vec4& c = face.m_vertices[2];
			const Vec4& d = face.m_vertices[3];
			(void)d;

			// Update the plane
			face.m_plane.setFrom3Points(a, b, c);

#if ANKI_EXTRA_CHECKS == 1
			// Make sure that all points are co-planar
			Vec4 n0 = (b - a).cross(c - b);
			Vec4 n1 = (c - b).cross(d - c);
			Vec4 n2 = (d - c).cross(a - d);
			n0.normalize();
			n1.normalize();
			n2.normalize();

			F32 dota = absolute(n0.dot(n1) - n1.dot(n2));
			F32 dotb = absolute(n0.dot(n2) - n0.dot(n1));
			ANKI_ASSERT(dota < 0.001 && dotb < 0.001);
#endif
		}
	}
	else
	{
		updated = false;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
