// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

BodyComponent::BodyComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
	node->setIgnoreParentTransform(true);
}

BodyComponent::~BodyComponent()
{
}

void BodyComponent::loadMeshResource(CString meshFilename)
{
	CpuMeshResourcePtr rsrc;
	const Error err = getExternalSubsystems(*m_node).m_resourceManager->loadResource(meshFilename, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load mesh");
		return;
	}

	m_mesh = std::move(rsrc);

	const Transform prevTransform = m_node->getWorldTransform();
	const F32 prevMass = (m_body) ? m_body->getMass() : 0.0f;

	PhysicsBodyInitInfo init;
	init.m_mass = prevMass;
	init.m_transform = prevTransform;
	init.m_shape = m_mesh->getCollisionShape();
	m_body = getExternalSubsystems(*m_node).m_physicsWorld->newInstance<PhysicsBody>(init);
	m_body->setUserData(this);
	m_body->setTransform(m_node->getWorldTransform());

	m_dirty = true;
}

CString BodyComponent::getMeshResourceFilename() const
{
	return (m_mesh.isCreated()) ? m_mesh->getFilename() : CString();
}

void BodyComponent::setMass(F32 mass)
{
	if(!ANKI_SCENE_ASSERT(mass >= 0.0f))
	{
		mass = 0.0f;
	}

	if(m_body.isCreated())
	{
		if((m_body->getMass() == 0.0f && mass != 0.0f) || (m_body->getMass() != 0.0f && mass == 0.0f))
		{
			// Will become from static to dynamic or the opposite, re-create the body

			const Transform& prevTransform = m_body->getTransform();
			PhysicsBodyInitInfo init;
			init.m_transform = prevTransform;
			init.m_mass = mass;
			init.m_shape = m_mesh->getCollisionShape();
			m_body = getExternalSubsystems(*m_node).m_physicsWorld->newInstance<PhysicsBody>(init);
			m_body->setUserData(this);
			m_body->setTransform(m_node->getWorldTransform());

			m_dirty = true;
		}
		else
		{
			m_body->setMass(mass);
		}
	}
	else
	{
		ANKI_SCENE_LOGW("BodyComponent is not initialized. Ignoring setting of mass");
	}
}

Error BodyComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty;
	m_dirty = false;

	if(m_body && m_body->getTransform() != info.m_node->getWorldTransform())
	{
		updated = true;
		info.m_node->setLocalTransform(m_body->getTransform());
	}

	return Error::kNone;
}

} // end namespace anki
