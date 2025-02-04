// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/ModelComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Resource/ModelResource.h>

namespace anki {

BodyComponent::BodyComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
	, m_node(node)
{
	node->setIgnoreParentTransform(true);
}

BodyComponent::~BodyComponent()
{
}

void BodyComponent::removeBody()
{
	m_shapeType = ShapeType::kCount;
	m_body.reset(nullptr);
	m_teleported = false;
	m_force = Vec3(0.0f);
	m_mesh.reset(nullptr);
	m_collisionShape.reset(nullptr);
}

void BodyComponent::loadMeshResource(CString meshFilename)
{
	CpuMeshResourcePtr rsrc;
	if(ResourceManager::getSingleton().loadResource(meshFilename, rsrc))
	{
		ANKI_SCENE_LOGE("Failed to load mesh");
		return;
	}

	removeBody();
	m_shapeType = ShapeType::kMesh;
	m_mesh = std::move(rsrc);
}

void BodyComponent::setMeshFromModelComponent(U32 patchIndex)
{
	if(!ANKI_SCENE_ASSERT(m_modelc))
	{
		return;
	}

	if(!ANKI_SCENE_ASSERT(patchIndex < m_modelc->getModelResource()->getModelPatches().getSize()))
	{
		return;
	}

	loadMeshResource(m_modelc->getModelResource()->getModelPatches()[patchIndex].getMesh()->getFilename());
}

CString BodyComponent::getMeshResourceFilename() const
{
	return (m_mesh.isCreated()) ? m_mesh->getFilename() : CString();
}

void BodyComponent::setBoxCollisionShape(const Vec3& extend)
{
	if(ANKI_SCENE_ASSERT(extend > 0.0f))
	{
		removeBody();
		m_shapeType = ShapeType::kAabb;
		m_boxExtend = extend;
		m_collisionShape = PhysicsWorld::getSingleton().newInstance<PhysicsBox>(extend);
	}
}

void BodyComponent::setMass(F32 mass)
{
	if(!ANKI_SCENE_ASSERT(mass >= 0.0f))
	{
		return;
	}

	if(m_mass != mass)
	{
		m_mass = mass;
		m_body.reset(nullptr);
	}
}

void BodyComponent::teleportTo(const Transform& trf)
{
	m_teleportTrf = trf;
	m_teleported = true;

	m_node->setLocalTransform(trf); // Set that just to be sure
}

Error BodyComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(m_shapeType == ShapeType::kCount)
	{
		return Error::kNone;
	}

	const Bool shapeDirty = !m_body.isCreated();
	if(shapeDirty)
	{
		updated = true;

		PhysicsBodyInitInfo init;
		init.m_mass = m_mass;
		init.m_transform = (m_teleported) ? m_teleportTrf : m_node->getWorldTransform();

		if(m_shapeType == ShapeType::kMesh)
		{
			init.m_shape = m_mesh->getOrCreateCollisionShape(m_mass == 0.0f);
		}
		else
		{
			init.m_shape = m_collisionShape;
		}

		m_body = PhysicsWorld::getSingleton().newInstance<PhysicsBody>(init);
		m_body->setUserData(this);

		m_teleported = false; // Cancel teleportation since the body was re-created
	}

	if(m_body && m_teleported)
	{
		updated = true;
		m_teleported = false;
		m_body->setTransform(m_teleportTrf);
	}

	if(m_body && m_body->getTransform() != info.m_node->getWorldTransform())
	{
		updated = true;
		info.m_node->setLocalTransform(m_body->getTransform());
	}

	if(m_force.getLengthSquared() > 0.0f)
	{
		if(m_body)
		{
			m_body->applyForce(m_force, m_forcePosition);
		}

		m_force = Vec3(0.0f);
	}

	return Error::kNone;
}

void BodyComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	if(other->getType() != SceneComponentType::kModel)
	{
		return;
	}

	if(added && m_modelc == nullptr)
	{
		m_modelc = static_cast<ModelComponent*>(other);
	}
	else if(!added && m_modelc == other)
	{
		m_modelc = nullptr;
	}
}

} // end namespace anki
