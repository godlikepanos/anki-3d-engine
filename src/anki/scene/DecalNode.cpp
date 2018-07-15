// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/DecalNode.h>
#include <anki/scene/components/DecalComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>

namespace anki
{

/// Decal feedback component.
class DecalMoveFeedbackComponent : public SceneComponent
{
public:
	DecalMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(Second, Second, Bool& updated) override
	{
		updated = false;

		MoveComponent& movec = m_node->getComponent<MoveComponent>();

		if(movec.getTimestamp() == m_node->getGlobalTimestamp())
		{
			static_cast<DecalNode*>(m_node)->onMove(movec);
		}

		return Error::NONE;
	}
};

/// Decal feedback component.
class DecalShapeFeedbackComponent : public SceneComponent
{
public:
	DecalShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(Second, Second, Bool& updated) override
	{
		updated = false;

		DecalComponent& decalc = m_node->getComponent<DecalComponent>();

		if(decalc.getTimestamp() == m_node->getGlobalTimestamp())
		{
			static_cast<DecalNode*>(m_node)->onDecalUpdated();
		}

		return Error::NONE;
	}
};

DecalNode::~DecalNode()
{
}

Error DecalNode::init()
{
	newComponent<MoveComponent>();
	newComponent<DecalMoveFeedbackComponent>();
	DecalComponent* decalc = newComponent<DecalComponent>();
	newComponent<DecalShapeFeedbackComponent>();
	newComponent<SpatialComponent>(&decalc->getBoundingVolume());

	return Error::NONE;
}

void DecalNode::onMove(MoveComponent& movec)
{
	SpatialComponent& sc = getComponent<SpatialComponent>();
	sc.setSpatialOrigin(movec.getWorldTransform().getOrigin());
	sc.markForUpdate();

	DecalComponent& decalc = getComponent<DecalComponent>();
	decalc.updateTransform(movec.getWorldTransform());
}

void DecalNode::onDecalUpdated()
{
	SpatialComponent& sc = getComponent<SpatialComponent>();
	sc.markForUpdate();
}

} // end namespace anki
