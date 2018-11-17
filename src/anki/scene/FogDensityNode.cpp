// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/FogDensityNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/FogDensityComponent.h>
#include <anki/scene/components/SpatialComponent.h>

namespace anki
{

class FogDensityNode::FeedbackComponent : public SceneComponent
{
public:
	FeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& movec = node.getComponent<MoveComponent>();
		if(movec.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<FogDensityNode&>(node).moveUpdated(movec);
		}

		return Error::NONE;
	}
};

FogDensityNode::FogDensityNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Create components
	newComponent<MoveComponent>(MoveComponentFlag::NONE);
	newComponent<FeedbackComponent>();
	newComponent<FogDensityComponent>();
	newComponent<SpatialComponent>(this, &m_spatialBox);
}

FogDensityNode::~FogDensityNode()
{
}

void FogDensityNode::moveUpdated(const MoveComponent& movec)
{
	// Update the fog component
	FogDensityComponent& fogc = getComponent<FogDensityComponent>();
	fogc.updatePosition(movec.getWorldTransform().getOrigin());

	// Update the spatial component
	SpatialComponent& spatialc = getComponent<SpatialComponent>();

	Vec4 min, max;
	if(fogc.isAabb())
	{
		fogc.getAabb(min, max);
	}
	else
	{
		F32 radius;
		fogc.getSphere(radius);

		min = Vec4(-radius, -radius, -radius, 0.0f);
		max = Vec4(radius, radius, radius, 0.0f);
	}

	min += movec.getWorldTransform().getOrigin();
	max += movec.getWorldTransform().getOrigin();

	m_spatialBox.setMin(min);
	m_spatialBox.setMax(max);

	spatialc.setSpatialOrigin(movec.getWorldTransform().getOrigin());
	spatialc.markForUpdate();
}

} // end namespace anki