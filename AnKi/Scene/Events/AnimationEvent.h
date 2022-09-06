// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Resource/AnimationResource.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Event controled by animation resource.
class AnimationEvent : public Event
{
public:
	AnimationEvent(EventManager* manager);

	Error init(CString animationFilename, CString channel, SceneNode* movableSceneNode);

	/// Implements Event::update
	Error update(Second prevUpdateTime, Second crntTime) override;

private:
	AnimationResourcePtr m_anim;
	U32 m_channelIndex = 0;
};
/// @}

} // end namespace anki
