#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
SpatialComponent::SpatialComponent(SceneNode* node, const CollisionShape* cs,
	U32 flags)
	:	Base(nullptr, node->getSceneAllocator()), 
		Bitset<U8>(flags),
		spatialCs(cs)
{
	ANKI_ASSERT(spatialCs);
	markForUpdate();
}

//==============================================================================
SpatialComponent::~SpatialComponent()
{}

//==============================================================================
void SpatialComponent::update()
{
	if(getParent() == nullptr)
	{
		visitThisAndChildren([](SpatialComponent& sp)
		{
			sp.updateInternal();
		});	
	}
}

//==============================================================================
void SpatialComponent::updateInternal()
{
	if(bitsEnabled(SF_MARKED_FOR_UPDATE))
	{
		spatialCs->toAabb(aabb);
		origin = (aabb.getMax() + aabb.getMin()) * 0.5;
		timestamp = getGlobTimestamp();

		disableBits(SF_MARKED_FOR_UPDATE);
	}
}

//==============================================================================
void SpatialComponent::resetFrame()
{
	// Call this only on roots
	if(getParent() == nullptr)
	{
		visitThisAndChildren([](SpatialComponent& sp)
		{
			sp.disableBits(SF_VISIBLE_ANY);
		});
	}
}

} // end namespace anki
