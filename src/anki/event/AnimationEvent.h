// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/Event.h>
#include <anki/resource/Animation.h>

namespace anki
{

/// @addtogroup event
/// @{

/// Event controled by animation resource
class AnimationEvent : public Event
{
public:
	AnimationEvent(EventManager* manager);

	ANKI_USE_RESULT Error init(const AnimationResourcePtr& anim, SceneNode* movableSceneNode);

	/// Implements Event::update
	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime) override;

private:
	AnimationResourcePtr m_anim;
};
/// @}

} // end namespace anki
