// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Sort spatial scene nodes on distance
class DistanceSortFunctor
{
public:
	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.m_node && b.m_node);
		return a.m_frustumDistanceSquared < b.m_frustumDistanceSquared;
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

/// Contains all the info of a single visibility test on a FrustumComponent.
class SingleVisTest : public NonCopyable
{
public:
	FrustumComponent* m_frc;
	Array<VisibilityTestResults*, ThreadPool::MAX_THREADS> m_threadResults;
	Timestamp m_timestamp = 0;
	SpinLock m_timestampLock;
	SectorGroupVisibilityTestsContext m_sectorsCtx;

	SingleVisTest(FrustumComponent* frc)
		: m_frc(frc)
	{
		ANKI_ASSERT(m_frc);
		memset(&m_threadResults[0], 0, sizeof(m_threadResults));
	}
};

/// Data common for all tasks.
class VisibilityContext
{
public:
	SceneGraph* m_scene = nullptr;
	const Renderer* m_r = nullptr;
	Barrier m_barrier;

	ListAuto<SingleVisTest*> m_pendingTests; ///< Frustums to test
	ListAuto<SingleVisTest*> m_completedTests;
	U32 m_completedTestsCount = 0;
	U32 m_testIdCounter = 1; ///< 1 because the main thread already added one
	SpinLock m_lock;

	Atomic<U32> m_listRaceAtomic = {0};

	VisibilityContext(U threadCount, SceneGraph* scene)
		: m_scene(scene)
		, m_barrier(threadCount)
		, m_pendingTests(m_scene->getFrameAllocator())
		, m_completedTests(m_scene->getFrameAllocator())
	{
	}
};

/// Task.
class VisibilityTestTask : public ThreadPool::Task
{
public:
	VisibilityContext* m_ctx = nullptr;

	/// Test a frustum component
	void test(SingleVisTest& testInfo, U32 threadId, PtrSize threadCount);

	void combineTestResults(SingleVisTest& testInfo, PtrSize threadCount);

	/// Do the tests
	Error operator()(U32 threadId, PtrSize threadCount) final
	{
		auto alloc = m_ctx->m_scene->getFrameAllocator();

		ANKI_ASSERT(!m_ctx->m_pendingTests.isEmpty());
		SingleVisTest* testInfo = m_ctx->m_pendingTests.getFront();
		ANKI_ASSERT(testInfo);
		do
		{
			// Test
			test(*testInfo, threadId, threadCount);

			// Sync
			m_ctx->m_barrier.wait();

			// Get something to test next
			{
				LockGuard<SpinLock> lock(m_ctx->m_lock);

				if(!m_ctx->m_pendingTests.isEmpty())
				{
					if(testInfo == m_ctx->m_pendingTests.getFront())
					{
						// Push it to the completed
						m_ctx->m_completedTests.pushBack(testInfo);
						++m_ctx->m_completedTestsCount;
						m_ctx->m_pendingTests.popFront();

						testInfo = (m_ctx->m_pendingTests.isEmpty())
							? nullptr
							: m_ctx->m_pendingTests.getFront();
					}
					else
					{
						testInfo = m_ctx->m_pendingTests.getFront();	
					}
				}
				else
				{
					testInfo = nullptr;
				}
			}
		} while(testInfo);

		// Sort and combind the results
		PtrSize start, end;
		choseStartEnd(
			threadId, threadCount, m_ctx->m_completedTestsCount, start, end);
		if(start != end)
		{
			auto it = m_ctx->m_completedTests.getBegin() + start;
			for(U i = start; i < end; ++i)
			{
				ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);

				combineTestResults(*(*it), threadCount);
				++it;

				ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);
			}
		}

		return ErrorCode::NONE;
	}

	/// Update the timestamp if the node moved or changed its shape.
	static void updateTimestamp(const SceneNode& node, SingleVisTest& testInfo)
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

		LockGuard<SpinLock> lock(testInfo.m_timestampLock);
		testInfo.m_timestamp = max(testInfo.m_timestamp, lastUpdate);
	}
};

