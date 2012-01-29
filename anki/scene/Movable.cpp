#include "anki/scene/Movable.h"
#include "anki/scene/Property.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
Movable::Movable(uint flags_, Movable* parent, PropertyMap& pmap)
	: Base(this, parent), flags(flags_)
{
	pmap.addNewProperty(new ReadWritePointerProperty<Transform>("localTransform", &lTrf));
	pmap.addNewProperty(new ReadPointerProperty<Transform>("worldTransform", &wTrf));
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

	BOOST_FOREACH(Movable* child, getChildren())
	{
		child->updateWorldTransform();
	}
}


} // namespace anki
