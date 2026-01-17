// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Resource/Forward.h>

namespace anki {

enum class BodyComponentCollisionShapeType : U8
{
	kFromMeshComponent, // Set the collision shape by looking at the MeshComponent's mesh.
	kAabb,
	kSphere,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(BodyComponentCollisionShapeType)

inline constexpr Array<const Char*, U32(BodyComponentCollisionShapeType::kCount)> kBodyComponentCollisionShapeTypeNames = {"Mesh Component", "AABB",
																														   "Sphere"};

// Rigid body component.
class BodyComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(BodyComponent)

public:
	BodyComponent(const SceneComponentInitInfo& init);

	~BodyComponent();

	void setCollisionShapeType(BodyComponentCollisionShapeType type)
	{
		if(ANKI_EXPECT(type <= BodyComponentCollisionShapeType::kCount) && m_shapeType != type)
		{
			m_shapeType = type;
			cleanup(); // Force recreate
		}
	}

	BodyComponentCollisionShapeType getCollisionShapeType() const
	{
		return m_shapeType;
	}

	void setBoxExtend(Vec3 extend)
	{
		if(ANKI_EXPECT(extend > 0.01f) && extend != m_box.m_extend)
		{
			m_box.m_extend = extend;
			if(m_shapeType == BodyComponentCollisionShapeType::kAabb)
			{
				cleanup(); // Force recreate
			}
		}
	}

	const Vec3& getBoxExtend() const
	{
		return m_box.m_extend;
	}

	void setSphereRadius(F32 radius)
	{
		if(ANKI_EXPECT(radius > 0.01f) && radius != m_sphere.m_radius)
		{
			m_sphere.m_radius = radius;
			if(m_shapeType == BodyComponentCollisionShapeType::kSphere)
			{
				cleanup(); // Force recreate
			}
		}
	}

	F32 getSphereRadius() const
	{
		return m_sphere.m_radius;
	}

	void setMass(F32 mass)
	{
		if(ANKI_EXPECT(mass >= 0.0f) && m_mass != mass)
		{
			m_mass = mass;
			cleanup(); // Force recreate
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

	SceneNode& getSceneNode()
	{
		return *m_node;
	}

	Bool isValid() const;

private:
	SceneNode* m_node = nullptr;
	PhysicsBodyPtr m_body;
	Vec3 m_creationScale = Vec3(0.0f); // Track the scale the body was created with

	PhysicsCollisionShapePtr m_collisionShape;

	class
	{
	public:
		MeshComponent* m_meshc = nullptr;
		U32 m_meshResourceUuid = 0;
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

	Vec3 m_force = Vec3(0.0f);
	Vec3 m_forcePosition = Vec3(0.0f);

	U32 m_transformVersion = 0;

	BodyComponentCollisionShapeType m_shapeType = BodyComponentCollisionShapeType::kAabb;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;

	Error serialize(SceneSerializer& serializer) override;

	void cleanup();
};

} // end namespace anki