//==============================================================================
void VisibilityTestTask::test(
	SingleVisTest& testInfo, U32 threadId, PtrSize threadCount)
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_TEST);

	ANKI_ASSERT(testInfo.m_frc);
	FrustumComponent& testedFrc = *testInfo.m_frc;
	ANKI_ASSERT(testedFrc.anyVisibilityTestEnabled());

	SceneNode& testedNode = testedFrc.getSceneNode();
	auto alloc = m_ctx->m_scene->getFrameAllocator();

	// Init test results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();

	visible->create(alloc);

	testInfo.m_threadResults[threadId] = visible;

	List<SceneNode*> frustumsList;

	Bool wantsRenderComponents = testedFrc.visibilityTestsEnabled(
		FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS);

	Bool wantsLightComponents = testedFrc.visibilityTestsEnabled(
		FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS);

	Bool wantsFlareComponents = testedFrc.visibilityTestsEnabled(
		FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS);

	Bool wantsShadowCasters = testedFrc.visibilityTestsEnabled(
		FrustumComponentVisibilityTestFlag::SHADOW_CASTERS);

	Bool wantsReflectionProbes = testedFrc.visibilityTestsEnabled(
		FrustumComponentVisibilityTestFlag::REFLECTION_PROBES);

	Bool wantsReflectionProxies = testedFrc.visibilityTestsEnabled(
		FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES);

	// Chose the test range and a few other things
	PtrSize start, end;
	choseStartEnd(threadId,
		threadCount,
		testInfo.m_sectorsCtx.getVisibleSceneNodeCount(),
		start,
		end);

	testInfo.m_sectorsCtx.iterateVisibleSceneNodes(
		start, end, [&](SceneNode& node) {
			// Skip if it is the same
			if(ANKI_UNLIKELY(&testedNode == &node))
			{
				return;
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

			LensFlareComponent* lfc =
				node.tryGetComponent<LensFlareComponent>();
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
				return;
			}

			// Test all spatial components of that node
			struct SpatialTemp
			{
				SpatialComponent* m_sp;
				U8 m_idx;
				Vec4 m_origin;
			};
			Array<SpatialTemp, ANKI_GL_MAX_SUB_DRAWCALLS> sps;

			U spIdx = 0;
			U count = 0;
			Error err = node.iterateComponentsOfType<SpatialComponent>(
				[&](SpatialComponent& sp) {
					if(testedFrc.insideFrustum(sp))
					{
						// Inside
						ANKI_ASSERT(spIdx < MAX_U8);
						sps[count++] = SpatialTemp{
							&sp, static_cast<U8>(spIdx), sp.getSpatialOrigin()};

						sp.setVisibleByCamera(true);
					}

					++spIdx;

					return ErrorCode::NONE;
				});
			(void)err;

			if(ANKI_UNLIKELY(count == 0))
			{
				return;
			}

			// Sort sub-spatials
			Vec4 origin = testedFrc.getFrustumOrigin();
			std::sort(sps.begin(),
				sps.begin() + count,
				[origin](const SpatialTemp& a, const SpatialTemp& b) -> Bool {
					const Vec4& spa = a.m_origin;
					const Vec4& spb = b.m_origin;

					F32 dist0 = origin.getDistanceSquared(spa);
					F32 dist1 = origin.getDistanceSquared(spb);

					return dist0 < dist1;
				});

			// Update the visibleNode
			VisibleNode visibleNode;
			visibleNode.m_node = &node;

			// Compute distance from the frustum
			visibleNode.m_frustumDistanceSquared =
				(sps[0].m_origin - testedFrc.getFrustumOrigin())
					.getLengthSquared();

			ANKI_ASSERT(count < MAX_U8);
			visibleNode.m_spatialsCount = count;
			visibleNode.m_spatialIndices = alloc.newArray<U8>(count);

			for(U i = 0; i < count; i++)
			{
				visibleNode.m_spatialIndices[i] = sps[i].m_idx;
			}

			if(rc)
			{
				if(wantsRenderComponents
					|| (wantsShadowCasters && rc->getCastsShadow()))
				{
					visible->moveBack(alloc,
						rc->getMaterial().getForwardShading()
							? VisibilityGroupType::RENDERABLES_FS
							: VisibilityGroupType::RENDERABLES_MS,
						visibleNode);

					if(wantsShadowCasters)
					{
						updateTimestamp(node, testInfo);
					}
				}
			}

			if(lc && wantsLightComponents)
			{
				VisibilityGroupType gt;
				switch(lc->getLightType())
				{
				case LightComponent::LightType::POINT:
					gt = VisibilityGroupType::LIGHTS_POINT;
					break;
				case LightComponent::LightType::SPOT:
					gt = VisibilityGroupType::LIGHTS_SPOT;
					break;
				default:
					ANKI_ASSERT(0);
					gt = VisibilityGroupType::TYPE_COUNT;
				}

				visible->moveBack(alloc, gt, visibleNode);
			}

			if(lfc && wantsFlareComponents)
			{
				visible->moveBack(
					alloc, VisibilityGroupType::FLARES, visibleNode);
			}

			if(reflc && wantsReflectionProbes)
			{
				visible->moveBack(
					alloc, VisibilityGroupType::REFLECTION_PROBES, visibleNode);
			}

			if(proxyc && wantsReflectionProxies)
			{
				visible->moveBack(alloc,
					VisibilityGroupType::REFLECTION_PROXIES,
					visibleNode);
			}

			// Add more frustums to the list
			err = node.iterateComponentsOfType<FrustumComponent>([&](
				FrustumComponent& frc) {
				SingleVisTest* t = nullptr;
				U testId = MAX_U;

				// Check enabled and make sure that the results are null (this
				// can happen on multiple on circular viewing)
				if(frc.anyVisibilityTestEnabled())
				{
					LockGuard<SpinLock> l(m_ctx->m_lock);

					// Check if already in the list
					Bool alreadyThere = false;
					for(const SingleVisTest* x : m_ctx->m_pendingTests)
					{
						if(x->m_frc == &frc)
						{
							alreadyThere = true;
							break;
						}
					}

					if(!alreadyThere)
					{
						for(const SingleVisTest* x : m_ctx->m_completedTests)
						{
							if(x->m_frc == &frc)
							{
								alreadyThere = true;
								break;
							}
						}
					}

					if(!alreadyThere)
					{
						testId = m_ctx->m_testIdCounter++;
						t = alloc.newInstance<SingleVisTest>(&frc);
						m_ctx->m_pendingTests.pushBack(t);
					}
				};

				// Do that outside the lock
				if(t)
				{
					m_ctx->m_scene->getSectorGroup().findVisibleNodes(frc,
						testId,
						t->m_sectorsCtx);
				}
				return ErrorCode::NONE;
			});
			(void)err;
		}); // end for

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_TEST);
}

