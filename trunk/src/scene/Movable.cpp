#include "anki/scene/Movable.h"
#include "anki/scene/Property.h"

namespace anki {

//==============================================================================
Movable::Movable(U32 flags_, Movable* parent, PropertyMap& pmap,
	const SceneAllocator<Movable>& alloc)
	: Base(parent, alloc), Bitset(flags_)
{
	pmap.addNewProperty(
		new ReadWritePointerProperty<Transform>("localTransform", &lTrf));
	pmap.addNewProperty(
		new ReadPointerProperty<Transform>("worldTransform", &wTrf));

	movableMarkForUpdate();
}

//==============================================================================
Movable::~Movable()
{}

//==============================================================================
void Movable::update()
{
	// If root begin updating
	if(getParent() == nullptr)
	{
		updateWorldTransform();
	}
}

//==============================================================================
void Movable::updateWorldTransform()
{
	prevWTrf = wTrf;
	const Movable* parent = getParent();
	const Bool dirty = bitsEnabled(MF_TRANSFORM_DIRTY);

	// If dirty then update world transform
	if(dirty)
	{
		if(parent)
		{
			if(bitsEnabled(MF_IGNORE_LOCAL_TRANSFORM))
			{
				wTrf = parent->getWorldTransform();
			}
			else
			{
				wTrf = Transform::combineTransformations(
					parent->getWorldTransform(), lTrf);
			}
		}
		else
		{
			wTrf = lTrf;
		}

		movableUpdate();
	}

	// Update the children
	Movable::Container::iterator it = getChildrenBegin();
	for(; it != getChildrenEnd(); it++)
	{
		// If parent is dirty then make children dirty as well
		if(dirty)
		{
			(*it)->movableMarkForUpdate();
		}

		(*it)->updateWorldTransform();
	}

	// Now it's a good time to cleanse parent
	disableBits(MF_TRANSFORM_DIRTY);
}

} // end namespace anki
