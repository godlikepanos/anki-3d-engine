#ifndef ANKI_SCENE_VISIBILITY_TEST_RESULTS_H
#define ANKI_SCENE_VISIBILITY_TEST_RESULTS_H

#include "anki/scene/Common.h"
#include "anki/collision/Forward.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"
#include "anki/core/ThreadPool.h"

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

/// Its actually a container for visible entities. It should be per frame
struct VisibilityTestResults
{
	typedef SceneFrameVector<SceneNode*> Container;

	/// Used to optimize the initial vector size a bit
	static const AVERAGE_NUMBER_OF_VISIBLE_SCENE_NODES = 10;

	Container renderables;
	Container lights;

	VisibilityTestResults(const SceneAllocator<U8>& frameAlloc,
		U32 renderablesReservedSize = AVERAGE_NUMBER_OF_VISIBLE_SCENE_NODES,
		U32 lightsReservedSize = AVERAGE_NUMBER_OF_VISIBLE_SCENE_NODES)
		: renderables(frameAlloc), lights(frameAlloc)
	{
		renderables.reserve(renderablesReservedSize);
		lights.reserve(lightsReservedSize);
	}

	Container::iterator getRenderablesBegin()
	{
		return renderables.begin();
	}
	Container::iterator getRenderablesEnd()
	{
		return renderables.end();
	}
	U32 getRenderablesCount() const
	{
		return renderables.size();
	}

	Container::iterator getLightsBegin()
	{
		return lights.begin();
	}
	Container::iterator getLightsEnd()
	{
		return lights.end();
	}
};

/// Sort spatial scene nodes on distance
struct DistanceSortFunctor
{
	Vec3 origin;

	Bool operator()(SceneNode* a, SceneNode* b)
	{
		ANKI_ASSERT(a->getSpatial() != nullptr && b->getSpatial() != nullptr);

		F32 dist0 = origin.getDistanceSquared(
			a->getSpatial()->getSpatialOrigin());
		F32 dist1 = origin.getDistanceSquared(
			b->getSpatial()->getSpatialOrigin());

		return dist0 < dist1;
	}
};

/// Sort renderable scene nodes on material
struct MaterialSortFunctor
{
	Bool operator()(SceneNode* a, SceneNode* b)
	{
		ANKI_ASSERT(a->getRenderable() != nullptr 
			&& b->getRenderable() != nullptr);

		return a->getRenderable()->getRenderableMaterial()
			< b->getRenderable()->getRenderableMaterial();
	}
};

/// Thread job to short scene nodes by distance
struct DistanceSortJob: ThreadJob
{
	U nodesCount;
	VisibilityTestResults::Container::iterator nodes;
	Vec3 origin;

	void operator()(U threadId, U threadsCount)
	{
		DistanceSortFunctor comp;
		comp.origin = origin;
		std::sort(nodes, nodes + nodesCount, comp);
	}
};

/// Thread job to short renderable scene nodes by material
struct MaterialSortJob: ThreadJob
{
	U nodesCount;
	VisibilityTestResults::Container::iterator nodes;

	void operator()(U threadId, U threadsCount)
	{
		std::sort(nodes, nodes + nodesCount, MaterialSortFunctor());
	}
};

/// Do visibility tests bypassing portals 
void doVisibilityTests(SceneNode& frustumable, Scene& scene, 
	Renderer& renderer);

/// @}

} // end namespace anki

#endif
