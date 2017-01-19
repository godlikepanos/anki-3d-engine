// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Visibility.h>
#include <anki/scene/VisibilityInternal.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/Sector.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/scene/ReflectionProxyComponent.h>
#include <anki/scene/OccluderComponent.h>
#include <anki/scene/DecalComponent.h>
#include <anki/scene/Light.h>
#include <anki/scene/MoveComponent.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/util/Logger.h>
#include <anki/util/ThreadHive.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

void VisibilityContext::submitNewWork(FrustumComponent& frc, ThreadHive& hive)
{
	// Check enabled and make sure that the results are null (this can happen on multiple on circular viewing)
	if(ANKI_UNLIKELY(!frc.anyVisibilityTestEnabled()))
	{
		return;
	}

	auto alloc = m_scene->getFrameAllocator();

	// Check if this frc was tested before
	{
		LockGuard<Mutex> l(m_mtx);

		// Check if already in the list
		for(const FrustumComponent* x : m_testedFrcs)
		{
			if(x == &frc)
			{
				return;
			}
		}

		// Not there, push it
		m_testedFrcs.pushBack(alloc, &frc);
	}

	// Submit new work
	//

	// Software rasterizer tasks
	SoftwareRasterizer* r = nullptr;
	Array<ThreadHiveDependencyHandle, ThreadHive::MAX_THREADS> rasterizeDeps;
	if(frc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::OCCLUDERS))
	{
		// Gather triangles task
		GatherVisibleTrianglesTask* gather = alloc.newInstance<GatherVisibleTrianglesTask>();
		gather->m_visCtx = this;
		gather->m_frc = &frc;
		gather->m_vertCount = 0;

		r = &gather->m_r;

		ThreadHiveTask gatherTask;
		gatherTask.m_callback = GatherVisibleTrianglesTask::callback;
		gatherTask.m_argument = gather;

		hive.submitTasks(&gatherTask, 1);

		// Rasterize triangles task
		U count = hive.getThreadCount();
		RasterizeTrianglesTask* rasterize = alloc.newArray<RasterizeTrianglesTask>(count);

		Array<ThreadHiveTask, ThreadHive::MAX_THREADS> rastTasks;
		while(count--)
		{
			RasterizeTrianglesTask& rast = rasterize[count];
			rast.m_gatherTask = gather;
			rast.m_taskIdx = count;
			rast.m_taskCount = hive.getThreadCount();

			rastTasks[count].m_callback = RasterizeTrianglesTask::callback;
			rastTasks[count].m_argument = &rast;
			rastTasks[count].m_inDependencies = WeakArray<ThreadHiveDependencyHandle>(&gatherTask.m_outDependency, 1);
		}

		count = hive.getThreadCount();
		hive.submitTasks(&rastTasks[0], count);
		while(count--)
		{
			rasterizeDeps[count] = rastTasks[count].m_outDependency;
		}
	}

	// Gather task
	GatherVisiblesFromSectorsTask* gather = alloc.newInstance<GatherVisiblesFromSectorsTask>();
	gather->m_visCtx = this;
	gather->m_frc = &frc;
	gather->m_r = r;

	ThreadHiveTask gatherTask;
	gatherTask.m_callback = GatherVisiblesFromSectorsTask::callback;
	gatherTask.m_argument = gather;
	if(r)
	{
		gatherTask.m_inDependencies = WeakArray<ThreadHiveDependencyHandle>(&rasterizeDeps[0], hive.getThreadCount());
	}

	hive.submitTasks(&gatherTask, 1);

	// Test tasks
	U testCount = hive.getThreadCount();
	WeakArray<VisibilityTestTask> tests(alloc.newArray<VisibilityTestTask>(testCount), testCount);
	WeakArray<ThreadHiveTask> testTasks(alloc.newArray<ThreadHiveTask>(testCount), testCount);

	for(U i = 0; i < testCount; ++i)
	{
		auto& test = tests[i];
		test.m_visCtx = this;
		test.m_frc = &frc;
		test.m_sectorsCtx = &gather->m_sectorsCtx;
		test.m_taskIdx = i;
		test.m_taskCount = testCount;
		test.m_r = r;

		auto& task = testTasks[i];
		task.m_callback = VisibilityTestTask::callback;
		task.m_argument = &test;
		task.m_inDependencies = WeakArray<ThreadHiveDependencyHandle>(&gatherTask.m_outDependency, 1);
	}

	hive.submitTasks(&testTasks[0], testCount);

	// Combind results task
	CombineResultsTask* combine = alloc.newInstance<CombineResultsTask>();
	combine->m_visCtx = this;
	combine->m_frc = &frc;
	combine->m_tests = tests;

	ThreadHiveTask combineTask;
	combineTask.m_callback = CombineResultsTask::callback;
	combineTask.m_argument = combine;
	combineTask.m_inDependencies =
		WeakArray<ThreadHiveDependencyHandle>(alloc.newArray<ThreadHiveDependencyHandle>(testCount), testCount);
	for(U i = 0; i < testCount; ++i)
	{
		combineTask.m_inDependencies[i] = testTasks[i].m_outDependency;
	}

	hive.submitTasks(&combineTask, 1);
}

