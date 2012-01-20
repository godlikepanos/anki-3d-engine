#include "anki/scene/Movable.h"
#include "anki/scene/Property.h"


namespace anki {


//==============================================================================
Movable::Movable(uint flags_, Movable* parent, PropertyMap& pmap)
	: Base(this, parent), flags(flags_)
{
	pmap.addProperty("localTransform", &lTrf, PropertyBase::PF_READ_WRITE);
	pmap.addProperty("worldTransform", &wTrf, PropertyBase::PF_READ);
}


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
