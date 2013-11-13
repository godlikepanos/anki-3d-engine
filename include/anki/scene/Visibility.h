#ifndef ANKI_SCENE_VISIBILITY_TEST_RESULTS_H
#define ANKI_SCENE_VISIBILITY_TEST_RESULTS_H

#include "anki/scene/Common.h"
#include "anki/collision/Forward.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/RenderComponent.h"
#include "anki/core/Threadpool.h"

namespace anki {

// Forward
class Renderer;

/// @addtogroup Scene
/// @{

/// Visibility test type
enum VisibilityTest
{
	VT_RENDERABLES = 1 << 0,
	VT_ONLY_SHADOW_CASTERS = 1 << 1,
	VT_LIGHTS = 1 << 2
};

/// Visible by
enum VisibleBy
{
	VB_NONE = 0,
	VB_CAMERA = 1 << 0,
	VB_LIGHT = 1 << 1
};

/// Visible node pointer with some more info
struct VisibleNode
{
	SceneNode* node;
	/// An array of the visible sub spatials. Allocated but never deallocated
	/// because it lives in the scene frame mem pool
	U32* subSpatialIndices;
	U32 subSpatialIndicesCount;

	VisibleNode()
		: node(nullptr), subSpatialIndices(nullptr), subSpatialIndicesCount(0)
	{}

	VisibleNode(SceneNode* node_, U32* subSpatialIndices_, 
		U32 subSpatialIndicesCount_)
		:	node(node_), 
			subSpatialIndices(subSpatialIndices_), 
			subSpatialIndicesCount(subSpatialIndicesCount_)
	{
		ANKI_ASSERT(node);
	}

	VisibleNode(const VisibleNode& other)
		:	node(other.node), 
			subSpatialIndices(other.subSpatialIndices),
			subSpatialIndicesCount(other.subSpatialIndicesCount)
	{}
};

/// Its actually a container for visible entities. It should be per frame
struct VisibilityTestResults
{
	typedef SceneFrameVector<VisibleNode> Container;

	Container renderables;
	Container lights;

	VisibilityTestResults(const SceneAllocator<U8>& frameAlloc,
		U32 renderablesReservedSize = 
		ANKI_FRUSTUMABLE_AVERAGE_VISIBLE_RENDERABLES_COUNT,
		U32 lightsReservedSize =
		ANKI_FRUSTUMABLE_AVERAGE_VISIBLE_LIGHTS_COUNT)
		: renderables(frameAlloc), lights(frameAlloc)
	{
		renderables.reserve(renderablesReservedSize);
		lights.reserve(lightsReservedSize);
	}
};

/// Sort spatial scene nodes on distance
struct DistanceSortFunctor
{
	Vec3 origin;

	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.node && b.node);

		F32 dist0 = origin.getDistanceSquared(
			a.node->getComponent<SpatialComponent>().getOrigin());
		F32 dist1 = origin.getDistanceSquared(
			b.node->getComponent<SpatialComponent>().getOrigin());

		return dist0 < dist1;
	}
};

/// Sort renderable scene nodes on material
struct MaterialSortFunctor
{
	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.node && b.node);

		return a.node->getComponent<RenderComponent>().getMaterial()
			< b.node->getComponent<RenderComponent>().getMaterial();
	}
};

/// Thread job to short scene nodes by distance
struct DistanceSortJob: ThreadpoolTask
{
	U nodesCount;
	VisibilityTestResults::Container::iterator nodes;
	Vec3 origin;

	void operator()(ThreadId /*threadId*/, U /*threadsCount*/)
	{
		DistanceSortFunctor comp;
		comp.origin = origin;
		std::sort(nodes, nodes + nodesCount, comp);
	}
};

/// Thread job to short renderable scene nodes by material
struct MaterialSortJob: ThreadpoolTask
{
	U nodesCount;
	VisibilityTestResults::Container::iterator nodes;

	void operator()(ThreadId /*threadId*/, U /*threadsCount*/)
	{
		std::sort(nodes, nodes + nodesCount, MaterialSortFunctor());
	}
};

/// Do visibility tests bypassing portals 
void doVisibilityTests(SceneNode& frustumable, SceneGraph& scene, 
	Renderer& renderer);

/// @}

} // end namespace anki

#endif
