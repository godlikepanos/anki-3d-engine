// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/DecalNode.h>
#include <anki/scene/DecalComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SpatialComponent.h>

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

	ANKI_USE_RESULT Error update(SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		MoveComponent& movec = node.getComponent<MoveComponent>();

		if(movec.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<DecalNode&>(node).onMove(movec);
		}

		return ErrorCode::NONE;
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

	ANKI_USE_RESULT Error update(SceneNode& node, F32, F32, Bool& updated)
	{
		updated = false;

		DecalComponent& decalc = node.getComponent<DecalComponent>();

		if(decalc.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<DecalNode&>(node).onDecalUpdated();
		}

		return ErrorCode::NONE;
	}
};

DecalNode::~DecalNode()
{
}

Error DecalNode::init()
{
	newComponent<MoveComponent>(this);
	newComponent<DecalMoveFeedbackComponent>(this);
	DecalComponent* decalc = newComponent<DecalComponent>(this);
	newComponent<DecalShapeFeedbackComponent>(this);
	newComponent<SpatialComponent>(this, &decalc->getBoundingVolume());

	return ErrorCode::NONE;
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
