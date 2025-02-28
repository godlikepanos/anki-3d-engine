// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Resource/AnimationResource.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Event controled by animation resource.
class AnimationEvent : public Event
{
public:
	AnimationEvent(CString animationFilename, CString channel, SceneNode* movableSceneNode);

	void update(Second prevUpdateTime, Second crntTime) override;

private:
	AnimationResourcePtr m_anim;
	U32 m_channelIndex = 0;
};
/// @}

} // end namespace anki
