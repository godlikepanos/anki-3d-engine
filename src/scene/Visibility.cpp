// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/Light.h"
#include "anki/renderer/Renderer.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Sort spatial scene nodes on distance
class DistanceSortFunctor
{
public:
	Vec4 m_origin;

	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.m_node && b.m_node);

		F32 dist0 = m_origin.getDistanceSquared(
			a.m_node->getComponent<SpatialComponent>().getSpatialOrigin());
		F32 dist1 = m_origin.getDistanceSquared(
			b.m_node->getComponent<SpatialComponent>().getSpatialOrigin());

		return dist0 < dist1;
	}
};

/// Sort renderable scene nodes on material
class MaterialSortFunctor
{
public:
	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.m_node && b.m_node);

		return a.m_node->getComponent<RenderComponent>().getMaterial()
			< b.m_node->getComponent<RenderComponent>().getMaterial();
	}
};

/// Data common for all tasks.
class VisibilityShared
{
public:
	SceneGraph* m_scene = nullptr;
	Barrier m_barrier;
	List<SceneNode*> m_frustumsList; ///< Nodes to test
	SpinLock m_lock;

	// Data per thread but that can be accessed by all threads
	Array<VisibilityTestResults*, Threadpool::MAX_THREADS> m_testResults;

	VisibilityShared(U threadCount)
	:	m_barrier(threadCount)
	{
		memset(&m_testResults[0], 0, sizeof(m_testResults));
	}
};

/// Task.
class VisibilityTestTask: public Threadpool::Task
{
public:
	VisibilityShared* m_shared;

	/// Test a frustum component
	void test(SceneNode& testedNode, U32 threadId, PtrSize threadsCount);

	void combineTestResults(FrustumComponent& frc, PtrSize threadsCount);

	/// Do the tests
	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		auto& list = m_shared->m_frustumsList;
		auto alloc = m_shared->m_scene->getFrameAllocator();

