// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/MoveComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
MoveComponent::MoveComponent(SceneNode* node, Flag flags)
:	SceneComponent(Type::MOVE, node),
	Bitset<Flag>(flags),
	m_node(node)
{
	markForUpdate();
}

//==============================================================================
MoveComponent::~MoveComponent()
{}

//==============================================================================
Error MoveComponent::update(SceneNode& node, F32, F32, Bool& updated)
{
	updated = updateWorldTransform(node);
	return ErrorCode::NONE;
}

//==============================================================================
Bool MoveComponent::updateWorldTransform(SceneNode& node)
{
	m_prevWTrf = m_wtrf;
	const Bool dirty = bitsEnabled(Flag::MARKED_FOR_UPDATE);

	// If dirty then update world transform
	if(dirty)
	{
		const SceneObject* parentObj = node.getParent();

		if(parentObj)
		{
			const SceneNode* parent = &parentObj->downCast<SceneNode>();
			const MoveComponent* parentMove = 
				parent->tryGetComponent<MoveComponent>();

			if(parentMove == nullptr)
			{
				// Parent not movable
				m_wtrf = m_ltrf;
			}
			else if(bitsEnabled(Flag::IGNORE_PARENT_TRANSFORM))
			{
				m_wtrf = m_ltrf;
			}
			else if(bitsEnabled(Flag::IGNORE_LOCAL_TRANSFORM))
			{
				m_wtrf = parentMove->getWorldTransform();
			}
			else
			{
				m_wtrf = parentMove->getWorldTransform().
					combineTransformations(m_ltrf);
			}
		}
		else
		{
			// No parent

			m_wtrf = m_ltrf;
		}

		// Now it's a good time to cleanse parent
		disableBits(Flag::MARKED_FOR_UPDATE);
	}

	// If this is dirty then make children dirty as well. Don't walk the 
	// whole tree because you will re-walk it later
	if(dirty)
	{
		Error err = node.visitChildrenMaxDepth(1, [](SceneObject& obj) -> Error
		{ 
			if(obj.getType() == SceneNode::getClassType())
			{
				SceneNode& childNode = obj.downCast<SceneNode>();

				childNode.iterateComponentsOfType<MoveComponent>([](
					MoveComponent& mov)
				{
					mov.markForUpdate();
				});
			}

			return ErrorCode::NONE;
		});

		(void)err;
	}

	return dirty;
}

} // end namespace anki
