// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/physics/PhysicsBody.h>
#include <anki/resource/Forward.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Rigid body component.
class BodyComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(BodyComponent)

public:
	BodyComponent(SceneNode* node);

	~BodyComponent();

	ANKI_USE_RESULT Error loadMeshResource(CString meshFilename);

	CString getMeshResourceFilename() const;

	void setMass(F32 mass);

	F32 getMass() const
	{
		return (m_body) ? m_body->getMass() : 0.0f;
	}

	void setWorldTransform(const Transform& trf)
	{
		if(m_body)
		{
			m_body->setTransform(trf);
		}
		else
		{
			m_trf = trf;
		}
	}

	Transform getWorldTransform() const
	{
		return (m_body) ? m_body->getTransform() : m_trf;
	}

	PhysicsBodyPtr getPhysicsBody() const
	{
		return m_body;
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override;

	Bool isEnabled() const
	{
		return m_mesh.isCreated();
	}

private:
	SceneNode* m_node = nullptr;
	CpuMeshResourcePtr m_mesh;
	PhysicsBodyPtr m_body;
	Transform m_trf = Transform::getIdentity();
	Bool m_markedForUpdate = true;
};
/// @}

} // end namespace anki
