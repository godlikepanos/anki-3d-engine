// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>

namespace anki
{

/// An always visible node that debug draws primitives of the physics world.
class PhysicsDebugNode : public SceneNode
{
public:
	PhysicsDebugNode(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~PhysicsDebugNode();

	ANKI_USE_RESULT Error init();

private:
	class MyRenderComponent;
};

} // end namespace anki