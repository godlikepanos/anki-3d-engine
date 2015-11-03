// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/Math.h>
#include <anki/collision/Obb.h>

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
	ANKI_USE_RESULT Error create(const CString& name, F32 width, F32 height,
		F32 maxDistance);

private:
	Array<Vec4, 4> m_quadLSpace; ///< Quad verts.
	Obb m_boxLSpace;
	Obb m_boxWSpace;

	void onMoveUpdate(const MoveComponent& move);
};
/// @}

} // end namespace anki

