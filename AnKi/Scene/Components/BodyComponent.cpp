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

void BodyComponent::teleportTo(Vec3 position, const Mat3& rotation)
{
	m_teleportPosition = position;
	m_teleportedRotation = rotation;
	m_teleported = true;

	// Set those just to be sure
	m_node->setLocalOrigin(position.xyz0());
	m_node->setLocalRotation(Mat3x4(Vec3(0.0f), rotation, Vec3(1.0f)));
}

Error BodyComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(m_shapeType == BodyComponentCollisionShapeType::kCount
	   || (m_shapeType == BodyComponentCollisionShapeType::kFromModelComponent && !m_mesh.m_modelc))
	{
		ANKI_ASSERT(!m_body);
		return Error::kNone;
	}

	const Bool shapeDirty =
		!m_body.isCreated()
		|| (m_shapeType == BodyComponentCollisionShapeType::kFromModelComponent && m_mesh.m_modelc->getUuid() != m_mesh.m_modelcUuid);
	if(shapeDirty)
	{
		updated = true;

		PhysicsBodyInitInfo init;
		init.m_mass = m_mass;
		if(m_teleported)
		{
			init.m_transform = Transform(m_teleportPosition, m_teleportedRotation, m_node->getLocalTransform().getScale().xyz());
			m_teleported = false; // Cancel teleportation since the body was re-created
		}
		else
		{
			init.m_transform = m_node->getLocalTransform();
		}

		if(m_mass == 0.0f)
		{
			init.m_layer = PhysicsLayer::kStatic;
		}
		else
		{
			init.m_layer = PhysicsLayer::kMoving;
		}

		if(m_shapeType == BodyComponentCollisionShapeType::kFromModelComponent)
		{
			m_mesh.m_modelcUuid = m_mesh.m_modelc->getUuid();

			ANKI_CHECK(m_mesh.m_modelc->getModelResource()->getModelPatches()[0].getMesh()->getOrCreateCollisionShape(
				m_mass == 0.0f, kMaxLodCount - 1, m_collisionShape));
		}
		else if(m_shapeType == BodyComponentCollisionShapeType::kAabb)
		{
			m_collisionShape = PhysicsWorld::getSingleton().newBoxCollisionShape(m_box.m_extend);
		}
		else
		{
			ANKI_ASSERT(m_shapeType == BodyComponentCollisionShapeType::kSphere);
			m_collisionShape = PhysicsWorld::getSingleton().newSphereCollisionShape(m_sphere.m_radius);
		}

		init.m_shape = m_collisionShape.get();

		m_body = PhysicsWorld::getSingleton().newPhysicsBody(init);
		m_body->setUserData(this);
	}

	if(m_teleported)
	{
		updated = true;
		m_teleported = false;
		m_body->setPositionAndRotation(m_teleportPosition, m_teleportedRotation);
	}

	U32 version;
	Transform bodyTrf = m_body->getTransform(&version);
	if(version != m_transformVersion)
	{
		m_transformVersion = version;
		updated = true;
		info.m_node->setLocalTransform(m_body->getTransform());
	}

	if(m_force.getLengthSquared() > 0.0f)
	{
		if(m_forcePosition != 0.0f)
		{
			m_body->applyForce(m_force, m_forcePosition);
		}
		else
		{
			m_body->applyForce(m_force);
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

	if(added && m_mesh.m_modelc == nullptr)
	{
		m_mesh.m_modelc = static_cast<ModelComponent*>(other);
	}
	else if(!added && m_mesh.m_modelc == other)
	{
		m_mesh.m_modelc = nullptr;
	}
}

} // end namespace anki
