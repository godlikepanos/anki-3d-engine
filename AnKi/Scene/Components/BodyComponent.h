// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

	CString getMeshResourceFilename() const;

	void setMass(F32 mass);

	F32 getMass() const
	{
		return (m_body) ? m_body->getMass() : 0.0f;
	}

	PhysicsBodyPtr getPhysicsBody() const
	{
		return m_body;
	}

	Bool isEnabled() const
	{
		return m_mesh.isCreated();
	}

	void teleportTo(const Transform& trf)
	{
		m_body->setTransform(trf);
	}

private:
	SceneNode* m_node = nullptr;
	CpuMeshResourcePtr m_mesh;
	PhysicsBodyPtr m_body;
	Bool m_dirty = true;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
