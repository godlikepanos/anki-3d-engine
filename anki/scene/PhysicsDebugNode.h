// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/DebugDrawer.h>

namespace anki
{

/// An always visible node that debug draws primitives of the physics world.
class PhysicsDebugNode : public SceneNode
{
public:
	PhysicsDebugNode(SceneGraph* scene, CString name);

	~PhysicsDebugNode();

private:
	PhysicsDebugDrawer m_physDbgDrawer;

	void draw(RenderQueueDrawContext& ctx);
};

} // end namespace anki
