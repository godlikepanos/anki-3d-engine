#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
/// Sort spatial scene nodes on distance
struct SortSubspatialsFunctor
{
	Vec3 origin; ///< The pos of the frustum
	Spatial* sp;

	Bool operator()(U32 a, U32 b)
	{
		ANKI_ASSERT(a != b 
			&& a < sp->getSubSpatialsCount()
			&& b < sp->getSubSpatialsCount());

		const Spatial* spa = *(sp->getSubSpatialsBegin() + a);
		const Spatial* spb = *(sp->getSubSpatialsBegin() + b);

		F32 dist0 = origin.getDistanceSquared(
			spa->getSpatialOrigin());
		F32 dist1 = origin.getDistanceSquared(
			spb->getSpatialOrigin());

		return dist0 < dist1;
	}
};

//==============================================================================
struct VisibilityTestJob: ThreadJob
{
	U nodesCount = 0;
	SceneGraph::Types<SceneNode>::Container::iterator nodes;
	SceneNode* frustumableSn = nullptr;
	Tiler* tiler = nullptr;
	SceneFrameAllocator<U8> frameAlloc;

	VisibilityTestResults* visible;

	/// Handle sub spatials
	Bool handleSubspatials(
		const Frustumable& fr,
		Spatial& sp,
		U32*& subSpatialIndices,
		U32& subSpatialIndicesCount)
	{
		Bool fatherSpVisible = true;
		subSpatialIndices = nullptr;
		subSpatialIndicesCount = 0;

		// Have subspatials?
		if(sp.getSubSpatialsCount())
		{
			subSpatialIndices = ANKI_NEW_ARRAY_0(
				U32, frameAlloc, sp.getSubSpatialsCount());

			U i = 0;
			for(auto it = sp.getSubSpatialsBegin();
				it != sp.getSubSpatialsEnd(); ++it)
			{
				Spatial* subsp = *it;
	
				// Check
				if(fr.insideFrustum(*subsp))
				{
					subSpatialIndices[subSpatialIndicesCount++] = i;
					subsp->enableBits(Spatial::SF_VISIBLE_CAMERA);
				}
				++i;
			}

			// Sort them
			SortSubspatialsFunctor functor;
			functor.origin = fr.getFrustumableOrigin();
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
	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, nodesCount, start, end);

		visible = ANKI_NEW(VisibilityTestResults, frameAlloc, frameAlloc);

		Frustumable* frustumable = frustumableSn->getFrustumable();
		ANKI_ASSERT(frustumable);

		for(auto it = nodes + start; it != nodes + end; it++)
		{
			SceneNode* node = *it;

			Frustumable* fr = node->getFrustumable();
			// Skip if it is the same
			if(ANKI_UNLIKELY(frustumable == fr))
			{
				continue;
			}

			Spatial* sp = node->getSpatial();
			if(!sp)
			{
				continue;
			}

			if(!frustumable->insideFrustum(*sp))
			{
				continue;
			}

			// Hierarchical spatial => check subspatials
			U32* subSpatialIndices = nullptr;
			U32 subSpatialIndicesCount = 0;
			if(!handleSubspatials(*frustumable, *sp, subSpatialIndices, 
				subSpatialIndicesCount))
			{
				continue;
			}

			// renderable
			Renderable* r = node->getRenderable();
			if(r)
			{
				visible->renderables.push_back(VisibleNode(
					node, subSpatialIndices, subSpatialIndicesCount));
			}
			else
			{
				Light* l = node->getLight();
				if(l)
				{
					visible->lights.push_back(VisibleNode(node, nullptr, 0));

					if(l->getShadowEnabled() && fr)
					{
						testLight(*node);
					}
				}
				else
				{
					continue;
				}
			}

			sp->enableBits(Spatial::SF_VISIBLE_CAMERA);
		} // end for
	}

	/// Test an individual light
	void testLight(SceneNode& lightSn)
	{
		ANKI_ASSERT(lightSn.getFrustumable() != nullptr);
		Frustumable& ref = *lightSn.getFrustumable();

		// Allocate new visibles
		VisibilityTestResults* lvisible = 
			ANKI_NEW(VisibilityTestResults, frameAlloc, frameAlloc);

		ref.setVisibilityTestResults(lvisible);

		for(auto it = nodes; it != nodes + nodesCount; it++)
		{
			SceneNode* node = *it;

			Frustumable* fr = node->getFrustumable();
			// Wont check the same
			if(ANKI_UNLIKELY(&ref == fr))
			{
				continue;
			}

			Spatial* sp = node->getSpatial();
			if(!sp)
			{
				continue;
			}

			if(!ref.insideFrustum(*sp))
			{
				continue;
			}

			// Hierarchical spatial => check subspatials
			U32* subSpatialIndices = nullptr;
			U32 subSpatialIndicesCount = 0;
			if(!handleSubspatials(ref, *sp, subSpatialIndices, 
				subSpatialIndicesCount))
			{
				continue;
			}

			sp->enableBits(Spatial::SF_VISIBLE_LIGHT);

			Renderable* r = node->getRenderable();
			if(r)
			{
				lvisible->renderables.push_back(VisibleNode(
					node, subSpatialIndices, subSpatialIndicesCount));
			}
		}
	}
};

//==============================================================================
void doVisibilityTests(SceneNode& fsn, SceneGraph& scene, 
	Renderer& r)
{
	Frustumable* fr = fsn.getFrustumable();
	ANKI_ASSERT(fr);

	//
	// Do the tests in parallel
	//
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	VisibilityTestJob jobs[ThreadPool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].nodesCount = scene.getSceneNodesCount();
		jobs[i].nodes = scene.getSceneNodesBegin();
		jobs[i].frustumableSn = &fsn;
		jobs[i].tiler = &r.getTiler();
		jobs[i].frameAlloc = scene.getFrameAllocator();

		threadPool.assignNewJob(i, &jobs[i]);
	}

	threadPool.waitForAllJobsToFinish();

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
		ANKI_NEW(VisibilityTestResults, scene.getFrameAllocator(), 
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
	dsjob.origin = fr->getFrustumableOrigin();
	threadPool.assignNewJob(0, &dsjob);

	// The rest of the jobs are dummy
	ThreadJobDummy dummyjobs[ThreadPool::MAX_THREADS];
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewJob(i, &dummyjobs[i]);
	}

	// Sort the renderables in the main thread
	DistanceSortFunctor dsfunc;
	dsfunc.origin = fr->getFrustumableOrigin();
	std::sort(visible->renderables.begin(), visible->renderables.end(), dsfunc);

	threadPool.waitForAllJobsToFinish();
}

} // end namespace anki