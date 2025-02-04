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

/// Rigid body component.
class BodyComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(BodyComponent)

public:
	BodyComponent(SceneNode* node);

	~BodyComponent();

	void loadMeshResource(CString meshFilename);

	void setMeshFromModelComponent(U32 patchIndex = 0);

	CString getMeshResourceFilename() const;

	void setBoxCollisionShape(const Vec3& extend);

	void removeBody();

	void setMass(F32 mass);

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

	void teleportTo(const Transform& trf);

private:
	enum class ShapeType : U8
	{
		kMesh,
		kAabb,
		kSphere,
		kCount
	};

	SceneNode* m_node = nullptr;
	PhysicsBodyPtr m_body;

	ModelComponent* m_modelc = nullptr;
	CpuMeshResourcePtr m_mesh;

	PhysicsCollisionShapePtr m_collisionShape;

	union
	{
		Vec3 m_boxExtend = Vec3(0.0f);
		F32 m_sphereRadius;
	};

	F32 m_mass = 0.0f;

	Transform m_teleportTrf;

	Vec3 m_force = Vec3(0.0f);
	Vec3 m_forcePosition = Vec3(0.0f);

	Bool m_teleported = false;

	ShapeType m_shapeType = ShapeType::kCount;

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;
};
/// @}

} // end namespace anki