void GatherVisibleTrianglesTask::gather()
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_GATHER_TRIANGLES);

	auto alloc = m_visCtx->m_scene->getFrameAllocator();
	m_verts.create(alloc, TRIANGLES_INITIAL_SIZE);
	SceneComponentLists& lists = m_visCtx->m_scene->getSceneComponentLists();

	ANKI_ASSERT(m_vertCount == 0);
	lists.iterateComponents<OccluderComponent>([&](OccluderComponent& comp) {
		if(m_frc->insideFrustum(comp.getBoundingVolume()))
		{
			U32 count, stride;
			const Vec3* it;
			comp.getVertices(it, count, stride);
			while(count--)
			{
				// Grow the array
				if(m_vertCount + 1 > m_verts.getSize())
				{
					m_verts.resize(alloc, m_verts.getSize() * 2);
				}

				m_verts[m_vertCount++] = *it;

				it = reinterpret_cast<const Vec3*>(reinterpret_cast<const U8*>(it) + stride);
			}
		}
	});

	m_r.init(alloc);
	m_r.prepare(m_frc->getViewMatrix(), m_frc->getProjectionMatrix(), 80, 50);

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_GATHER_TRIANGLES);
}

void RasterizeTrianglesTask::rasterize()
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_RASTERIZE);

	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(m_taskIdx, m_taskCount, m_gatherTask->m_vertCount / 3, start, end);

	if(start != end)
	{
		const Vec3* first = &m_gatherTask->m_verts[start * 3];
		U count = (end - start) * 3;
		ANKI_ASSERT(count <= m_gatherTask->m_vertCount);

		m_gatherTask->m_r.draw(&first[0][0], count, sizeof(Vec3));
	}

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_RASTERIZE);
}

