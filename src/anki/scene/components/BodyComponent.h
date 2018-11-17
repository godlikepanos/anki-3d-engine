// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/physics/PhysicsBody.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Rigid body component.
class BodyComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::BODY;

	BodyComponent(PhysicsBodyPtr body)
		: SceneComponent(CLASS_TYPE)
		, m_body(body)
	{
	}

	~BodyComponent();

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_body->setTransform(trf);
	}

	PhysicsBodyPtr getPhysicsBody() const
	{
		return m_body;
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		Transform newTrf = m_body->getTransform();
		updated = newTrf != m_trf;
		m_trf = newTrf;
		return Error::NONE;
	}

private:
	PhysicsBodyPtr m_body;
	Transform m_trf = Transform::getIdentity();
};
/// @}

} // end namespace anki
