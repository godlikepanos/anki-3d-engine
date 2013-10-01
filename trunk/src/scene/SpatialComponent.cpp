#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"

namespace anki {

//==============================================================================
SpatialComponent::SpatialComponent(SceneNode* node, const CollisionShape* cs,
	U32 flags)
	:	Base(nullptr, node->getSceneAllocator()), 
		Bitset<U8>(flags | SF_MARKED_FOR_UPDATE),
		spatialCs(cs)
{
}

//==============================================================================
SpatialComponent::~SpatialComponent()
{}

//==============================================================================
void SpatialComponent::spatialMarkForUpdate()
{
	// Call this only on roots
	ANKI_ASSERT(getParent() == nullptr);

	visitThisAndChildren([](SpatialComponent& sp)
	{
		sp.enableBits(SF_MARKED_FOR_UPDATE);
	});
}

//==============================================================================
void SpatialComponent::update()
{
	// Call this only on roots
	ANKI_ASSERT(getParent() == nullptr);

	visitThisAndChildren([](SpatialComponent& sp)
	{
		sp.updateInternal();
	});
}

//==============================================================================
void SpatialComponent::resetFrame()
{
	// Call this only on roots
	ANKI_ASSERT(getParent() == nullptr);

	visitThisAndChildren([](SpatialComponent& sp)
	{
		sp.disableBits(SF_VISIBLE_ANY);
	});
}

} // end namespace anki
