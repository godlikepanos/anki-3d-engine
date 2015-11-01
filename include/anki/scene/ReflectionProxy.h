// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/Math.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Proxy used in realtime reflections.
class ReflectionProxy: public SceneNode
{
	friend class ReflectionProxyMoveFeedbackComponent;

public:
	ReflectionProxy(SceneGraph* scene)
		: SceneNode(scene)
	{}

	~ReflectionProxy()
	{}

	/// Create the proxy. The points form a quad and they should be in local
	/// space.
	ANKI_USE_RESULT Error create(const CString& name, const Vec4& p0,
		const Vec4& p1, const Vec4& p2, const Vec4& p3);

private:
	Array<Vec4, 4> m_quadLSpace; ///< Quad verts.

	void onMoveUpdate(const MoveComponent& move);
};
/// @}

} // end namespace anki

