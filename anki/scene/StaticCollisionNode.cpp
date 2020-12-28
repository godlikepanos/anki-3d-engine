// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/StaticCollisionNode.h>
#include <anki/scene/components/BodyComponent.h>

namespace anki
{

StaticCollisionNode::StaticCollisionNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

StaticCollisionNode::~StaticCollisionNode()
{
}

Error StaticCollisionNode::init()
{
	newComponent<BodyComponent>();
	return Error::NONE;
}

} // namespace anki
