// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Reflection proxy component.
class ReflectionProxyComponent: public SceneComponent
{
public:
	ReflectionProxyComponent(SceneNode* node)
		: SceneComponent(SceneComponent::Type::REFLECTION_PROXY, node)
	{}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::REFLECTION_PROXY;
	}

	void setVertex(U idx, const Vec4& v)
	{
		m_quad[idx] = v;
	}

#if ANKI_ASSERTS_ENABLED
	ANKI_USE_RESULT Error update(SceneNode& node, F32 prevTime, F32 crntTime,
		Bool& updated) final
	{
		// Check that the quad is planar
		Vec4 n0 = (m_quad[1] - m_quad[0]).cross(m_quad[2] - m_quad[1]);
		Vec4 n1 = (m_quad[2] - m_quad[1]).cross(m_quad[3] - m_quad[2]);
		Vec4 n2 = (m_quad[3] - m_quad[2]).cross(m_quad[0] - m_quad[3]);
		n0.normalize();
		n1.normalize();
		n2.normalize();
		ANKI_ASSERT(isZero(n0.dot(n1) - n1.dot(n2))
			&& isZero(n0.dot(n2) - n0.dot(n1)));

		updated = false;
		return ErrorCode::NONE;
	}
#endif

private:
	Array<Vec4, 4> m_quad; ///< Quad verts.
};
/// @}

} // end namespace anki

