// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/events/AnimationEvent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/components/MoveComponent.h>
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

	Event::init(m_anim->getStartingTime(), m_anim->getDuration());
	m_reanimate = true;
	m_associatedNodes.emplaceBack(getAllocator(), movableSceneNode);

	return Error::NONE;
}

Error AnimationEvent::update(Second prevUpdateTime, Second crntTime)
{
	Vec3 pos;
	Quat rot;
	F32 scale = 1.0;
	m_anim->interpolate(0, crntTime, pos, rot, scale);

	Transform trf;
	trf.setOrigin(pos.xyz0());
	trf.setRotation(Mat3x4(rot));
	trf.setScale(scale);

	MoveComponent& move = m_associatedNodes[0]->getComponent<MoveComponent>();
	move.setLocalTransform(trf);

	return Error::NONE;
}

} // end namespace anki
