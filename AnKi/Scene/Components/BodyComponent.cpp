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
	m_body.reset(nullptr);
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(meshFilename, m_mesh));

	const Transform prevTransform = (m_body) ? m_body->getTransform() : m_trf;
	const F32 prevMass = (m_body) ? m_body->getMass() : 0.0f;

	PhysicsBodyInitInfo init;
	init.m_mass = prevMass;
	init.m_transform = prevTransform;
	init.m_shape = m_mesh->getCollisionShape();
	m_body = m_node->getSceneGraph().getPhysicsWorld().newInstance<PhysicsBody>(init);
	m_body->setUserData(this);

	m_markedForUpdate = true;
	return Error::kNone;
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
	else
	{
		ANKI_SCENE_LOGW("BodyComponent is not initialized. Ignoring setting of mass");
	}
}

Error BodyComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_markedForUpdate;
	m_markedForUpdate = false;

	if(m_body && m_body->getTransform() != m_trf)
	{
		updated = true;
		m_trf = m_body->getTransform();
	}

	return Error::kNone;
}

} // end namespace anki
