#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
struct VisibilityTestTask: ThreadpoolTask
{
	U nodesCount = 0;
	SceneGraph* scene = nullptr;
	SceneNode* frustumableSn = nullptr;
	SceneFrameAllocator<U8> frameAlloc;

	VisibilityTestResults* cameraVisible; // out

	/// Test a frustum component
	void test(SceneNode& testedNode, Bool isLight, 
		ThreadId threadId, U threadsCount)
	{
		ANKI_ASSERT(isLight == 
			(testedNode.tryGetComponent<LightComponent>() != nullptr));

		VisibilityTestResults* visible = 
			frameAlloc.newInstance<VisibilityTestResults>(frameAlloc);

		FrustumComponent& testedFr = 
			testedNode.getComponent<FrustumComponent>();

		// Chose the test range and a few other things
		PtrSize start, end;
		if(!isLight)
		{
			choseStartEnd(threadId, threadsCount, nodesCount, start, end);
			cameraVisible = visible;
		}
		else
		{
			// Is light
			start = 0;
			end = nodesCount;
			testedFr.setVisibilityTestResults(visible);
		}

		// Iterate range of nodes
		scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			FrustumComponent* fr = node.tryGetComponent<FrustumComponent>();

			// Skip if it is the same
			if(ANKI_UNLIKELY(&testedFr == fr))
			{
				return;
			}

			VisibleNode visibleNode;
			visibleNode.node = &node;

			// Test all spatial components of that node
			struct SpatialTemp
			{
				SpatialComponent* sp;
				U8 idx;
			};
			Array<SpatialTemp, ANKI_MAX_MULTIDRAW_PRIMITIVES> sps;

			U spIdx = 0;
			U count = 0;
			node.iterateComponentsOfType<SpatialComponent>(
				[&](SpatialComponent& sp)
			{
				if(testedFr.insideFrustum(sp))
				{
					// Inside
					ANKI_ASSERT(spIdx < MAX_U8);
					sps[count++] = SpatialTemp{&sp, (U8)spIdx};

					sp.enableBits(isLight 
						? SpatialComponent::SF_VISIBLE_LIGHT 
						: SpatialComponent::SF_VISIBLE_CAMERA);
				}

				++spIdx;
			});

			if(count == 0)
			{
				return;
			}

			// Sort spatials
			Vec3 origin = testedFr.getFrustumOrigin();
			std::sort(sps.begin(), sps.begin() + count, 
				[&](const SpatialTemp& a, const SpatialTemp& b) -> Bool
			{
				Vec3 spa = a.sp->getSpatialOrigin();
				Vec3 spb = b.sp->getSpatialOrigin();

				F32 dist0 = origin.getDistanceSquared(spa);
				F32 dist1 = origin.getDistanceSquared(spb);

				return dist0 < dist1;
			});

			// Update the visibleNode
			ANKI_ASSERT(count < MAX_U8);
			visibleNode.spatialsCount = count;
			visibleNode.spatialIndices = frameAlloc.newArray<U8>(count);
			for(U i = 0; i < count; i++)
			{
				visibleNode.spatialIndices[i] = sps[i].idx;
			}

			// Do something with the result
			RenderComponent* r = node.tryGetComponent<RenderComponent>();
			if(isLight)
			{
				if(r && r->getCastsShadow())
				{
					visible->renderables.push_back(visibleNode);
				}
			}
			else
			{
				if(r)
				{
					visible->renderables.push_back(visibleNode);
				}
				else
				{
					LightComponent* l = node.tryGetComponent<LightComponent>();
					if(l)
					{
						Light* light = staticCast<Light*>(&node);

						visible->lights.push_back(visibleNode);

						if(light->getShadowEnabled() && fr)
						{
							test(node, true, 0, 0);
						}
					}
				}
			}
		}); // end for
	}

	/// Do the tests
	void operator()(ThreadId threadId, U threadsCount)
	{
		test(*frustumableSn, false, threadId, threadsCount);
	}
};

//==============================================================================
void doVisibilityTests(SceneNode& fsn, SceneGraph& scene, 
	Renderer& r)
{
	FrustumComponent& fr = fsn.getComponent<FrustumComponent>();

	//
	// Do the tests in parallel
	//
	Threadpool& threadPool = ThreadpoolSingleton::get();
	VisibilityTestTask jobs[Threadpool::MAX_THREADS];
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].nodesCount = scene.getSceneNodesCount();
		jobs[i].scene = &scene;
		jobs[i].frustumableSn = &fsn;
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
		renderablesSize += jobs[i].cameraVisible->renderables.size();
		lightsSize += jobs[i].cameraVisible->lights.size();
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
		const VisibilityTestResults& from = *jobs[i].cameraVisible;

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
	fr.setVisibilityTestResults(visible);

	//
	// Sort
	//

	// The lights
	DistanceSortJob dsjob;
	dsjob.nodes = visible->lights.begin();
	dsjob.nodesCount = visible->lights.size();
	dsjob.origin = fr.getFrustumOrigin();
	threadPool.assignNewTask(0, &dsjob);

	// The rest of the jobs are dummy
	DummyThreadpoolTask dummyjobs[Threadpool::MAX_THREADS];
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewTask(i, &dummyjobs[i]);
	}

	// Sort the renderables in the main thread
	DistanceSortFunctor dsfunc;
	dsfunc.origin = fr.getFrustumOrigin();
	std::sort(visible->renderables.begin(), visible->renderables.end(), dsfunc);

	threadPool.waitForAllThreadsToFinish();
}

} // end namespace anki