//==============================================================================
void VisibilityTestTask::combineTestResults(
	SingleVisTest& testInfo, PtrSize threadCount)
{
	auto alloc = m_ctx->m_scene->getFrameAllocator();

	// Prepare
	Array<VisibilityTestResults*, ThreadPool::MAX_THREADS> results;
	for(U i = 0; i < threadCount; i++)
	{
		results[i] = testInfo.m_threadResults[i];
	}
	SArray<VisibilityTestResults*> rez(&results[0], threadCount);

	// Create the new combined results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();
	visible->combineWith(alloc, rez);
	visible->setShapeUpdateTimestamp(testInfo.m_timestamp);

	// Set the frustumable
	testInfo.m_frc->setVisibilityTestResults(visible);

	// Sort some of the arrays
	DistanceSortFunctor comp;
	std::sort(visible->getBegin(VisibilityGroupType::RENDERABLES_MS),
		visible->getEnd(VisibilityGroupType::RENDERABLES_MS),
		comp);

	// TODO: Reverse the sort
	std::sort(visible->getBegin(VisibilityGroupType::RENDERABLES_FS),
		visible->getEnd(VisibilityGroupType::RENDERABLES_FS),
		comp);

	std::sort(visible->getBegin(VisibilityGroupType::REFLECTION_PROBES),
		visible->getEnd(VisibilityGroupType::REFLECTION_PROBES),
		comp);
}

