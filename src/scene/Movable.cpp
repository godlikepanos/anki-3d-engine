#include "anki/scene/Movable.h"
#include "anki/scene/Property.h"

namespace anki {

//==============================================================================
Movable::Movable(uint flags_, Movable* parent, PropertyMap& pmap)
	: Base(this, parent), flags(flags_)
{
	pmap.addNewProperty(
		new ReadWritePointerProperty<Transform>("localTransform", &lTrf));
	pmap.addNewProperty(
		new ReadPointerProperty<Transform>("worldTransform", &wTrf));

	lTrf.setIdentity();
	wTrf.setIdentity();
	// Change the wTrf so it wont be the same as lTrf. This means that in the
	// updateWorldTransform() the "is moved" check will return true
	wTrf.setScale(0.0);
}

//==============================================================================
Movable::~Movable()
{}

//==============================================================================
void Movable::update()
{
	if(getParent() == nullptr) // If root
	{
		updateWorldTransform();
	}
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

	// Moved?
	if(prevWTrf != wTrf)
	{
		enableFlag(MF_MOVED);
		movableUpdate();
	}
	else
	{
		disableFlag(MF_MOVED);
	}

	for(Movable* child : getChildren())
	{
		child->updateWorldTransform();
	}
}

} // namespace anki
