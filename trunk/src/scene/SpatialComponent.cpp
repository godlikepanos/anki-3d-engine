#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
SpatialComponent::SpatialComponent(SceneNode* node, U32 flags)
	:	SceneComponent(this, node),
		Bitset<U8>(flags)
{
	markForUpdate();
}

//==============================================================================
SpatialComponent::~SpatialComponent()
{}

//==============================================================================
Bool SpatialComponent::update(SceneNode&, F32, F32, UpdateType uptype)
{
	Bool updated = false;

	if(uptype == ASYNC_UPDATE)
	{
		updated = bitsEnabled(SF_MARKED_FOR_UPDATE);
		if(updated)
		{
			getSpatialCollisionShape().toAabb(aabb);
			disableBits(SF_MARKED_FOR_UPDATE);
		}
	}

	return updated;
}

//==============================================================================
void SpatialComponent::reset()
{
	disableBits(SF_VISIBLE_ANY);
}

} // end namespace anki
