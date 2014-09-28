// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
Bool SceneComponent::updateReal(SceneNode& node, F32 prevTime, F32 crntTime)
{
	reset();
	Bool updated = update(node, prevTime, crntTime);
	if(updated)
	{
		onUpdate(node, prevTime, crntTime);
		m_timestamp = getGlobTimestamp();
	}
	return updated;
}

} // end namespace anki
