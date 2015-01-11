// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_BODY_COMPONENT_H
#define ANKI_SCENE_BODY_COMPONENT_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/physics/PhysicsBody.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Rigid body component.
class BodyComponent: public SceneComponent
{
public:
	BodyComponent(SceneNode* node, PhysicsBody* body)
	:	SceneComponent(Type::BODY, node), 
		m_body(body)
	{
		ANKI_ASSERT(m_body);
	}

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_body->setTransform(trf);
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override
	{
		m_trf = m_body->getTransform(updated);
		return ErrorCode::NONE;
	}
	/// @}

	static constexpr Type getClassType()
	{
		return Type::BODY;
	}

private:
	PhysicsBody* m_body;
	Transform m_trf;
};
/// @}

} // end namespace anki

#endif

