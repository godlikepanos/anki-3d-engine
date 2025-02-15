// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Resource/Forward.h>

namespace anki {

/// @addtogroup scene
/// @{

/// @memberof BodyComponent
enum class BodyComponentCollisionShapeType : U8
{
	kFromModelComponent, ///< Set the collision shape by looking at the ModelComponent's meshes.
	kAabb,
	kSphere,

	kCount
};

/// Rigid body component.
class BodyComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(BodyComponent)

public:
	BodyComponent(SceneNode* node);

	~BodyComponent();

	void setBoxExtend(Vec3 extend)
	{
		if(ANKI_SCENE_ASSERT(extend > 0.01f) && extend != m_box.m_extend)
		{
			m_box.m_extend = extend;
			if(m_shapeType == BodyComponentCollisionShapeType::kAabb)
			{
				m_body.reset(nullptr); // Force recreate
			}
		}
	}

	const Vec3& getBoxExtend() const
	{
		return m_box.m_extend;
	}

	void setSphereRadius(F32 radius)
	{
		if(ANKI_SCENE_ASSERT(radius > 0.01f) && radius != m_sphere.m_radius)
		{
			m_sphere.m_radius = radius;
			if(m_shapeType == BodyComponentCollisionShapeType::kSphere)
			{
				m_body.reset(nullptr); // Force recreate
			}
		}
	}

	F32 getSphereRadius() const
	{
		return m_sphere.m_radius;
	}

	void setCollisionShapeType(BodyComponentCollisionShapeType type)
	{
		if(ANKI_SCENE_ASSERT(type <= BodyComponentCollisionShapeType::kCount) && m_shapeType != type)
		{
			m_shapeType = type;
			m_body.reset(nullptr); // Force recreate
		}
	}

	void setMass(F32 mass)
	{
		if(ANKI_SCENE_ASSERT(mass >= 0.0f) && m_mass != mass)
		{
			m_mass = mass;
			m_body.reset(nullptr); // Force recreate
		}
	}

	F32 getMass() const
	{
		return m_mass;
	}

	const PhysicsBodyPtr& getPhysicsBody() const
	{
		return m_body;
	}

	void applyForce(Vec3 force, Vec3 relativePosition)
	{
		m_force = force;
		m_forcePosition = relativePosition;
	}

	void teleportTo(Vec3 position, const Mat3& rotation);

	SceneNode& getSceneNode()
	{
		return *m_node;
	}

private:
	SceneNode* m_node = nullptr;
	PhysicsBodyPtr m_body;

	PhysicsCollisionShapePtr m_collisionShape;

	class
	{
	public:
		ModelComponent* m_modelc = nullptr;
		U32 m_modelcUuid = 0;
	} m_mesh;

	class
	{
	public:
		Vec3 m_extend = Vec3(1.0f);
	} m_box;

	class
	{
	public:
		F32 m_radius = 1.0f;
	} m_sphere;

	F32 m_mass = 0.0f;

	Vec3 m_teleportPosition;
	Mat3 m_teleportedRotation;

	Vec3 m_force = Vec3(0.0f);
	Vec3 m_forcePosition = Vec3(0.0f);

	U32 m_transformVersion = 0;

	Bool m_teleported = false;

	BodyComponentCollisionShapeType m_shapeType = BodyComponentCollisionShapeType::kCount;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;
};
/// @}

} // end namespace anki
