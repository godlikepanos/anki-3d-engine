// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/JitterMoveEvent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Util/Functions.h>

namespace anki {

Error JitterMoveEvent::init(Second startTime, Second duration, SceneNode* node)
{
	ANKI_ASSERT(node);
	Event::init(startTime, duration);
	m_associatedNodes.emplaceBack(getMemoryPool(), node);

	const MoveComponent& move = node->getFirstComponentOfType<MoveComponent>();

	m_originalPos = move.getLocalTransform().getOrigin();

	return Error::kNone;
}

void JitterMoveEvent::setPositionLimits(const Vec4& posMin, const Vec4& posMax)
{
	for(U i = 0; i < 3; i++)
	{
		m_newPos[i] = getRandomRange(posMin[i], posMax[i]);
	}

	m_newPos[3] = 0.0;
	m_newPos += m_originalPos;
}

Error JitterMoveEvent::update([[maybe_unused]] Second prevUpdateTime, Second crntTime)
{
	SceneNode* node = m_associatedNodes[0];

	MoveComponent& move = node->getFirstComponentOfType<MoveComponent>();

	Transform trf = move.getLocalTransform();

	F32 factor = F32(sin(getDelta(crntTime) * kPi));

	trf.getOrigin() = linearInterpolate(m_originalPos, m_newPos, factor);

	move.setLocalTransform(trf);

	return Error::kNone;
}

} // end namespace anki