void VisibilityTestTask::test(ThreadHive& hive)
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_TEST);

	FrustumComponent& testedFrc = *m_frc;
	ANKI_ASSERT(testedFrc.anyVisibilityTestEnabled());

	SceneNode& testedNode = testedFrc.getSceneNode();
	auto alloc = m_visCtx->m_scene->getFrameAllocator();

	// Init test results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();
	visible->create(alloc);
	m_result = visible;

	Bool wantsRenderComponents =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS);

	Bool wantsLightComponents = testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS);

	Bool wantsFlareComponents =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS);

	Bool wantsShadowCasters = testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::SHADOW_CASTERS);

	Bool wantsReflectionProbes =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::REFLECTION_PROBES);

	Bool wantsReflectionProxies =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES);

	Bool wantsDecals = testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::DECALS);

	// Chose the test range and a few other things
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(m_taskIdx, m_taskCount, m_sectorsCtx->getVisibleSceneNodeCount(), start, end);

	m_sectorsCtx->iterateVisibleSceneNodes(start, end, [&](SceneNode& node) {
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

		LensFlareComponent* lfc = node.tryGetComponent<LensFlareComponent>();
		if(lfc && wantsFlareComponents)
		{
			wantNode = true;
		}

		ReflectionProbeComponent* reflc = node.tryGetComponent<ReflectionProbeComponent>();
		if(reflc && wantsReflectionProbes)
		{
			wantNode = true;
		}

		ReflectionProxyComponent* proxyc = node.tryGetComponent<ReflectionProxyComponent>();
		if(proxyc && wantsReflectionProxies)
		{
			wantNode = true;
		}

		DecalComponent* decalc = node.tryGetComponent<DecalComponent>();
		if(decalc && wantsDecals)
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
		Array<SpatialTemp, MAX_SUB_DRAWCALLS> sps;

		U spIdx = 0;
		U count = 0;
		Error err = node.iterateComponentsOfType<SpatialComponent>([&](SpatialComponent& sp) {
			if(testedFrc.insideFrustum(sp))
			{
				// Inside
				ANKI_ASSERT(spIdx < MAX_U8);
				sps[count++] = SpatialTemp{&sp, static_cast<U8>(spIdx), sp.getSpatialOrigin()};

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
		std::sort(sps.begin(), sps.begin() + count, [origin](const SpatialTemp& a, const SpatialTemp& b) -> Bool {
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
		visibleNode.m_frustumDistanceSquared = (sps[0].m_origin - testedFrc.getFrustumOrigin()).getLengthSquared();

		ANKI_ASSERT(count < MAX_U8);
		visibleNode.m_spatialsCount = count;
		visibleNode.m_spatialIndices = alloc.newArray<U8>(count);

		for(U i = 0; i < count; i++)
		{
			visibleNode.m_spatialIndices[i] = sps[i].m_idx;
		}

		if(rc)
		{
			if(wantsRenderComponents || (wantsShadowCasters && rc->getCastsShadow()))
			{
				visible->moveBack(alloc,
					rc->getMaterial().getForwardShading() ? VisibilityGroupType::RENDERABLES_FS
														  : VisibilityGroupType::RENDERABLES_MS,
					visibleNode);

				if(wantsShadowCasters)
				{
					updateTimestamp(node);
				}
			}
		}

		if(lc && wantsLightComponents)
		{
			// Perform an additional check against the rasterizer
			Bool in;
			if(lc->getShadowEnabled())
			{
				ANKI_ASSERT(spIdx == 1);
				const SpatialComponent& sc = *sps[0].m_sp;
				in = testAgainstRasterizer(sc.getSpatialCollisionShape(), sc.getAabb());
			}
			else
			{
				in = true;
			}

			if(in)
			{
				VisibilityGroupType gt;
				switch(lc->getLightComponentType())
				{
				case LightComponentType::POINT:
					gt = VisibilityGroupType::LIGHTS_POINT;
					break;
				case LightComponentType::SPOT:
					gt = VisibilityGroupType::LIGHTS_SPOT;
					break;
				default:
					ANKI_ASSERT(0);
					gt = VisibilityGroupType::TYPE_COUNT;
				}

				visible->moveBack(alloc, gt, visibleNode);
			}
		}

		if(lfc && wantsFlareComponents)
		{
			visible->moveBack(alloc, VisibilityGroupType::FLARES, visibleNode);
		}

		if(reflc && wantsReflectionProbes)
		{
			ANKI_ASSERT(spIdx == 1);
			const SpatialComponent& sc = *sps[0].m_sp;
			if(testAgainstRasterizer(sc.getSpatialCollisionShape(), sc.getAabb()))
			{
				visible->moveBack(alloc, VisibilityGroupType::REFLECTION_PROBES, visibleNode);
			}
		}

		if(proxyc && wantsReflectionProxies)
		{
			visible->moveBack(alloc, VisibilityGroupType::REFLECTION_PROXIES, visibleNode);
		}

		if(decalc && wantsDecals)
		{
			visible->moveBack(alloc, VisibilityGroupType::DECALS, visibleNode);
		}

		// Add more frustums to the list
		err = node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
			m_visCtx->submitNewWork(frc, hive);
			return ErrorCode::NONE;
		});
		(void)err;
	}); // end for

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_TEST);
}

void VisibilityTestTask::updateTimestamp(const SceneNode& node)
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

	m_timestamp = max(m_timestamp, lastUpdate);
}

