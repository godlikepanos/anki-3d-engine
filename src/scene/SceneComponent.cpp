// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
Error SceneComponent::updateReal(SceneNode& node, F32 prevTime, F32 crntTime,
	Bool& updated)
{
	reset();

	Error err = update(node, prevTime, crntTime, updated);
	if(!err && updated)
	{
		err = onUpdate(node, prevTime, crntTime);

		if(!err)
		{
			m_timestamp = getGlobTimestamp();
		}
	}

	return err;
}

} // end namespace anki
