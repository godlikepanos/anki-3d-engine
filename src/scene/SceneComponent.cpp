// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/SceneComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
Timestamp SceneComponent::getGlobalTimestamp() const
{
	return m_node->getGlobalTimestamp();
}

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
			m_timestamp = getGlobalTimestamp();
		}
	}

	return err;
}

//==============================================================================
SceneGraph& SceneComponent::getSceneGraph()
{
	return m_node->getSceneGraph();
}

//==============================================================================
const SceneGraph& SceneComponent::getSceneGraph() const
{
	return m_node->getSceneGraph();
}

} // end namespace anki
