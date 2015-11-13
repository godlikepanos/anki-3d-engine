// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Visibility.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Sector.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/scene/ReflectionProxyComponent.h>
#include <anki/scene/Light.h>
#include <anki/scene/MoveComponent.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/util/Logger.h>
#include <anki/core/Trace.h>

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
	const Renderer* m_r = nullptr;
	Barrier m_barrier;
	List<FrustumComponent*> m_frustumsList; ///< Frustums to test
	SpinLock m_lock;

	Timestamp m_timestamp = 0;
	SpinLock m_timestampLock;

	// Data per thread but that can be accessed by all threads
	Array<VisibilityTestResults*, ThreadPool::MAX_THREADS> m_testResults;

	VisibilityShared(U threadCount)
		: m_barrier(threadCount)
	{
		memset(&m_testResults[0], 0, sizeof(m_testResults));
	}
};

/// Task.
class VisibilityTestTask: public ThreadPool::Task
{
public:
	VisibilityShared* m_shared;

	/// Test a frustum component
	void test(FrustumComponent& frcToTest, U32 threadId, PtrSize threadsCount);

	void combineTestResults(FrustumComponent& frc, PtrSize threadsCount);

	/// Do the tests
	Error operator()(U32 threadId, PtrSize threadsCount) override
	{
		ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_TESTS);
		auto& list = m_shared->m_frustumsList;
		auto alloc = m_shared->m_scene->getFrameAllocator();

		// Iterate the nodes to check against
		while(!list.isEmpty())
		{
			// Get front and pop it
			FrustumComponent* frc = list.getFront();
			ANKI_ASSERT(frc);

			m_shared->m_barrier.wait();
			if(threadId == 0)
			{
				list.popFront(alloc);

				// Set initial value of timestamp
				m_shared->m_timestamp = 0;
			}

			m_shared->m_barrier.wait();
			test(*frc, threadId, threadsCount);
		}
		ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_TESTS);

		return ErrorCode::NONE;
	}

	/// Update the timestamp if the node moved or changed its shape.
	void updateTimestamp(const SceneNode& node)
	{
		Timestamp lastUpdate = 0;

		const FrustumComponent* bfr = node.tryGetComponent<FrustumComponent>();
		if(bfr)
		{
			lastUpdate = max(lastUpdate, bfr->getTimestamp());
		}

		const MoveComponent* bmov = node.tryGetComponent<MoveComponent>();
		if(bmov)
		{
			lastUpdate = max(lastUpdate, bmov->getTimestamp());
		}

		const SpatialComponent* sp = node.tryGetComponent<SpatialComponent>();
		if(sp)
		{
			lastUpdate = max(lastUpdate, sp->getTimestamp());
		}

		LockGuard<SpinLock> lock(m_shared->m_timestampLock);
		m_shared->m_timestamp = max(m_shared->m_timestamp, lastUpdate);
	}
};

//==============================================================================
void VisibilityTestTask::test(FrustumComponent& testedFrc,
	U32 threadId, PtrSize threadsCount)
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_TEST);
	ANKI_ASSERT(testedFrc.anyVisibilityTestEnabled());

	SceneNode& testedNode = testedFrc.getSceneNode();
	auto alloc = m_shared->m_scene->getFrameAllocator();

	// Init test results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();

	FrustumComponent::VisibilityStats stats =
		testedFrc.getLastVisibilityStats();

	visible->create(alloc, stats.m_renderablesCount, stats.m_lightsCount, 4, 4);

	m_shared->m_testResults[threadId] = visible;

	List<SceneNode*> frustumsList;

	Bool wantsRenderComponents = testedFrc.visibilityTestsEnabled(
		FrustumComponent::VisibilityTestFlag::TEST_RENDER_COMPONENTS);

	Bool wantsLightComponents = testedFrc.visibilityTestsEnabled(
		FrustumComponent::VisibilityTestFlag::TEST_LIGHT_COMPONENTS);

	Bool wantsFlareComponents = testedFrc.visibilityTestsEnabled(
		FrustumComponent::VisibilityTestFlag::TEST_LENS_FLARE_COMPONENTS);

	Bool wantsShadowCasters = testedFrc.visibilityTestsEnabled(
		FrustumComponent::VisibilityTestFlag::TEST_SHADOW_CASTERS);

	Bool wantsReflectionProbes = testedFrc.visibilityTestsEnabled(
		FrustumComponent::VisibilityTestFlag::TEST_REFLECTION_PROBES);

	Bool wantsReflectionProxies = testedFrc.visibilityTestsEnabled(
		FrustumComponent::VisibilityTestFlag::TEST_REFLECTION_PROXIES);

