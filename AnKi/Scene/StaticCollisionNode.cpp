// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/StaticCollisionNode.h>
#include <AnKi/Scene/Components/BodyComponent.h>

namespace anki
{

StaticCollisionNode::StaticCollisionNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<BodyComponent>();
}

StaticCollisionNode::~StaticCollisionNode()
{
}

} // namespace anki
