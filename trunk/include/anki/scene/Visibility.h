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
/// @note Keep this structore as small as possible
struct VisibleNode
{
	SceneNode* node;
	/// An array of the visible spatials
	U8* spatialIndices;
	U8 spatialsCount;

	VisibleNode()
		: node(nullptr), spatialIndices(nullptr), spatialsCount(0)
	{}

	VisibleNode(const VisibleNode& other)
	{
		operator=(other);
	}

	VisibleNode(VisibleNode&& other)
	{
		node = other.node;
		spatialIndices = other.spatialIndices;
		spatialsCount = other.spatialsCount;

		other.node = nullptr;
		other.spatialIndices = nullptr;
		other.spatialsCount = 0;
	}

	VisibleNode& operator=(const VisibleNode& other)
	{
		node = other.node;
		spatialIndices = other.spatialIndices;
		spatialsCount = other.spatialsCount;
		return *this;
	}

	U8 getSpatialIndex(U i)
	{
		ANKI_ASSERT(spatialsCount != 0 && i < spatialsCount);
		return spatialIndices[i];
	}
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
			a.node->getComponent<SpatialComponent>().getSpatialOrigin());
		F32 dist1 = origin.getDistanceSquared(
			b.node->getComponent<SpatialComponent>().getSpatialOrigin());

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
