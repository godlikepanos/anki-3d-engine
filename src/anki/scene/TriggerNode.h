// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/physics/Forward.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Trigger node.
class TriggerNode : public SceneNode
{
public:
	TriggerNode(SceneGraph* scene, CString name);

	~TriggerNode();

	ANKI_USE_RESULT Error init(const CString& collisionShapeFilename);

	ANKI_USE_RESULT Error init(F32 sphereRadius);

private:
	class MoveFeedbackComponent;

	PhysicsCollisionShapePtr m_shape;
	PhysicsTriggerPtr m_trigger;
};
/// @}

} // end namespace anki