#include "anki/scene/Movable.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
Movable::Movable(U32 flags_, SceneNode* node_)
	: Bitset(flags_), node(node_)
{
	ANKI_ASSERT(node);

	/*pmap.addNewProperty(
		new ReadWritePointerProperty<Transform>("localTransform", &lTrf));
	pmap.addNewProperty(
		new ReadPointerProperty<Transform>("worldTransform", &wTrf));*/

	movableMarkForUpdate();
}

//==============================================================================
Movable::~Movable()
{}

//==============================================================================
void Movable::update()
{
	// If root begin updating
	if(node->getParent() == nullptr)
	{
		updateWorldTransform();
	}
}

//==============================================================================
void Movable::updateWorldTransform()
{
	prevWTrf = wTrf;
	const Bool dirty = bitsEnabled(MF_TRANSFORM_DIRTY);

	// If dirty then update world transform
	if(dirty)
	{
		const Movable* parent = 
			node->getParent() 
			? node->getParent()->getMovable() 
			: nullptr;

		if(parent && !bitsEnabled(MF_IGNORE_PARENT))
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
	SceneNode::Base::Container::iterator it = node->getChildrenBegin();
	for(; it != node->getChildrenEnd(); it++)
	{
		SceneNode* sn = (*it);
		ANKI_ASSERT(sn);
		Movable* mov = sn->getMovable();

		// If child is movable update it
		if(mov)
		{
			// If parent is dirty then make children dirty as well
			if(dirty)
			{
				mov->movableMarkForUpdate();
			}

			mov->updateWorldTransform();
		}
	}

	// Now it's a good time to cleanse parent
	disableBits(MF_TRANSFORM_DIRTY);
}

} // end namespace anki
