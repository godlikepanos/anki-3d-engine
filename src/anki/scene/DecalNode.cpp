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
class DecalNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& movec = node.getComponent<MoveComponent>();

		if(movec.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<DecalNode&>(node).onMove(movec);
		}

		return Error::NONE;
	}
};

/// Decal feedback component.
class DecalNode::ShapeFeedbackComponent : public SceneComponent
{
public:
	ShapeFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		DecalComponent& decalc = node.getComponent<DecalComponent>();

		if(decalc.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<DecalNode&>(node).onDecalUpdated();
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
	newComponent<MoveFeedbackComponent>();
	DecalComponent* decalc = newComponent<DecalComponent>(this);
	newComponent<ShapeFeedbackComponent>();
	newComponent<SpatialComponent>(this, &decalc->getBoundingVolume());

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
