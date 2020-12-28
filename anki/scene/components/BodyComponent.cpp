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

ANKI_SCENE_COMPONENT_STATICS(BodyComponent)

BodyComponent::BodyComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

BodyComponent::~BodyComponent()
{
}

Error BodyComponent::loadMeshResource(CString meshFilename)
{
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(meshFilename, m_mesh));

	const Transform prevTransform = (m_body) ? m_body->getTransform() : Transform::getIdentity();
	const F32 prevMass = (m_body) ? m_body->getMass() : 0.0f;

	PhysicsBodyInitInfo init;
	init.m_mass = prevMass;
	init.m_transform = prevTransform;
	init.m_shape = m_mesh->getCollisionShape();
	m_body = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
	m_body->setUserData(this);

	m_markedForUpdate = true;
	return Error::NONE;
}

CString BodyComponent::getMeshResourceFilename() const
{
	return (m_mesh.isCreated()) ? m_mesh->getFilename() : CString();
}

void BodyComponent::setMass(F32 mass)
{
	if(mass < 0.0f)
	{
		ANKI_SCENE_LOGW("Attempting to set a negative mass");
		mass = 0.0f;
	}

	if(m_body.isCreated())
	{
		if((m_body->getMass() == 0.0f && mass != 0.0f) || (m_body->getMass() != 0.0f && mass == 0.0f))
		{
			// Will become from static to dynamic or the opposite, re-create the body

			const Transform prevTransform = getWorldTransform();
			PhysicsBodyInitInfo init;
			init.m_transform = prevTransform;
			init.m_mass = mass;
			init.m_shape = m_mesh->getCollisionShape();
			m_body = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
			m_body->setUserData(this);

			m_markedForUpdate = true;
		}
		else
		{
			m_body->setMass(mass);
		}
	}
}

Error BodyComponent::update(SceneNode& node, Second, Second, Bool& updated)
{
	updated = m_markedForUpdate;
	m_markedForUpdate = false;

	if(m_body && m_body->getTransform() != m_trf)
	{
		updated = true;
		m_trf = m_body->getTransform();
	}

	return Error::NONE;
}

} // end namespace anki
