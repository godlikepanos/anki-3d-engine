#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
/// Sort spatial scene nodes on distance
struct SortSubspatialsFunctor
{
	Vec3 origin; ///< The pos of the frustum
	SpatialComponent* sp;

	Bool operator()(U32 a, U32 b)
	{
		ANKI_ASSERT(a != b 
			&& a < sp->getSubSpatialsCount()
			&& b < sp->getSubSpatialsCount());

		const SpatialComponent& spa = sp->getSubSpatial(a);
		const SpatialComponent& spb = sp->getSubSpatial(b);

		F32 dist0 = 
			origin.getDistanceSquared(spa.getSpatialOrigin());
		F32 dist1 = 
			origin.getDistanceSquared(spb.getSpatialOrigin());

		return dist0 < dist1;
	}
};

//==============================================================================
struct VisibilityTestJob: ThreadpoolTask
{
	U nodesCount = 0;
	SceneGraph* scene = nullptr;
	SceneNode* frustumableSn = nullptr;
	Tiler* tiler = nullptr;
	SceneFrameAllocator<U8> frameAlloc;

	VisibilityTestResults* visible;

	/// Handle sub spatials
	Bool handleSubspatials(
		const FrustumComponent& fr,
		SpatialComponent& sp,
		U32*& subSpatialIndices,
		U32& subSpatialIndicesCount)
	{
		Bool fatherSpVisible = true;
		subSpatialIndices = nullptr;
		subSpatialIndicesCount = 0;

		// Have subspatials?
		if(sp.getSubSpatialsCount())
		{
			subSpatialIndices = 
				frameAlloc.newArray<U32>(sp.getSubSpatialsCount());

			U i = 0;
			sp.visitChildren([&](SpatialComponent& subsp)
			{	
				// Check
				if(fr.insideFrustum(subsp))
				{
					subSpatialIndices[subSpatialIndicesCount++] = i;
					subsp.enableBits(SpatialComponent::SF_VISIBLE_CAMERA);
				}
				++i;
			});

			// Sort them
			SortSubspatialsFunctor functor;
			functor.origin = fr.getFrustumOrigin();
			functor.sp = &sp;
			std::sort(subSpatialIndices, 
				subSpatialIndices + subSpatialIndicesCount,
				functor);

			// The subSpatialIndicesCount == 0 then the camera is looking 
			// something in between all sub spatials
			fatherSpVisible = subSpatialIndicesCount != 0;
		}

		return fatherSpVisible;
	}

	/// Do the tests
	void operator()(ThreadId threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, nodesCount, start, end);

		visible = frameAlloc.newInstance<VisibilityTestResults>(frameAlloc);

		FrustumComponent* frustumable = frustumableSn->getFrustumComponent();
		ANKI_ASSERT(frustumable);

		scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			FrustumComponent* fr = node.getFrustumComponent();

			// Skip if it is the same
			if(ANKI_UNLIKELY(frustumable == fr))
			{
				return;
			}

			SpatialComponent* sp = node.getSpatialComponent();
			if(!sp)
			{
				return;
			}

			if(!frustumable->insideFrustum(*sp))
			{
				return;
			}

			// Hierarchical spatial => check subspatials
			U32* subSpatialIndices = nullptr;
			U32 subSpatialIndicesCount = 0;
			if(!handleSubspatials(*frustumable, *sp, subSpatialIndices, 
				subSpatialIndicesCount))
			{
				return;
			}

			// renderable
			RenderComponent* r = node.getRenderComponent();
			if(r)
			{
				visible->renderables.push_back(VisibleNode(
					&node, subSpatialIndices, subSpatialIndicesCount));
			}
			else
			{
				Light* l = node.getLight();
				if(l)
				{
					visible->lights.push_back(VisibleNode(&node, nullptr, 0));

					if(l->getShadowEnabled() && fr)
					{
						testLight(node);
					}
				}
				else
				{
					return;
				}
			}

