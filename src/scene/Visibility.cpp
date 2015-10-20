// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Visibility.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Sector.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/ReflectionProbe.h>
#include <anki/scene/Light.h>
#include <anki/scene/MoveComponent.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/util/Logger.h>

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

	// Gather the results from all threads
	m_shared->m_barrier.wait();

	if(threadId == 0)
	{
		combineTestResults(testedFrc, threadsCount);
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
	U renderablesSize = 0;
	U lightsSize = 0;
	U lensFlaresSize = 0;
	U reflSize = 0;
	for(U i = 0; i < threadsCount; i++)
	{
		VisibilityTestResults& rez = *m_shared->m_testResults[i];

		renderablesSize += rez.getRenderablesCount();
		lightsSize += rez.getLightsCount();
		lensFlaresSize += rez.getLensFlaresCount();
		reflSize += rez.getReflectionProbeCount();
	}

	// Allocate
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();

	visible->setShapeUpdateTimestamp(m_shared->m_timestamp);

	visible->create(
		alloc,
		renderablesSize,
		lightsSize,
		lensFlaresSize,
		reflSize);

	visible->prepareMerge();

	// Append thread results
	VisibleNode* renderables = visible->getRenderablesBegin();
	VisibleNode* lights = visible->getLightsBegin();
	VisibleNode* lensFlares = visible->getLensFlaresBegin();
	VisibleNode* refl = visible->getReflectionProbesBegin();
	for(U i = 0; i < threadsCount; i++)
	{
		VisibilityTestResults& from = *m_shared->m_testResults[i];

		U rCount = from.getRenderablesCount();
		U lCount = from.getLightsCount();
		U lfCount = from.getLensFlaresCount();
		U reflCount = from.getReflectionProbeCount();

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

		if(reflCount > 0)
		{
			memcpy(refl,
				from.getReflectionProbesBegin(),
				sizeof(VisibleNode) * reflCount);

			refl += reflCount;
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
	U32 lensFlaresReservedSize,
	U32 reflectionProbesReservedSize)
{
	m_groups[RENDERABLES].m_nodes.create(alloc, renderablesReservedSize);
	m_groups[LIGHTS].m_nodes.create(alloc, lightsReservedSize);
	m_groups[FLARES].m_nodes.create(alloc, lensFlaresReservedSize);
	m_groups[REFLECTION_PROBES].m_nodes.create(alloc,
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
void VisibilityTestResults::prepareMerge()
{
	for(U i = 0; i < TYPE_COUNT; ++i)
	{
		Group& group = m_groups[i];
		ANKI_ASSERT(group.m_count == 0);
		group.m_count = group.m_nodes.getSize();
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
