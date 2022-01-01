// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/DebugDrawer.h>

namespace anki {

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
