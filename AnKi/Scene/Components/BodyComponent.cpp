// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/CpuMeshResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Resource/MeshResource.h>

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
	m_node->setLocalOrigin(position);
	m_node->setLocalRotation(rotation);
}

void BodyComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool meshIsValid = m_mesh.m_meshc && m_mesh.m_meshc->hasMeshResource();
	if(m_shapeType == BodyComponentCollisionShapeType::kCount || (m_shapeType == BodyComponentCollisionShapeType::kFromMeshComponent && !meshIsValid))
	{
		// It's invalid, return
		ANKI_ASSERT(!m_body);
		return;
	}

	const Bool shapeDirty = !m_body.isCreated()
							|| (m_shapeType == BodyComponentCollisionShapeType::kFromMeshComponent
								&& m_mesh.m_meshc->getMeshResource().getUuid() != m_mesh.m_meshResourceUuid);
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

		const Bool isStatic = m_mass == 0.0f;
		if(isStatic)
		{
			init.m_layer = PhysicsLayer::kStatic;
		}
		else
		{
			init.m_layer = PhysicsLayer::kMoving;
		}

		if(m_shapeType == BodyComponentCollisionShapeType::kFromMeshComponent)
		{
			const MeshResource& meshResource = m_mesh.m_meshc->getMeshResource();
			m_mesh.m_meshResourceUuid = meshResource.getUuid();

			if(meshResource.getOrCreateCollisionShape(isStatic, kMaxLodCount - 1, m_collisionShape))
			{
				ANKI_SCENE_LOGE("BodyComponent::update failed due to error");
				return;
			}
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

	if(m_force.lengthSquared() > 0.0f)
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
}

void BodyComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	if(other->getType() != SceneComponentType::kMesh)
	{
		return;
	}

	if(added && m_mesh.m_meshc == nullptr)
	{
		m_mesh.m_meshc = static_cast<MeshComponent*>(other);
		if(m_shapeType == BodyComponentCollisionShapeType::kFromMeshComponent)
		{
			cleanup();
		}
	}
	else if(!added && m_mesh.m_meshc == other)
	{
		m_mesh.m_meshc = nullptr;
		if(m_shapeType == BodyComponentCollisionShapeType::kFromMeshComponent)
		{
			cleanup();
		}
	}
}

void BodyComponent::cleanup()
{
	m_body.reset(nullptr);
	m_collisionShape.reset(nullptr);
}

} // end namespace anki