		// Iterate the nodes to check against
		while(!list.isEmpty())
		{
			// Get front and pop it
			SceneNode* node = list.getFront();
			ANKI_ASSERT(node);

			m_shared->m_barrier.wait();
			if(threadId == 0)
			{
				list.popFront(alloc);
			}

			m_shared->m_barrier.wait();
			test(*node, threadId, threadsCount);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
void VisibilityTestTask::test(SceneNode& testedNode, 
	U32 threadId, PtrSize threadsCount)
{
	FrustumComponent& testedFr = 
		testedNode.getComponent<FrustumComponent>();
	Bool testedNodeShadowCaster = testedFr.getShadowCaster();
	auto alloc = m_shared->m_scene->getFrameAllocator();

	// Init test results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();

	FrustumComponent::VisibilityStats stats = testedFr.getLastVisibilityStats();

	visible->create(alloc, stats.m_renderablesCount, stats.m_lightsCount, 4);

	m_shared->m_testResults[threadId] = visible;

	List<SceneNode*> frustumsList;

	// Chose the test range and a few other things
	PtrSize start, end;
	U nodesCount = m_shared->m_scene->getSceneNodesCount();
	choseStartEnd(threadId, threadsCount, nodesCount, start, end);

	// Iterate range of nodes
	Error err = m_shared->m_scene->iterateSceneNodes(
		start, end, [&](SceneNode& node) -> Error
	{
		FrustumComponent* fr = node.tryGetComponent<FrustumComponent>();
		
		// Skip if it is the same
		if(ANKI_UNLIKELY(&testedFr == fr))
		{
			return ErrorCode::NONE;
		}

		VisibleNode visibleNode;
		visibleNode.m_node = &node;

		// Test all spatial components of that node
		struct SpatialTemp
		{
			SpatialComponent* m_sp;
			U8 m_idx;
		};
		Array<SpatialTemp, ANKI_GL_MAX_SUB_DRAWCALLS> sps;

		U spIdx = 0;
		U count = 0;
		Error err = node.iterateComponentsOfType<SpatialComponent>(
			[&](SpatialComponent& sp)
		{
			if(testedFr.insideFrustum(sp))
			{
				// Inside
				ANKI_ASSERT(spIdx < MAX_U8);
				sps[count++] = SpatialTemp{&sp, static_cast<U8>(spIdx)};

				sp.enableBits(testedNodeShadowCaster 
					? SpatialComponent::Flag::VISIBLE_LIGHT 
					: SpatialComponent::Flag::VISIBLE_CAMERA);
			}

			++spIdx;

			return ErrorCode::NONE;
		});
		(void)err;

		if(ANKI_UNLIKELY(count == 0))
		{
			return ErrorCode::NONE;
		}

		// Sort sub-spatials
		Vec4 origin = testedFr.getFrustumOrigin();
		std::sort(sps.begin(), sps.begin() + count, 
			[origin](const SpatialTemp& a, const SpatialTemp& b) -> Bool
		{
			Vec4 spa = a.m_sp->getSpatialOrigin();
			Vec4 spb = b.m_sp->getSpatialOrigin();

			F32 dist0 = origin.getDistanceSquared(spa);
			F32 dist1 = origin.getDistanceSquared(spb);

			return dist0 < dist1;
		});

		// Update the visibleNode
		ANKI_ASSERT(count < MAX_U8);
		visibleNode.m_spatialsCount = count;
		visibleNode.m_spatialIndices = alloc.newArray<U8>(count);

		for(U i = 0; i < count; i++)
		{
			visibleNode.m_spatialIndices[i] = sps[i].m_idx;
		}

		// Do something with the result
		RenderComponent* r = node.tryGetComponent<RenderComponent>();
		if(testedNodeShadowCaster)
		{
			if(r && r->getCastsShadow())
			{
				visible->moveBackRenderable(alloc, visibleNode);
			}
		}
		else
		{
			if(r)
			{
				visible->moveBackRenderable(alloc, visibleNode);
			}

			LightComponent* l = node.tryGetComponent<LightComponent>();
			if(l)
			{
				visible->moveBackLight(alloc, visibleNode);

				if(l->getShadowEnabled() && fr)
				{
					LockGuard<SpinLock> l(m_shared->m_lock);
					m_shared->m_frustumsList.pushBack(alloc, &node);
				}
			}

			LensFlareComponent* lf = node.tryGetComponent<LensFlareComponent>();
			if(lf)
			{
				visible->moveBackLensFlare(alloc, visibleNode);
				ANKI_ASSERT(visibleNode.m_node);
			}
		}
		
		return ErrorCode::NONE;
	}); // end for
	(void)err;

	// Gather the results from all threads
	m_shared->m_barrier.wait();
	
	if(threadId == 0)
	{
		combineTestResults(testedFr, threadsCount);
	}
}

//==============================================================================
void VisibilityTestTask::combineTestResults(
	FrustumComponent& frc,
	PtrSize threadsCount)
{
	auto alloc = m_shared->m_scene->getFrameAllocator();

	// Count the visible scene nodes to optimize the allocation of the 
	// final result
	U32 renderablesSize = 0;
	U32 lightsSize = 0;
	U32 lensFlaresSize = 0;
	for(U i = 0; i < threadsCount; i++)
	{
		VisibilityTestResults& rez = *m_shared->m_testResults[i];

		renderablesSize += rez.getRenderablesCount();
		lightsSize += rez.getLightsCount();
		lensFlaresSize += rez.getLensFlaresCount();
	}

	// Allocate
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();

	visible->create(
		alloc, 
		renderablesSize, 
		lightsSize,
		lensFlaresSize);

	visible->prepareMerge();

	if(renderablesSize == 0)
	{
		ANKI_LOGW("No visible renderables");
	}

	// Append thread results
	VisibleNode* renderables = visible->getRenderablesBegin();
	VisibleNode* lights = visible->getLightsBegin();
	VisibleNode* lensFlares = visible->getLensFlaresBegin();
	for(U i = 0; i < threadsCount; i++)
	{
		VisibilityTestResults& from = *m_shared->m_testResults[i];

		U rCount = from.getRenderablesCount();
		U lCount = from.getLightsCount();
		U lfCount = from.getLensFlaresCount();

		if(rCount > 0)
		{
			memcpy(renderables,
				from.getRenderablesBegin(),
				sizeof(VisibleNode) * rCount);

			renderables += rCount;
		}

		if(lCount > 0)
		{
			memcpy(lights,
				from.getLightsBegin(),
				sizeof(VisibleNode) * lCount);

			lights += lCount;
		}

		if(lfCount > 0)
		{
			memcpy(lensFlares,
				from.getLensFlaresBegin(),
				sizeof(VisibleNode) * lfCount);

			lensFlares += lfCount;
		}
	}

	// Set the frustumable
	frc.setVisibilityTestResults(visible);

	// Sort lights
	DistanceSortFunctor comp;
	comp.m_origin = frc.getFrustumOrigin();
	//std::sort(visible->getLightsBegin(), visible->getLightsEnd(), comp);

	// Sort the renderables
	std::sort(
		visible->getRenderablesBegin(), visible->getRenderablesEnd(), comp);
}

//==============================================================================
// VisibilityTestResults                                                       =
//==============================================================================

//==============================================================================
void VisibilityTestResults::create(
	SceneFrameAllocator<U8> alloc,
	U32 renderablesReservedSize,
	U32 lightsReservedSize,
	U32 lensFlaresReservedSize)
{
	m_renderables.create(alloc, renderablesReservedSize);
	m_lights.create(alloc, lightsReservedSize);
	m_flares.create(alloc, lensFlaresReservedSize);
}

//==============================================================================
void VisibilityTestResults::moveBack(
	SceneFrameAllocator<U8> alloc, Container& c, U32& count, VisibleNode& x)
{
	if(count + 1 > c.getSize())
	{
		// Need to grow
		U newSize = (c.getSize() != 0) ? c.getSize() * 2 : 2;
		c.resize(alloc, newSize);
	}

	c[count++] = x;
}

//==============================================================================
// doVisibilityTests                                                           =
//==============================================================================

//==============================================================================
Error doVisibilityTests(SceneNode& fsn, SceneGraph& scene, Renderer& r)
{
	// Do the tests in parallel
	Threadpool& threadPool = scene._getThreadpool();
	
	VisibilityShared shared(threadPool.getThreadsCount());
	shared.m_scene = &scene;
	shared.m_frustumsList.pushBack(scene.getFrameAllocator(), &fsn);

	Array<VisibilityTestTask, Threadpool::MAX_THREADS> tasks;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_shared = &shared;
		threadPool.assignNewTask(i, &tasks[i]);
	}
	
	return threadPool.waitForAllThreadsToFinish();
}

} // end namespace anki
