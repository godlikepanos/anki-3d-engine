#include "anki/scene/Movable.h"
#include "anki/scene/Property.h"

namespace anki {

//==============================================================================
Movable::Movable(uint32_t flags_, Movable* parent, PropertyMap& pmap)
	: Base(this, parent), Flags(flags_)
{
	pmap.addNewProperty(
		new ReadWritePointerProperty<Transform>("localTransform", &lTrf));
	pmap.addNewProperty(
		new ReadPointerProperty<Transform>("worldTransform", &wTrf));
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
	uint32_t sceneFramesCount = SceneSingleton::get().getFramesCount();

	if(parent)
	{
		if(parent->lastUpdateFrame == sceneFramesCount
			|| lastUpdateFrame == sceneFramesCount)
		{
			if(isFlagEnabled(MF_IGNORE_LOCAL_TRANSFORM))
			{
				wTrf = parent->getWorldTransform();
			}
			else
			{
				wTrf = Transform::combineTransformations(
					parent->getWorldTransform(), lTrf);
			}
			lastUpdateFrame = sceneFramesCount;
		}
	}
	else if(lastUpdateFrame == sceneFramesCount)
	{
		wTrf = lTrf;
	}

	if(lastUpdateFrame == sceneFramesCount)
	{
		movableUpdate();
	}

	for(Movable* child : getChildren())
	{
		child->updateWorldTransform();
	}
}

} // namespace anki
