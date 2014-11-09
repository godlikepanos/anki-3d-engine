// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/MoveEvent.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
Error MoveEvent::create(EventManager* manager, F32 startTime, F32 duration,
	SceneNode* node, const MoveEventData& data)
{
	ANKI_ASSERT(node);
	Error err = Event::create(manager, startTime, duration, node);
	if(err) return err;

	*static_cast<MoveEventData*>(this) = data;

	const MoveComponent& move = node->getComponent<MoveComponent>();

	m_originalPos = move.getLocalTransform().getOrigin();

	for(U i = 0; i < 3; i++)
	{
		m_newPos[i] = randRange(m_posMin[i], m_posMax[i]);
	}

	m_newPos[3] = 0.0;
	m_newPos += move.getLocalTransform().getOrigin();

	return err;
}

//==============================================================================
Error MoveEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	SceneNode* node = getSceneNode();
	ANKI_ASSERT(node);

	MoveComponent& move = node->getComponent<MoveComponent>();

	Transform trf = move.getLocalTransform();

	F32 factor = sin(getDelta(crntTime) * getPi<F32>());

	trf.getOrigin() = linearInterpolate(m_originalPos, m_newPos, factor);

	move.setLocalTransform(trf);

	return ErrorCode::NONE;
}

} // end namespace anki
