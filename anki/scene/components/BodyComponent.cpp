// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/CpuMeshResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

BodyComponent::BodyComponent(SceneNode* node)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
{
}

BodyComponent::~BodyComponent()
{
}

void BodyComponent::setMeshResource(CString meshFilename)
{
	const Error err = m_node->getSceneGraph().getResourceManager().loadResource(meshFilename, m_mesh);
	if(err)
	{
		ANKI_SCENE_LOGE("Couldn't initialize the body component. Error ignored");
		return;
	}

	PhysicsBodyInitInfo init;
	init.m_mass = 1.0f;
	init.m_shape = m_mesh->getCollisionShape();
	m_body = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
	m_body->setUserData(m_node);
}

} // end namespace anki
