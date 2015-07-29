// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/StaticCollisionNode.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/CollisionResource.h"
#include "anki/resource/ResourceManager.h"
#include "anki/physics/PhysicsBody.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
StaticCollisionNode::StaticCollisionNode(SceneGraph* scene)
	: SceneNode(scene)
{}

//==============================================================================
StaticCollisionNode::~StaticCollisionNode()
{}

//==============================================================================
Error StaticCollisionNode::create(const CString& name,
	const CString& resourceFname, const Transform& transform)
{
	// Load resource
	ANKI_CHECK(getResourceManager().loadResource(resourceFname, m_rsrc));

	// Create body
	PhysicsBody::Initializer init;
	init.m_shape = m_rsrc->getShape();
	init.m_static = true;
	init.m_startTrf = transform;

	m_body = getSceneGraph()._getPhysicsWorld().newInstance<PhysicsBody>(init);

	return ErrorCode::NONE;
}

} // end namespace
