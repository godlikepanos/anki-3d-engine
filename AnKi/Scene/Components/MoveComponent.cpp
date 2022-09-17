// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(MoveComponent)

MoveComponent::MoveComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_ignoreLocalTransform(false)
	, m_ignoreParentTransform(false)
{
	markForUpdate();
}

MoveComponent::~MoveComponent()
{
}

Error MoveComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = updateWorldTransform(*info.m_node);
	return Error::kNone;
}

Bool MoveComponent::updateWorldTransform(SceneNode& node)
{
	m_prevWTrf = m_wtrf;
	const Bool dirty = m_markedForUpdate;

	// If dirty then update world transform
	if(dirty)
	{
		const SceneNode* parent = node.getParent();

		if(parent)
		{
			const MoveComponent* parentMove = parent->tryGetFirstComponentOfType<MoveComponent>();

			if(parentMove == nullptr)
			{
				// Parent not movable
				m_wtrf = m_ltrf;
			}
			else if(m_ignoreParentTransform)
			{
				m_wtrf = m_ltrf;
			}
			else if(m_ignoreLocalTransform)
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
		m_markedForUpdate = false;
	}

	// If this is dirty then make children dirty as well. Don't walk the whole tree because you will re-walk it later
	if(dirty)
	{
		[[maybe_unused]] const Error err = node.visitChildrenMaxDepth(1, [](SceneNode& childNode) -> Error {
			childNode.iterateComponentsOfType<MoveComponent>([](MoveComponent& mov) {
				mov.markForUpdate();
			});
			return Error::kNone;
		});
	}

	return dirty;
}

} // end namespace anki
