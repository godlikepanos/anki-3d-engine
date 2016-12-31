// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/AnimationEvent.h>
#include <anki/resource/Animation.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/MoveComponent.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

AnimationEvent::AnimationEvent(EventManager* manager)
	: Event(manager)
{
}

Error AnimationEvent::init(const AnimationResourcePtr& anim, SceneNode* movableSceneNode)
{
	ANKI_ASSERT(movableSceneNode);
	m_anim = anim;

	Event::init(m_anim->getStartingTime(),
		m_anim->getDuration(),
		movableSceneNode,
		m_anim->getRepeat() ? Flag::REANIMATE : Flag::NONE);

	return ErrorCode::NONE;
}

Error AnimationEvent::update(F32 prevUpdateTime, F32 crntTime)
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

	return ErrorCode::NONE;
}

} // end namespace anki
