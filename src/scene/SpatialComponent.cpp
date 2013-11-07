#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
SpatialComponent::SpatialComponent(SceneNode* node, const CollisionShape* cs,
	U32 flags)
	:	SceneComponent(this),
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
Bool SpatialComponent::update(SceneNode&, F32, F32)
{
	Bool needsUpdate = bitsEnabled(SF_MARKED_FOR_UPDATE);
	if(needsUpdate)
	{
		spatialCs->toAabb(aabb);
		disableBits(SF_MARKED_FOR_UPDATE);
	}

	return needsUpdate;
}

//==============================================================================
void SpatialComponent::reset()
{
	disableBits(SF_VISIBLE_ANY);
}

} // end namespace anki