void CombineResultsTask::combine()
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);

	auto alloc = m_visCtx->m_scene->getFrameAllocator();

	// Prepare
	Array<VisibilityTestResults*, 32> rezArr;
	Timestamp timestamp = 0;
	for(U i = 0; i < m_tests.getSize(); ++i)
	{
		rezArr[i] = m_tests[i].m_result;
		timestamp = max(timestamp, m_tests[i].m_timestamp);
	}

	WeakArray<VisibilityTestResults*> rez(&rezArr[0], m_tests.getSize());

	// Create the new combined results
	VisibilityTestResults* visible = alloc.newInstance<VisibilityTestResults>();
	visible->combineWith(alloc, rez);
	visible->setShapeUpdateTimestamp(timestamp);

	// Set the frustumable
	m_frc->setVisibilityTestResults(visible);

	// Sort some of the arrays
	DistanceSortFunctor comp;
	std::sort(visible->getBegin(VisibilityGroupType::RENDERABLES_MS),
		visible->getEnd(VisibilityGroupType::RENDERABLES_MS),
		comp);

	std::sort(visible->getBegin(VisibilityGroupType::RENDERABLES_FS),
		visible->getEnd(VisibilityGroupType::RENDERABLES_FS),
		RevDistanceSortFunctor());

	std::sort(visible->getBegin(VisibilityGroupType::REFLECTION_PROBES),
		visible->getEnd(VisibilityGroupType::REFLECTION_PROBES),
		comp);

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);
}

void VisibilityTestResults::create(SceneFrameAllocator<U8> alloc)
{
	m_groups[VisibilityGroupType::RENDERABLES_MS].m_nodes.create(alloc, 64);
	m_groups[VisibilityGroupType::RENDERABLES_FS].m_nodes.create(alloc, 8);
	m_groups[VisibilityGroupType::LIGHTS_POINT].m_nodes.create(alloc, 8);
	m_groups[VisibilityGroupType::LIGHTS_SPOT].m_nodes.create(alloc, 4);
}

void VisibilityTestResults::moveBack(SceneFrameAllocator<U8> alloc, VisibilityGroupType type, VisibleNode& x)
{
	Group& group = m_groups[type];
	if(group.m_count + 1 > group.m_nodes.getSize())
	{
		// Need to grow
		U newSize = (group.m_nodes.getSize() != 0) ? group.m_nodes.getSize() * 2 : 2;
		group.m_nodes.resize(alloc, newSize);
	}

	group.m_nodes[group.m_count++] = x;
}

void VisibilityTestResults::combineWith(SceneFrameAllocator<U8> alloc, WeakArray<VisibilityTestResults*>& results)
{
	ANKI_ASSERT(results.getSize() > 0);

	// Count the visible scene nodes to optimize the allocation of the
	// final result
	Array<U, U(VisibilityGroupType::TYPE_COUNT)> counts;
	memset(&counts[0], 0, sizeof(counts));
	for(U i = 0; i < results.getSize(); ++i)
	{
		VisibilityTestResults& rez = *results[i];

		for(VisibilityGroupType t = VisibilityGroupType::FIRST; t < VisibilityGroupType::TYPE_COUNT; ++t)
		{
			counts[t] += rez.m_groups[t].m_count;
		}
	}

	// Allocate
	for(VisibilityGroupType t = VisibilityGroupType::FIRST; t < VisibilityGroupType::TYPE_COUNT; ++t)
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

		for(VisibilityGroupType t = VisibilityGroupType::FIRST; t < VisibilityGroupType::TYPE_COUNT; ++t)
		{
			U copyCount = rez.m_groups[t].m_count;
			if(copyCount > 0)
			{
				memcpy(
					&m_groups[t].m_nodes[0] + counts[t], &rez.m_groups[t].m_nodes[0], sizeof(VisibleNode) * copyCount);

				counts[t] += copyCount;
			}
		}
	}
}

void doVisibilityTests(SceneNode& fsn, SceneGraph& scene, const Renderer& r)
{
	ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_TESTS);

	ThreadHive& hive = scene.getThreadHive();
	scene.getSectorGroup().prepareForVisibilityTests();

	VisibilityContext ctx;
	ctx.m_scene = &scene;
	ctx.submitNewWork(fsn.getComponent<FrustumComponent>(), hive);

	hive.waitAllTasks();
	ctx.m_testedFrcs.destroy(scene.getFrameAllocator());

	ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_TESTS);
}

} // end namespace anki