#if 0
	ANKI_LOGW("Running test code");

	// Chose the test range and a few other things
	PtrSize start, end;
	U nodesCount = m_shared->m_scene->getSceneNodesCount();
	choseStartEnd(threadId, threadsCount, nodesCount, start, end);

	// Iterate range of nodes
	Error err = m_shared->m_scene->iterateSceneNodes(
		start, end, [&](SceneNode& node) -> Error
#else
	SectorGroup& sectors = m_shared->m_scene->getSectorGroup();
	if(threadId == 0)
	{
		sectors.prepareForVisibilityTests(testedFrc, *m_shared->m_r);
	}
	m_shared->m_barrier.wait();

	// Chose the test range and a few other things
	PtrSize start, end;
	U nodesCount = sectors.getVisibleNodesCount();
	choseStartEnd(threadId, threadsCount, nodesCount, start, end);

	Error err = sectors.iterateVisibleSceneNodes(
		start, end, [&](SceneNode& node) -> Error
#endif
	{
		// Skip if it is the same
		if(ANKI_UNLIKELY(&testedNode == &node))
		{
			return ErrorCode::NONE;
		}

		// Check what components the frustum needs
		Bool wantNode = false;

		RenderComponent* rc = node.tryGetComponent<RenderComponent>();
		if(rc && wantsRenderComponents)
		{
			wantNode = true;
		}

		if(rc && rc->getCastsShadow() && wantsShadowCasters)
		{
			wantNode = true;
		}

		LightComponent* lc = node.tryGetComponent<LightComponent>();
		if(lc && wantsLightComponents)
		{
			wantNode = true;
		}

		LensFlareComponent* lfc = node.tryGetComponent<LensFlareComponent>();
		if(lfc && wantsFlareComponents)
		{
			wantNode = true;
		}

		ReflectionProbeComponent* reflc =
			node.tryGetComponent<ReflectionProbeComponent>();
		if(reflc && wantsReflectionProbes)
		{
			wantNode = true;
		}

		ReflectionProxyComponent* proxyc =
			node.tryGetComponent<ReflectionProxyComponent>();
		if(proxyc && wantsReflectionProxies)
		{
			wantNode = true;
		}

		if(ANKI_UNLIKELY(!wantNode))
		{
			// Skip node
			return ErrorCode::NONE;
		}

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
			if(testedFrc.insideFrustum(sp))
			{
				// Inside
				ANKI_ASSERT(spIdx < MAX_U8);
				sps[count++] = SpatialTemp{&sp, static_cast<U8>(spIdx)};

				sp.setVisibleByCamera(true);
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
		Vec4 origin = testedFrc.getFrustumOrigin();
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
		VisibleNode visibleNode;
		visibleNode.m_node = &node;

		ANKI_ASSERT(count < MAX_U8);
		visibleNode.m_spatialsCount = count;
		visibleNode.m_spatialIndices = alloc.newArray<U8>(count);

		for(U i = 0; i < count; i++)
		{
			visibleNode.m_spatialIndices[i] = sps[i].m_idx;
		}

		if(rc)
		{
			if(wantsRenderComponents ||
				(wantsShadowCasters && rc->getCastsShadow()))
			{
				visible->moveBackRenderable(alloc, visibleNode);

				if(wantsShadowCasters)
				{
					updateTimestamp(node);
				}
			}
		}

		if(lc && wantsLightComponents)
		{
			visible->moveBackLight(alloc, visibleNode);
		}

		if(lfc && wantsFlareComponents)
		{
			visible->moveBackLensFlare(alloc, visibleNode);
		}

		if(reflc && wantsReflectionProbes)
		{
			visible->moveBackReflectionProbe(alloc, visibleNode);
		}

		if(proxyc && wantsReflectionProxies)
		{
			visible->moveBackReflectionProxy(alloc, visibleNode);
		}

		// Add more frustums to the list
		err = node.iterateComponentsOfType<FrustumComponent>(
			[&](FrustumComponent& frc)
		{
			// Check enabled and make sure that the results are null (this can
			// happen on multiple on circular viewing)
			if(frc.anyVisibilityTestEnabled()
				&& !frc.hasVisibilityTestResults())
			{
				LockGuard<SpinLock> l(m_shared->m_lock);

				// Check if already in the list
				Bool alreadyThere = false;
				for(const FrustumComponent* x : m_shared->m_frustumsList)
				{
					if(x == &frc)
					{
						alreadyThere = true;
						break;
					}
				}

				if(!alreadyThere)
				{
					m_shared->m_frustumsList.pushBack(alloc, &frc);
				}
			}

			return ErrorCode::NONE;
		});
		(void)err;

		return ErrorCode::NONE;
	}); // end for
	(void)err;

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_TEST);

	// Gather the results from all threads
	m_shared->m_barrier.wait();

	if(threadId == 0)
	{
		ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);
		combineTestResults(testedFrc, threadsCount);
		ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);
	}
}

