#include "anki/scene/SceneComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
Bool SceneComponent::updateReal(SceneNode& node, F32 prevTime, F32 crntTime,
	UpdateType updateType)
{
	Bool updated = update(node, prevTime, crntTime, updateType);
	if(updated)
	{
		node.componentUpdated(*this, updateType);
		timestamp = getGlobTimestamp();
	}
	return updated;
}

} // end namespace anki
