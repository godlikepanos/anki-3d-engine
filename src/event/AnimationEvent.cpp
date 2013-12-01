#include "anki/event/AnimationEvent.h"
#include "anki/resource/Animation.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"

namespace anki {

//==============================================================================
AnimationEvent::AnimationEvent(EventManager* manager, 
	const AnimationResourcePointer& anim_, SceneNode* movableSceneNode)
	:	Event(manager, 0.0, 0.0, movableSceneNode),
		anim(anim_)
{
	ANKI_ASSERT(movableSceneNode);

	startTime = anim->getStartingTime();
	duration = anim->getDuration();

	enableBits(EF_REANIMATE, anim->getRepeat());
}

//==============================================================================
void AnimationEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	ANKI_ASSERT(getSceneNode());
	MoveComponent& move = getSceneNode()->getComponent<MoveComponent>();

	Vec3 pos;
	Quat rot;
	F32 scale = 1.0;
	anim->interpolate(0, crntTime, pos, rot, scale);

	Transform trf;
	trf.setOrigin(pos);
	trf.setRotation(Mat3(rot));
	trf.setScale(scale);
	move.setLocalTransform(trf);
}

} // end namespace anki
