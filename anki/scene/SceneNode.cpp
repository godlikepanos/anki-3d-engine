#include "anki/scene/SceneNode.h"


namespace anki {


//==============================================================================
void SceneNode::updateWorldTransform()
{
	prevWTrf = wTrf;

	if(getParent())
	{
		if(isFlagEnabled(SNF_INHERIT_PARENT_TRANSFORM))
		{
			wTrf = getParent()->getWorldTransform();
		}
		else
		{
			wTrf = Transform::combineTransformations(
				getParent()->getWorldTransform(), lTrf);
		}
	}
	else // else copy
	{
		wTrf = lTrf;
	}
}


} // end namespace
