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
			static_cast<DecalNode&>(node).onDecalUpdated(decalc);
		}

		return ErrorCode::NONE;
	}
};

DecalNode::~DecalNode()
{
}

Error DecalNode::init()
{
	SceneComponent* comp;

	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	comp = getSceneAllocator().newInstance<DecalMoveFeedbackComponent>(this);
	addComponent(comp, true);

	comp = getSceneAllocator().newInstance<DecalComponent>(this);
	addComponent(comp, true);

	comp = getSceneAllocator().newInstance<DecalShapeFeedbackComponent>(this);
	addComponent(comp, true);

	comp = getSceneAllocator().newInstance<SpatialComponent>(this, &m_obbW);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

void DecalNode::onMove(MoveComponent& movec)
{
	SpatialComponent& sc = getComponent<SpatialComponent>();
	sc.setSpatialOrigin(movec.getWorldTransform().getOrigin());
	sc.markForUpdate();

	DecalComponent& decalc = getComponent<DecalComponent>();
	decalc.updateTransform(movec.getWorldTransform());

	updateObb(movec.getWorldTransform(), decalc);
}

void DecalNode::onDecalUpdated(DecalComponent& decalc)
{
	SpatialComponent& sc = getComponent<SpatialComponent>();
	sc.markForUpdate();

	const MoveComponent& movec = getComponent<MoveComponent>();
	updateObb(movec.getWorldTransform(), decalc);
}

void DecalNode::updateObb(const Transform& trf, const DecalComponent& decalc)
{
	Vec4 center(0.0f, 0.0f, -decalc.getDepth() / 2.0f, 0.0f);
	Vec4 extend(decalc.getWidth() / 2.0f, decalc.getHeight() / 2.0f, decalc.getDepth() / 2.0f, 0.0f);

	Obb obbL(center, Mat3x4::getIdentity(), extend);

	m_obbW = obbL.getTransformed(trf);
}

} // end namespace anki
