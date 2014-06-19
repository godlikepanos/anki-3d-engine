// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
class VisibilityTestTask: public ThreadpoolTask
{
public:
	U m_nodesCount = 0;
	SceneGraph* m_scene = nullptr;
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
			choseStartEnd(threadId, threadsCount, m_nodesCount, start, end);
			cameraVisible = visible;
		}
		else
		{
			// Is light
			start = 0;
			end = m_nodesCount;
			testedFr.setVisibilityTestResults(visible);
		}

		// Iterate range of nodes
		m_scene->iterateSceneNodes(start, end, [&](SceneNode& node)
		{
			FrustumComponent* fr = node.tryGetComponent<FrustumComponent>();

			// Skip if it is the same
			if(ANKI_UNLIKELY(&testedFr == fr))
			{
				return;
			}

			VisibleNode visibleNode;
			visibleNode.m_node = &node;

			// Test all spatial components of that node
			struct SpatialTemp
			{
				SpatialComponent* sp;
				U8 idx;
			};
			Array<SpatialTemp, ANKI_GL_MAX_SUB_DRAWCALLS> sps;

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
			Vec4 origin = testedFr.getFrustumOrigin();
			std::sort(sps.begin(), sps.begin() + count, 
				[&](const SpatialTemp& a, const SpatialTemp& b) -> Bool
			{
				Vec4 spa = a.sp->getSpatialOrigin();
				Vec4 spb = b.sp->getSpatialOrigin();

				F32 dist0 = origin.getDistanceSquared(spa);
				F32 dist1 = origin.getDistanceSquared(spb);

				return dist0 < dist1;
			});

			// Update the visibleNode
			ANKI_ASSERT(count < MAX_U8);
			visibleNode.m_spatialsCount = count;
			visibleNode.m_spatialIndices = frameAlloc.newArray<U8>(count);
			for(U i = 0; i < count; i++)
			{
				visibleNode.m_spatialIndices[i] = sps[i].idx;
			}

			// Do something with the result
			RenderComponent* r = node.tryGetComponent<RenderComponent>();
			if(isLight)
			{
				if(r && r->getCastsShadow())
				{
					visible->m_renderables.emplace_back(std::move(visibleNode));
				}
			}
			else
			{
				if(r)
				{
					visible->m_renderables.emplace_back(std::move(visibleNode));
				}
				else
				{
					LightComponent* l = node.tryGetComponent<LightComponent>();
					if(l)
					{
						Light* light = staticCastPtr<Light*>(&node);

						visible->m_lights.emplace_back(std::move(visibleNode));

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
		jobs[i].m_nodesCount = scene.getSceneNodesCount();
		jobs[i].m_scene = &scene;
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
		renderablesSize += jobs[i].cameraVisible->m_renderables.size();
		lightsSize += jobs[i].cameraVisible->m_lights.size();
	}

	// Allocate
	VisibilityTestResults* visible = 
		scene.getFrameAllocator().newInstance<VisibilityTestResults>(
		scene.getFrameAllocator(), 
		renderablesSize, 
		lightsSize);

	if(renderablesSize == 0)
	{
		ANKI_LOGW("No visible renderables");
	}

	visible->m_renderables.resize(renderablesSize);
	visible->m_lights.resize(lightsSize);

	// Append thread results
	renderablesSize = 0;
	lightsSize = 0;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		const VisibilityTestResults& from = *jobs[i].cameraVisible;

		if(from.m_renderables.size() > 0)
		{
			memcpy(&visible->m_renderables[renderablesSize],
				&from.m_renderables[0],
				sizeof(VisibleNode) * from.m_renderables.size());

			renderablesSize += from.m_renderables.size();
		}

		if(from.m_lights.size() > 0)
		{
			memcpy(&visible->m_lights[lightsSize],
				&from.m_lights[0],
				sizeof(VisibleNode) * from.m_lights.size());

			lightsSize += from.m_lights.size();
		}
	}

	// Set the frustumable
	fr.setVisibilityTestResults(visible);

	//
	// Sort
	//

	// The lights
	DistanceSortJob dsjob;
	dsjob.m_nodes = visible->m_lights.begin();
	dsjob.m_nodesCount = visible->m_lights.size();
	dsjob.m_origin = fr.getFrustumOrigin();
	threadPool.assignNewTask(0, &dsjob);

	// The rest of the jobs are dummy
	DummyThreadpoolTask dummyjobs[Threadpool::MAX_THREADS];
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewTask(i, &dummyjobs[i]);
	}

	// Sort the renderables in the main thread
	DistanceSortFunctor dsfunc;
	dsfunc.m_origin = fr.getFrustumOrigin();
	std::sort(
		visible->m_renderables.begin(), visible->m_renderables.end(), dsfunc);

	threadPool.waitForAllThreadsToFinish();
}

} // end namespace anki
