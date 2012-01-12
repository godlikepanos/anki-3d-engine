#include "anki/scene/Movable.h"


namespace anki {


//==============================================================================
void Movable::updateWorldTransform()
{
	prevWTrf = wTrf;

	if(getParent())
	{
		if(isFlagEnabled(MF_IGNORE_LOCAL_TRANSFORM))
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

	enableFlag(MF_MOVED, prevWTrf != wTrf);
}


} // namespace anki
