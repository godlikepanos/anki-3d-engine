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

ANKI_SVAR(BodiesCreated, StatCategory::kScene, "Bodies created", StatFlag::kNone)

BodyComponent::BodyComponent(const SceneComponentInitInfo& init)
	: SceneComponent(kClassType, init)
	, m_node(init.m_node)
{
	// A body lives in world space, so its node must ignore the parent's transform (after this, world == local). If the node currently has a parent,
	// bake the real world transform into the local first, otherwise enabling ignore-parent would snap the node to its parent-relative transform. Do
	// this here (not in update()) because right now the world transform is still valid; inside update() it's stale (MoveComponent refreshes it and
	// runs after us).
	if(m_node->getParent())
	{
		m_node->setLocalTransform(m_node->getWorldTransform());
	}

	m_node->setIgnoreParentTransform(true);
}

BodyComponent::~BodyComponent()
{
}

Bool BodyComponent::isValid() const
{
	return m_shapeType != BodyComponentCollisionShapeType::kFromMeshComponent || (m_mesh.m_meshc && m_mesh.m_meshc->isValid());
}

void BodyComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isValid())
	{
		// It's invalid, return
		ANKI_ASSERT(!m_body);
		return;
	}

	// Find if the shape is dirty
	Bool shapeDirty = false;
	if(!m_body.isCreated())
	{
		shapeDirty = true;
	}

	auto isCollisionShapeDirty = [this]() {
		if(m_mesh.m_meshc->getMeshComponentType() == MeshComponentType::kMeshResource)
		{
			return m_mesh.m_meshc->getMeshResource().getUuid() != m_mesh.m_meshResourceUuid;
		}
		else
		{
			return m_collisionShape.tryGet() != m_mesh.m_meshc->getPrimitiveCollisionShape();
		}
	};

	if(!shapeDirty && (m_shapeType == BodyComponentCollisionShapeType::kFromMeshComponent && isCollisionShapeDirty()))
	{
		shapeDirty = true;
	}

	if(!shapeDirty && info.m_node->getLocalScale() != m_creationScale)
	{
		// Scale is baked into the body and it changed, recreate the body
		shapeDirty = true;
	}

	if(shapeDirty)
	{
		// Re-create the body

		updated = true;

		PhysicsBodyInitInfo init;
		init.m_mass = m_mass;
		// The node ignores its parent so local == world. Use local: it's the fresh value here, because MoveComponent (which recomputes the world
		// transform via updateTransform()) updates after us.
		init.m_transform = m_node->getLocalTransform();

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
			if(m_mesh.m_meshc->getMeshComponentType() == MeshComponentType::kMeshResource)
			{
				const MeshResource& meshResource = m_mesh.m_meshc->getMeshResource();
				m_mesh.m_meshResourceUuid = meshResource.getUuid();

				if(meshResource.getOrCreateCollisionShape(isStatic, kMaxLodCount - 1, m_collisionShape))
				{
					ANKI_SCENE_LOGE("BodyComponent::update failed due to error");
					return;
				}
			}
			else
			{
				m_collisionShape.reset(m_mesh.m_meshc->getPrimitiveCollisionShape());
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
		m_creationScale = init.m_transform.getScale().xyz;

		g_svarBodiesCreated.increment(1);

		// Adopt the body's current version so next frame's readback doesn't echo our own creation transform back into the node.
		m_body->getTransform(&m_transformVersion);
	}
	else
	{
		// Body doesn't need re-creation

		if(info.m_node->isLocalTransformDirty())
		{
			// Someone moved the body, teleport it to its new position
			updated = true;
			m_body->setPositionAndRotation(info.m_node->getLocalOrigin(), info.m_node->getLocalRotation());

			// Swallow the version bump caused by our own teleport so we don't read it back next frame.
			m_body->getTransform(&m_transformVersion);
		}
		else
		{
			// Check if the body moved on its own (physics) and follow it
			U32 version;
			const Transform bodyTrf = m_body->getTransform(&version);
			if(version != m_transformVersion)
			{
				m_transformVersion = version;
				updated = true;
				info.m_node->setLocalTransform(bodyTrf);
			}
		}
	}

	// Apply force
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

Error BodyComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_box.m_extend, 1);
	ANKI_SERIALIZE(m_sphere.m_radius, 1);
	ANKI_SERIALIZE(m_mass, 1);
	ANKI_SERIALIZE(m_shapeType, 1);
	return Error::kNone;
}

} // end namespace anki
