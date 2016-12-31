// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/JitterMoveEvent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/MoveComponent.h>
#include <anki/util/Functions.h>

namespace anki
{

Error JitterMoveEvent::init(F32 startTime, F32 duration, SceneNode* node)
{
	ANKI_ASSERT(node);
	Event::init(startTime, duration, node);

	const MoveComponent& move = node->getComponent<MoveComponent>();

	m_originalPos = move.getLocalTransform().getOrigin();

	return ErrorCode::NONE;
}

void JitterMoveEvent::setPositionLimits(const Vec4& posMin, const Vec4& posMax)
{
	for(U i = 0; i < 3; i++)
	{
		m_newPos[i] = randRange(posMin[i], posMax[i]);
	}

	m_newPos[3] = 0.0;
	m_newPos += m_originalPos;
}

Error JitterMoveEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	SceneNode* node = getSceneNode();
	ANKI_ASSERT(node);

	MoveComponent& move = node->getComponent<MoveComponent>();

	Transform trf = move.getLocalTransform();

	F32 factor = sin(getDelta(crntTime) * PI);

	trf.getOrigin() = linearInterpolate(m_originalPos, m_newPos, factor);

	move.setLocalTransform(trf);

	return ErrorCode::NONE;
}

} // end namespace anki
