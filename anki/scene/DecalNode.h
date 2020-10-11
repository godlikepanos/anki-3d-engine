// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/collision/Obb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Node that has a decal component.
class DecalNode : public SceneNode
{
public:
	DecalNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~DecalNode();

	ANKI_USE_RESULT Error init();

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	DebugDrawer2 m_dbgDrawer;
	TextureResourcePtr m_dbgTex;

	void onMove(MoveComponent& movec);
	void onDecalUpdated();

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};
/// @}

} // end namespace anki