			sp->enableBits(SpatialComponent::SF_VISIBLE_CAMERA);
		}); // end for
	}

	/// Test an individual light
	void testLight(SceneNode& lightSn)
	{
		ANKI_ASSERT(lightSn.getFrustumComponent() != nullptr);
		FrustumComponent& ref = *lightSn.getFrustumComponent();

		// Allocate new visibles
		VisibilityTestResults* lvisible = 
			frameAlloc.newInstance<VisibilityTestResults>(frameAlloc);

		ref.setVisibilityTestResults(lvisible);

		scene->iterateSceneNodes([&](SceneNode& node)
		{
			FrustumComponent* fr = node.getFrustumComponent();
			RenderComponent* r = node.getRenderComponent();

			// Wont check the same
			if(ANKI_UNLIKELY(&ref == fr))
			{
				return;
			}

			SpatialComponent* sp = node.getSpatialComponent();
			if(!sp)
			{
				return;
			}

			if(!ref.insideFrustum(*sp))
			{
				return;
			}

			// Hierarchical spatial => check subspatials
			U32* subSpatialIndices = nullptr;
			U32 subSpatialIndicesCount = 0;
			if(!handleSubspatials(ref, *sp, subSpatialIndices, 
				subSpatialIndicesCount))
			{
				return;
			}

			sp->enableBits(SpatialComponent::SF_VISIBLE_LIGHT);

			if(r && r->castsShadow())
			{
				lvisible->renderables.push_back(VisibleNode(
					&node, subSpatialIndices, subSpatialIndicesCount));
			}
		}); // end lambda
	}
};

//==============================================================================
void doVisibilityTests(SceneNode& fsn, SceneGraph& scene, 
	Renderer& r)
{
	FrustumComponent* fr = fsn.getFrustumComponent();
	ANKI_ASSERT(fr);

	//
	// Do the tests in parallel
	//
	Threadpool& threadPool = ThreadpoolSingleton::get();
	VisibilityTestJob jobs[Threadpool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].nodesCount = scene.getSceneNodesCount();
		jobs[i].scene = &scene;
		jobs[i].frustumableSn = &fsn;
		jobs[i].tiler = &r.getTiler();
		jobs[i].frameAlloc = scene.getFrameAllocator();

		threadPool.assignNewTask(i, &jobs[i]);
	}

	threadPool.waitForAllThreadsToFinish();

	//
	// Combine results
	//

	// Count the visible scene nodes to optimize the allocation of the 
	// final result
	U32 renderablesSize = 0;
	U32 lightsSize = 0;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		renderablesSize += jobs[i].visible->renderables.size();
		lightsSize += jobs[i].visible->lights.size();
	}

	// Allocate
	VisibilityTestResults* visible = 
		scene.getFrameAllocator().newInstance<VisibilityTestResults>(
		scene.getFrameAllocator(), 
		renderablesSize, 
		lightsSize);

	visible->renderables.resize(renderablesSize);
	visible->lights.resize(lightsSize);

	// Append thread results
	renderablesSize = 0;
	lightsSize = 0;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		const VisibilityTestResults& from = *jobs[i].visible;

		memcpy(&visible->renderables[renderablesSize],
			&from.renderables[0],
			sizeof(VisibleNode) * from.renderables.size());

		renderablesSize += from.renderables.size();

		memcpy(&visible->lights[lightsSize],
			&from.lights[0],
			sizeof(VisibleNode) * from.lights.size());

		lightsSize += from.lights.size();
	}

	// Set the frustumable
	fr->setVisibilityTestResults(visible);

	//
	// Sort
	//

	// The lights
	DistanceSortJob dsjob;
	dsjob.nodes = visible->lights.begin();
	dsjob.nodesCount = visible->lights.size();
	dsjob.origin = fr->getFrustumOrigin();
	threadPool.assignNewTask(0, &dsjob);

	// The rest of the jobs are dummy
	DummyThreadpoolTask dummyjobs[Threadpool::MAX_THREADS];
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewTask(i, &dummyjobs[i]);
	}

	// Sort the renderables in the main thread
	DistanceSortFunctor dsfunc;
	dsfunc.origin = fr->getFrustumOrigin();
	std::sort(visible->renderables.begin(), visible->renderables.end(), dsfunc);

	threadPool.waitForAllThreadsToFinish();
}

} // end namespace anki
