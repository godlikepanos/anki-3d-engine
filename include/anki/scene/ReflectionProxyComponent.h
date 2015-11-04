// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>
#include <anki/collision/Plane.h>

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
		m_dirty = true;
	}

	const Array<Vec4, 4>& getVertices() const
	{
		return m_quad;
	}

	const Plane& getPlane() const
	{
		return m_plane;
	}

	ANKI_USE_RESULT Error update(SceneNode& node, F32 prevTime, F32 crntTime,
		Bool& updated) final;

private:
	Array<Vec4, 4> m_quad; ///< Quad verts.
	Plane m_plane; ///< Caches some values.
	Bool8 m_dirty = true;
};
/// @}

} // end namespace anki

