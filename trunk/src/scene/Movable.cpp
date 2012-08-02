#include "anki/scene/Movable.h"
#include "anki/scene/Property.h"

namespace anki {

//==============================================================================
Movable::Movable(uint32_t flags_, Movable* parent, PropertyMap& pmap)
	: Base(this, parent), Flags(flags_ | MF_MOVED)
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
	Movable* parent = getParent();

	if(parent)
	{
		if(isFlagEnabled(MF_IGNORE_LOCAL_TRANSFORM))
		{
			wTrf = parent->getWorldTransform();
		}
		else
		{
			wTrf = parent->getWorldTransform().getTransformed(lTrf);
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



	///////////////////
	if(parent)
	{
		if(parent->isFlagEnabled(MF_MOVED) || isFlagEnabled(MF_MOVED))
		{
			enableFlag(MF_MOVED);
			if(isFlagEnabled(MF_IGNORE_LOCAL_TRANSFORM))
			{
				wTrf = parent->getWorldTransform();
			}
			else
			{
				wTrf = parent->getWorldTransform().getTransformed(lTrf);
			}
		}
	}
	else
	{
		if(isFlagEnabled(MF_MOVED))
		{
			wTrf = lTrf;
		}
	}
	///////////////////

	if(isFlagEnabled(MF_MOVED))
	{
		movableUpdate();
	}

	for(Movable* child : getChildren())
	{
		child->updateWorldTransform();
	}
}

} // namespace anki
