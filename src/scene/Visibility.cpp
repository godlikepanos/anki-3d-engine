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
// VisibilityTestTask                                                          =
//==============================================================================

struct ThreadLocal
{
	VisibilityTestResults* m_testResults = nullptr;
};

struct ThreadCommon
{
	Array<ThreadLocal, Threadpool::MAX_THREADS> m_threadLocal;
	Barrier m_barrier;
	List<SceneNode*> m_frustumsList;
	SpinLock m_lock;
}

class VisibilityTestTask: public Threadpool::Task
{
public:
	SceneGraph* m_scene = nullptr;
	SceneNode* m_frustumableSn = nullptr;
	SceneFrameAllocator<U8> m_alloc;

	ThreadsLocal* m_threadLocal;

	/// Test a frustum component
	ANKI_USE_RESULT Error test(SceneNode& testedNode,
		U32 threadId, PtrSize threadsCount);

	ANKI_USE_RESULT Error combineTestResults(
		FrustumComponent& frc,
		PtrSize threadsCount);

	/// Do the tests
	Error operator()(U32 threadId, PtrSize threadsCount)
	{
		// Run once
		ANKI_CHECK(test(*m_frustumableSn, threadId, threadsCount));

		// Rucurse the extra frustumables
		while(!m_frustumsList.isEmpty())
		{
			printf("%d: going deeper\n", threadId);

			// Get front
			SceneNode* node = m_frustumsList.getFront();
			ANKI_ASSERT(node);

			m_barrier->wait();

			ANKI_CHECK(test(*node, threadId, threadsCount));

			// Pop
			if(threadId == 0)
			{
				m_frustumsList.popFront(m_alloc);
			}
		}

		if(threadId == 0)
		{
			m_frustumsList.destroy(m_alloc);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
Error VisibilityTestTask::test(SceneNode& testedNode, 
	U32 threadId, PtrSize threadsCount)
{
	Error err = ErrorCode::NONE;

	FrustumComponent& testedFr = 
		testedNode.getComponent<FrustumComponent>();
	Bool testedNodeShadowCaster = testedFr.getShadowCaster();

	// Init test results
	VisibilityTestResults* visible = 
		m_alloc.newInstance<VisibilityTestResults>();

	FrustumComponent::VisibilityStats stats = testedFr.getLastVisibilityStats();

	ANKI_CHECK(visible->create(
		m_alloc, stats.m_renderablesCount, stats.m_lightsCount, 4));

	// Chose the test range and a few other things
	PtrSize start, end;
	U nodesCount = m_scene->getSceneNodesCount();
	choseStartEnd(threadId, threadsCount, nodesCount, start, end);

	// Iterate range of nodes
	err = m_scene->iterateSceneNodes(start, end, [&](SceneNode& node) -> Error
	{
		Error err = ErrorCode::NONE;

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
		err = node.iterateComponentsOfType<SpatialComponent>(
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

		if(ANKI_UNLIKELY(count == 0))
		{
			return err;
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
		visibleNode.m_spatialIndices = m_alloc.newArray<U8>(count);
		if(ANKI_UNLIKELY(visibleNode.m_spatialIndices == nullptr))
		{
			return ErrorCode::OUT_OF_MEMORY;
		}

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
				err = visible->moveBackRenderable(m_alloc, visibleNode);
			}
		}
		else
		{
			if(r)
			{
				err = visible->moveBackRenderable(m_alloc, visibleNode);
			}

			LightComponent* l = node.tryGetComponent<LightComponent>();
			if(!err && l)
			{
				err = visible->moveBackLight(m_alloc, visibleNode);

				if(!err && l->getShadowEnabled() && fr)
				{
					LockGuard<SpinLock> l(m_lock);
					err = m_frustumsList.pushBack(m_alloc, &node);
				}
			}

			LensFlareComponent* lf = node.tryGetComponent<LensFlareComponent>();
			if(!err && lf)
			{
				err = visible->moveBackLensFlare(m_alloc, visibleNode);
				ANKI_ASSERT(visibleNode.m_node);
			}
		}
		
		return err;
	}); // end for

	ANKI_CHECK(err);

	m_threadLocal[threadId].m_testResults = visible;
	printf("%d: assigning %p\n", threadId, (void*)visible);

	// Gather the results from all threads
	m_barrier->wait();

	//printf("%d: checking what is assigned %p\n", threadId, 
	//		(void*)m_threadLocal[threadId].m_testResults);

	printf("%d: checking what is assigned to 0 %p\n", threadId, 
			(void*)m_threadLocal[0].m_testResults);
	
	if(threadId == 0)
	{
		printf("%d: combine\n", threadId);
		ANKI_CHECK(combineTestResults(testedFr, threadsCount));
	}
	m_barrier->wait();

	return err;
}

//==============================================================================
ANKI_USE_RESULT Error VisibilityTestTask::combineTestResults(
	FrustumComponent& frc,
	PtrSize threadsCount)
{
	// Count the visible scene nodes to optimize the allocation of the 
	// final result
	U32 renderablesSize = 0;
	U32 lightsSize = 0;
	U32 lensFlaresSize = 0;
	for(U i = 0; i < threadsCount; i++)
	{
		ANKI_ASSERT(m_threadLocal[i].m_testResults);
		VisibilityTestResults& rez = *m_threadLocal[i].m_testResults;

		renderablesSize += rez.getRenderablesCount();
		lightsSize += rez.getLightsCount();
		lensFlaresSize += rez.getLensFlaresCount();
	}

	// Allocate
	VisibilityTestResults* visible = 
		m_alloc.newInstance<VisibilityTestResults>();
	if(visible == nullptr)
	{
		return ErrorCode::OUT_OF_MEMORY;
	}

	ANKI_CHECK(
		visible->create(
		m_alloc, 
		renderablesSize, 
		lightsSize,
		lensFlaresSize));

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
		VisibilityTestResults& from = *m_threadLocal[i].m_testResults;

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

	return ErrorCode::NONE;
}

//==============================================================================
// VisibilityTestResults                                                       =
//==============================================================================

//==============================================================================
Error VisibilityTestResults::create(
	SceneFrameAllocator<U8> alloc,
	U32 renderablesReservedSize,
	U32 lightsReservedSize,
	U32 lensFlaresReservedSize)
{
	Error err = m_renderables.create(alloc, renderablesReservedSize);
	
	if(!err)
	{
		err = m_lights.create(alloc, lightsReservedSize);
	}

	if(!err)
	{
		err = m_flares.create(alloc, lensFlaresReservedSize);
	}

	return err;
}

//==============================================================================
Error VisibilityTestResults::moveBack(
	SceneFrameAllocator<U8> alloc, Container& c, U32& count, VisibleNode& x)
{
	Error err = ErrorCode::NONE;

	if(count + 1 > c.getSize())
	{
		// Need to grow
		U newSize = (c.getSize() != 0) ? c.getSize() * 2 : 2;
		err = c.resize(alloc, newSize);
	}

	if(!err)
	{
		c[count++] = x;
	}

	return err;
}

//==============================================================================
// doVisibilityTests                                                           =
//==============================================================================

//==============================================================================
Error doVisibilityTests(SceneNode& fsn, SceneGraph& scene, Renderer& r)
{
	// Do the tests in parallel
	Threadpool& threadPool = scene._getThreadpool();
	Barrier barrier(threadPool.getThreadsCount());
	ThreadsLocal tlocal;

	Array<VisibilityTestTask, Threadpool::MAX_THREADS> jobs;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].m_scene = &scene;
		jobs[i].m_frustumableSn = &fsn;
		jobs[i].m_alloc = scene.getFrameAllocator();
		jobs[i].m_barrier = &barrier;
		jobs[i].m_threadLocal = &tlocal;

		threadPool.assignNewTask(i, &jobs[i]);
	}
	
	return threadPool.waitForAllThreadsToFinish();
}

} // end namespace anki
