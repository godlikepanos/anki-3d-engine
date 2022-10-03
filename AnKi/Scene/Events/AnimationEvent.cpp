// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/AnimationEvent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

AnimationEvent::AnimationEvent(EventManager* manager)
	: Event(manager)
{
}

Error AnimationEvent::init(CString animationFilename, CString channelName, SceneNode* movableSceneNode)
{
	ANKI_ASSERT(movableSceneNode);
	ANKI_CHECK(getSceneGraph().getResourceManager().loadResource(animationFilename, m_anim));

	m_channelIndex = 0;
	for(const AnimationChannel& channel : m_anim->getChannels())
	{
		if(channel.m_name == channelName)
		{
			break;
		}
		++m_channelIndex;
	}

	if(m_channelIndex == m_anim->getChannels().getSize())
	{
		ANKI_SCENE_LOGE("Can't initialize AnimationEvent. Channel not found: %s", channelName.cstr());
		return Error::kUserData;
	}

	Event::init(m_anim->getStartingTime(), m_anim->getDuration());
	m_reanimate = true;
	m_associatedNodes.emplaceBack(getMemoryPool(), movableSceneNode);

	return Error::kNone;
}

Error AnimationEvent::update([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	Vec3 pos;
	Quat rot;
	F32 scale = 1.0;
	m_anim->interpolate(m_channelIndex, crntTime, pos, rot, scale);

	Transform trf;
	trf.setOrigin(pos.xyz0());
	trf.setRotation(Mat3x4(Vec3(0.0f), rot));
	trf.setScale(scale);

	MoveComponent& move = m_associatedNodes[0]->getFirstComponentOfType<MoveComponent>();
	move.setLocalTransform(trf);

	return Error::kNone;
}

} // end namespace anki
