// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/MoveComponent.h>
#include <anki/scene/SceneNode.h>

namespace anki
{

MoveComponent::MoveComponent(SceneNode* node, MoveComponentFlag flags)
	: SceneComponent(CLASS_TYPE, node)
	, m_flags(flags)
{
	markForUpdate();
}

MoveComponent::~MoveComponent()
{
}

Error MoveComponent::update(SceneNode& node, F32, F32, Bool& updated)
{
	updated = updateWorldTransform(node);
	return ErrorCode::NONE;
}

Bool MoveComponent::updateWorldTransform(SceneNode& node)
{
	m_prevWTrf = m_wtrf;
	const Bool dirty = m_flags.get(MoveComponentFlag::MARKED_FOR_UPDATE);

	// If dirty then update world transform
	if(dirty)
	{
		const SceneNode* parent = node.getParent();

		if(parent)
		{
			const MoveComponent* parentMove = parent->tryGetComponent<MoveComponent>();

			if(parentMove == nullptr)
			{
				// Parent not movable
				m_wtrf = m_ltrf;
			}
			else if(m_flags.get(MoveComponentFlag::IGNORE_PARENT_TRANSFORM))
			{
				m_wtrf = m_ltrf;
			}
			else if(m_flags.get(MoveComponentFlag::IGNORE_LOCAL_TRANSFORM))
			{
				m_wtrf = parentMove->getWorldTransform();
			}
			else
			{
				m_wtrf = parentMove->getWorldTransform().combineTransformations(m_ltrf);
			}
		}
		else
		{
			// No parent

			m_wtrf = m_ltrf;
		}

		// Now it's a good time to cleanse parent
		m_flags.unset(MoveComponentFlag::MARKED_FOR_UPDATE);
	}

	// If this is dirty then make children dirty as well. Don't walk the whole tree because you will re-walk it later
	if(dirty)
	{
		Error err = node.visitChildrenMaxDepth(1, [](SceneNode& childNode) -> Error {
			Error e = childNode.iterateComponentsOfType<MoveComponent>([](MoveComponent& mov) -> Error {
				mov.markForUpdate();
				return ErrorCode::NONE;
			});

			(void)e;

			return ErrorCode::NONE;
		});

		(void)err;
	}

	return dirty;
}

} // end namespace anki