//==============================================================================
// VisibilityTestResults                                                       =
//==============================================================================

//==============================================================================
void VisibilityTestResults::create(SceneFrameAllocator<U8> alloc)
{
	m_groups[VisibilityGroupType::RENDERABLES_MS].m_nodes.create(alloc, 64);
	m_groups[VisibilityGroupType::RENDERABLES_FS].m_nodes.create(alloc, 8);
	m_groups[VisibilityGroupType::LIGHTS_POINT].m_nodes.create(alloc, 8);
	m_groups[VisibilityGroupType::LIGHTS_SPOT].m_nodes.create(alloc, 4);
}

//==============================================================================
void VisibilityTestResults::moveBack(
	SceneFrameAllocator<U8> alloc, VisibilityGroupType type, VisibleNode& x)
{
	Group& group = m_groups[type];
	if(group.m_count + 1 > group.m_nodes.getSize())
	{
		// Need to grow
		U newSize =
			(group.m_nodes.getSize() != 0) ? group.m_nodes.getSize() * 2 : 2;
		group.m_nodes.resize(alloc, newSize);
	}

	group.m_nodes[group.m_count++] = x;
}

//==============================================================================
void VisibilityTestResults::combineWith(
	SceneFrameAllocator<U8> alloc, SArray<VisibilityTestResults*>& results)
{
	ANKI_ASSERT(results.getSize() > 0);

	// Count the visible scene nodes to optimize the allocation of the
	// final result
	Array<U, U(VisibilityGroupType::TYPE_COUNT)> counts;
	memset(&counts[0], 0, sizeof(counts));
	for(U i = 0; i < results.getSize(); ++i)
	{
		VisibilityTestResults& rez = *results[i];

		for(VisibilityGroupType t = VisibilityGroupType::FIRST;
			t < VisibilityGroupType::TYPE_COUNT;
			++t)
		{
			counts[t] += rez.m_groups[t].m_count;
		}
	}

	// Allocate
	for(VisibilityGroupType t = VisibilityGroupType::FIRST;
		t < VisibilityGroupType::TYPE_COUNT;
		++t)
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

		for(VisibilityGroupType t = VisibilityGroupType::FIRST;
			t < VisibilityGroupType::TYPE_COUNT;
			++t)
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

	scene.getSectorGroup().prepareForVisibilityTests();

	VisibilityContext ctx(threadPool.getThreadsCount(), &scene);
	ctx.m_r = &r;

	SingleVisTest* t = scene.getFrameAllocator().newInstance<SingleVisTest>(
		&fsn.getComponent<FrustumComponent>());
	ctx.m_pendingTests.pushBack(t);
	scene.getSectorGroup().findVisibleNodes(
		fsn.getComponent<FrustumComponent>(),
		0,
		t->m_sectorsCtx);

	Array<VisibilityTestTask, ThreadPool::MAX_THREADS> tasks;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_ctx = &ctx;
		threadPool.assignNewTask(i, &tasks[i]);
	}

	Error err = threadPool.waitForAllThreadsToFinish();
	ANKI_ASSERT(ctx.m_pendingTests.isEmpty());
	return err;
}

} // end namespace anki
