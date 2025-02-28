// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/JitterMoveEvent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Util/Functions.h>

namespace anki {

JitterMoveEvent::JitterMoveEvent(Second startTime, Second duration, SceneNode* node)
	: Event(startTime, duration)
{
	if(!ANKI_EXPECT(node))
	{
		markForDeletion();
		return;
	}

	m_associatedNodes.emplaceBack(node);
	m_originalPos = node->getWorldTransform().getOrigin().xyz();
}

void JitterMoveEvent::setPositionLimits(Vec3 posMin, Vec3 posMax)
{
	for(U i = 0; i < 3; i++)
	{
		m_newPos[i] = getRandomRange(posMin[i], posMax[i]);
	}

	m_newPos[3] = 0.0;
	m_newPos += m_originalPos;
}

void JitterMoveEvent::update([[maybe_unused]] Second prevUpdateTime, Second crntTime)
{
	SceneNode* node = m_associatedNodes[0];

	Transform trf = node->getLocalTransform();

	const F32 factor = F32(sin(getDelta(crntTime) * kPi));

	trf.setOrigin(linearInterpolate(m_originalPos, m_newPos, factor));

	node->setLocalTransform(trf);
}

} // end namespace anki