//==============================================================================
void VisibilityTestTask::combineTestResults(
	FrustumComponent& frc,
	PtrSize threadsCount)
{
	auto alloc = m_shared->m_scene->getFrameAllocator();

	// Prepare
	Array<VisibilityTestResults*, ThreadPool::MAX_THREADS> results;
	for(U i = 0; i < threadsCount; i++)
	{
		results[i] = m_shared->m_testResults[i];
	}
	SArray<VisibilityTestResults*> rez(&results[0], threadsCount);

	// Create the new combined results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();
	visible->combineWith(alloc, rez);
	visible->setShapeUpdateTimestamp(m_shared->m_timestamp);

	// Set the frustumable
	frc.setVisibilityTestResults(visible);

	// Sort some of the arrays
	DistanceSortFunctor comp;
	comp.m_origin = frc.getFrustumOrigin();
	std::sort(visible->getRenderablesBegin(), visible->getRenderablesEnd(),
		comp);

	std::sort(visible->getReflectionProbesBegin(),
		visible->getReflectionProbesEnd(), comp);
}

//==============================================================================
// VisibilityTestResults                                                       =
//==============================================================================

//==============================================================================
void VisibilityTestResults::create(
	SceneFrameAllocator<U8> alloc,
	U32 renderablesReservedSize,
	U32 lightsReservedSize,
	U32 lensFlaresReservedSize,
	U32 reflectionProbesReservedSize)
{
	m_groups[RENDERABLES].m_nodes.create(alloc, renderablesReservedSize);
	m_groups[LIGHTS].m_nodes.create(alloc, lightsReservedSize);
	m_groups[FLARES].m_nodes.create(alloc, lensFlaresReservedSize);
	m_groups[REFLECTION_PROBES].m_nodes.create(alloc,
		reflectionProbesReservedSize);
	m_groups[REFLECTION_PROXIES].m_nodes.create(alloc,
		reflectionProbesReservedSize);
}

//==============================================================================
void VisibilityTestResults::moveBack(SceneFrameAllocator<U8> alloc,
	GroupType type, VisibleNode& x)
{
	Group& group = m_groups[type];
	if(group.m_count + 1 > group.m_nodes.getSize())
	{
		// Need to grow
		U newSize = (group.m_nodes.getSize() != 0)
			? group.m_nodes.getSize() * 2
			: 2;
		group.m_nodes.resize(alloc, newSize);
	}

	group.m_nodes[group.m_count++] = x;
}

//==============================================================================
void VisibilityTestResults::combineWith(SceneFrameAllocator<U8> alloc,
	SArray<VisibilityTestResults*>& results)
{
	ANKI_ASSERT(results.getSize() > 0);

	// Count the visible scene nodes to optimize the allocation of the
	// final result
	Array<U, TYPE_COUNT> counts;
	memset(&counts[0], 0, sizeof(counts));
	for(U i = 0; i < results.getSize(); i++)
	{
		VisibilityTestResults& rez = *results[i];

		for(U t = 0; t < TYPE_COUNT; ++t)
		{
			counts[t] += rez.m_groups[t].m_count;
		}
	}

	// Allocate
	for(U t = 0; t < TYPE_COUNT; ++t)
	{
		if(counts[t] > 0)
		{
			m_groups[t].m_nodes.create(alloc, counts[t]);
			m_groups[t].m_count = counts[t];
		}
	}

	// Combine
	memset(&counts[0], 0, sizeof(counts)); // Re-use
	for(U i = 0; i < results.getSize(); ++i)
	{
		VisibilityTestResults& rez = *results[i];

		for(U t = 0; t < TYPE_COUNT; ++t)
		{
			U copyCount = rez.m_groups[t].m_count;
			if(copyCount > 0)
			{
				memcpy(&m_groups[t].m_nodes[0] + counts[t],
					&rez.m_groups[t].m_nodes[0],
					sizeof(VisibleNode) * copyCount);

				counts[t] += copyCount;
			}
		}
	}
}

//==============================================================================
// doVisibilityTests                                                           =
//==============================================================================

//==============================================================================
Error doVisibilityTests(SceneNode& fsn, SceneGraph& scene, const Renderer& r)
{
	// Do the tests in parallel
	ThreadPool& threadPool = scene._getThreadPool();

	VisibilityShared shared(threadPool.getThreadsCount());
	shared.m_scene = &scene;
	shared.m_r = &r;
	shared.m_frustumsList.pushBack(
		scene.getFrameAllocator(), &fsn.getComponent<FrustumComponent>());

	Array<VisibilityTestTask, ThreadPool::MAX_THREADS> tasks;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_shared = &shared;
		threadPool.assignNewTask(i, &tasks[i]);
	}

	return threadPool.waitForAllThreadsToFinish();
}

} // end namespace anki
