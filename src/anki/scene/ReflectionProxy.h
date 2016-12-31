// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/Math.h>
#include <anki/collision/Obb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Proxy used in realtime reflections.
class ReflectionProxy : public SceneNode
{
	friend class ReflectionProxyMoveFeedbackComponent;

public:
	ReflectionProxy(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~ReflectionProxy()
	{
		m_quadsLSpace.destroy(getSceneAllocator());
	}

	/// Create the proxy. The points form a quad and they should be in local
	/// space.
	ANKI_USE_RESULT Error init(const CString& proxyMesh);

private:
	DynamicArray<Array<Vec4, 4>> m_quadsLSpace; ///< Quads in local space.
	Obb m_boxLSpace;
	Obb m_boxWSpace;

	void onMoveUpdate(const MoveComponent& move);
};
/// @}

} // end namespace anki
