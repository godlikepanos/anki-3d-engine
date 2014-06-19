// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/AnimationEvent.h"
#include "anki/resource/Animation.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"

namespace anki {

//==============================================================================
AnimationEvent::AnimationEvent(EventManager* manager, 
	const AnimationResourcePointer& anim, SceneNode* movableSceneNode)
	:	Event(manager, 0.0, 0.0, movableSceneNode),
		m_anim(anim)
{
	ANKI_ASSERT(movableSceneNode);

	startTime = m_anim->getStartingTime();
	duration = m_anim->getDuration();

	enableBits(EF_REANIMATE, m_anim->getRepeat());
}

//==============================================================================
void AnimationEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	ANKI_ASSERT(getSceneNode());
	MoveComponent& move = getSceneNode()->getComponent<MoveComponent>();

	Vec3 pos;
	Quat rot;
	F32 scale = 1.0;
	m_anim->interpolate(0, crntTime, pos, rot, scale);

	Transform trf;
	trf.setOrigin(pos.xyz0());
	trf.setRotation(Mat3x4(rot));
	trf.setScale(scale);
	move.setLocalTransform(trf);
}

} // end namespace anki
