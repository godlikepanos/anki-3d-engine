// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/StaticCollisionNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/CollisionResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

StaticCollisionNode::StaticCollisionNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

StaticCollisionNode::~StaticCollisionNode()
{
}

Error StaticCollisionNode::init(const CString& resourceFname, const Transform& transform)
{
	// Load resource
	ANKI_CHECK(getResourceManager().loadResource(resourceFname, m_rsrc));

	// Create body
	PhysicsBodyInitInfo init;
	init.m_shape = m_rsrc->getShape();
	init.m_static = true;
	init.m_startTrf = transform;

	m_body = getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);

	return ErrorCode::NONE;
}

} // end namespace
