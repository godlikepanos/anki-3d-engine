// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Visibility.h>
#include <anki/scene/VisibilityInternal.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SectorNode.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/components/ReflectionProxyComponent.h>
#include <anki/scene/components/OccluderComponent.h>
#include <anki/scene/components/DecalComponent.h>
#include <anki/scene/LightNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/util/Logger.h>
#include <anki/util/ThreadHive.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

void VisibilityContext::submitNewWork(FrustumComponent& frc, RenderQueue& rqueue, ThreadHive& hive)
{
	// Check enabled and make sure that the results are null (this can happen on multiple on circular viewing)
	if(ANKI_UNLIKELY(!frc.anyVisibilityTestEnabled()))
	{
		return;
	}

	rqueue.m_cameraTransform = Mat4(frc.getFrustum().getTransform());
	rqueue.m_viewMatrix = frc.getViewMatrix();
	rqueue.m_projectionMatrix = frc.getProjectionMatrix();
	rqueue.m_viewProjectionMatrix = frc.getViewProjectionMatrix();
	rqueue.m_cameraNear = frc.getFrustum().getNear();
	rqueue.m_cameraFar = frc.getFrustum().getFar();

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

	// Gather visibles from sector
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
	combine->m_results = &rqueue;
	combine->m_tests = tests;
	combine->m_swRast = r;

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
	ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_GATHER_TRIANGLES);

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
}

void RasterizeTrianglesTask::rasterize()
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_RASTERIZE);

	const U totalVertCount = m_gatherTask->m_vertCount;

	U32 idx;
	while((idx = m_gatherTask->m_rasterizedVertCount.fetchAdd(3)) < totalVertCount)
	{
		m_gatherTask->m_r.draw(&m_gatherTask->m_verts[idx][0], 3, sizeof(Vec3), false);
	}
}

void VisibilityTestTask::test(ThreadHive& hive)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_TEST);

	FrustumComponent& testedFrc = *m_frc;
	ANKI_ASSERT(testedFrc.anyVisibilityTestEnabled());

	SceneNode& testedNode = testedFrc.getSceneNode();
	auto alloc = m_visCtx->m_scene->getFrameAllocator();

	m_timestamp = testedNode.getComponentMaxTimestamp();

	const Bool wantsRenderComponents =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS);

	const Bool wantsLightComponents =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS);

	const Bool wantsFlareComponents =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS);

	const Bool wantsShadowCasters =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::SHADOW_CASTERS);

	const Bool wantsReflectionProbes =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::REFLECTION_PROBES);

	const Bool wantsReflectionProxies =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES);

	const Bool wantsDecals = testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::DECALS);

	const Bool wantsEarlyZ =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::EARLY_Z) && m_visCtx->m_earlyZDist > 0.0f;

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
			if(testedFrc.insideFrustum(sp) && testAgainstRasterizer(sp.getSpatialCollisionShape(), sp.getAabb()))
			{
				// Inside
				ANKI_ASSERT(spIdx < MAX_U8);
				sps[count++] = SpatialTemp{&sp, static_cast<U8>(spIdx), sp.getSpatialOrigin()};

				sp.setVisibleByCamera(true);
			}

			++spIdx;

			return Error::NONE;
		});
		(void)err;

		if(ANKI_UNLIKELY(count == 0))
		{
			return;
		}

		ANKI_ASSERT(count == 1 && "TODO: Support sub-spatials");

		// Sort sub-spatials
		Vec4 origin = testedFrc.getFrustumOrigin();
		std::sort(sps.begin(), sps.begin() + count, [origin](const SpatialTemp& a, const SpatialTemp& b) -> Bool {
			const Vec4& spa = a.m_origin;
			const Vec4& spb = b.m_origin;

			F32 dist0 = origin.getDistanceSquared(spa);
			F32 dist1 = origin.getDistanceSquared(spb);

			return dist0 < dist1;
		});

		WeakArray<RenderQueue> nextQueues;

		if(rc)
		{
			if(wantsRenderComponents || (wantsShadowCasters && rc->getCastsShadow()))
			{
				RenderableQueueElement* el;
				if(rc->getMaterial().isForwardShading())
				{
					el = m_result.m_forwardShadingRenderables.newElement(alloc);
				}
				else
				{
					el = m_result.m_renderables.newElement(alloc);
				}

				rc->setupRenderableQueueElement(*el);

				// Compute distance from the frustum
				const Plane& nearPlane = testedFrc.getFrustum().getPlanesWorldSpace()[FrustumPlaneType::NEAR];
				el->m_distanceFromCamera = max(0.0f, sps[0].m_sp->getAabb().testPlane(nearPlane));

				if(wantsEarlyZ && el->m_distanceFromCamera < m_visCtx->m_earlyZDist
					&& !rc->getMaterial().isForwardShading())
				{
					RenderableQueueElement* el2 = m_result.m_earlyZRenderables.newElement(alloc);
					*el2 = *el;
				}
			}
		}

		if(lc && wantsLightComponents)
		{
			switch(lc->getLightComponentType())
			{
			case LightComponentType::POINT:
			{
				PointLightQueueElement* el = m_result.m_pointLights.newElement(alloc);
				lc->setupPointLightQueueElement(*el);

				if(lc->getShadowEnabled())
				{
					RenderQueue* a = alloc.newArray<RenderQueue>(6);
					nextQueues = WeakArray<RenderQueue>(a, 6);

					el->m_shadowRenderQueues[0] = &nextQueues[0];
					el->m_shadowRenderQueues[1] = &nextQueues[1];
					el->m_shadowRenderQueues[2] = &nextQueues[2];
					el->m_shadowRenderQueues[3] = &nextQueues[3];
					el->m_shadowRenderQueues[4] = &nextQueues[4];
					el->m_shadowRenderQueues[5] = &nextQueues[5];

					U32* p = m_result.m_shadowPointLights.newElement(alloc);
					*p = m_result.m_pointLights.m_elementCount - 1;
				}
				else
				{
					zeroMemory(el->m_shadowRenderQueues);
				}

				break;
			}
			case LightComponentType::SPOT:
			{
				SpotLightQueueElement* el = m_result.m_spotLights.newElement(alloc);
				lc->setupSpotLightQueueElement(*el);

				if(lc->getShadowEnabled())
				{
					RenderQueue* a = alloc.newInstance<RenderQueue>();
					nextQueues = WeakArray<RenderQueue>(a, 1);
					el->m_shadowRenderQueue = a;

					U32* p = m_result.m_shadowSpotLights.newElement(alloc);
					*p = m_result.m_spotLights.m_elementCount - 1;
				}
				else
				{
					el->m_shadowRenderQueue = nullptr;
				}

				break;
			}
			default:
				ANKI_ASSERT(0);
			}
		}

		if(lfc && wantsFlareComponents)
		{
			LensFlareQueueElement* el = m_result.m_lensFlares.newElement(alloc);
			lfc->setupLensFlareQueueElement(*el);
		}

		if(reflc && wantsReflectionProbes)
		{
			ReflectionProbeQueueElement* el = m_result.m_reflectionProbes.newElement(alloc);
			reflc->setupReflectionProbeQueueElement(*el);

			if(reflc->getMarkedForRendering())
			{
				RenderQueue* a = alloc.newArray<RenderQueue>(6);
				nextQueues = WeakArray<RenderQueue>(a, 6);

				el->m_renderQueues[0] = &nextQueues[0];
				el->m_renderQueues[1] = &nextQueues[1];
				el->m_renderQueues[2] = &nextQueues[2];
				el->m_renderQueues[3] = &nextQueues[3];
				el->m_renderQueues[4] = &nextQueues[4];
				el->m_renderQueues[5] = &nextQueues[5];
			}
			else
			{
				el->m_renderQueues = {{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}};
			}
		}

		if(proxyc && wantsReflectionProxies)
		{
			ANKI_ASSERT(!"TODO");
		}

		if(decalc && wantsDecals)
		{
			DecalQueueElement* el = m_result.m_decals.newElement(alloc);
			decalc->setupDecalQueueElement(*el);
		}

		// Add more frustums to the list
		if(nextQueues.getSize() > 0)
		{
			count = 0;
			err = node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
				m_visCtx->submitNewWork(frc, nextQueues[count++], hive);
				return Error::NONE;
			});
			(void)err;
		}

		// Update timestamp
		m_timestamp = max(m_timestamp, node.getComponentMaxTimestamp());
	}); // end for
}

void CombineResultsTask::combine()
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_COMBINE_RESULTS);

	auto alloc = m_visCtx->m_scene->getFrameAllocator();

	m_results->m_shadowRenderablesLastUpdateTimestamp = 0;
	for(U i = 0; i < m_tests.getSize(); ++i)
	{
		m_results->m_shadowRenderablesLastUpdateTimestamp =
			max(m_results->m_shadowRenderablesLastUpdateTimestamp, m_tests[i].m_timestamp);
	}
	ANKI_ASSERT(m_results->m_shadowRenderablesLastUpdateTimestamp);

#define ANKI_VIS_COMBINE(t_, member_) \
	{ \
		Array<TRenderQueueElementStorage<t_>, 64> subStorages; \
		for(U i = 0; i < m_tests.getSize(); ++i) \
		{ \
			subStorages[i] = m_tests[i].m_result.member_; \
		} \
		combineQueueElements<t_>(alloc, \
			WeakArray<TRenderQueueElementStorage<t_>>(&subStorages[0], m_tests.getSize()), \
			nullptr, \
			m_results->member_, \
			nullptr); \
	}

#define ANKI_VIS_COMBINE_AND_PTR(t_, member_, ptrMember_) \
	{ \
		Array<TRenderQueueElementStorage<t_>, 64> subStorages; \
		Array<TRenderQueueElementStorage<U32>, 64> ptrSubStorages; \
		for(U i = 0; i < m_tests.getSize(); ++i) \
		{ \
			subStorages[i] = m_tests[i].m_result.member_; \
			ptrSubStorages[i] = m_tests[i].m_result.ptrMember_; \
		} \
		WeakArray<TRenderQueueElementStorage<U32>> arr(&ptrSubStorages[0], m_tests.getSize()); \
		combineQueueElements<t_>(alloc, \
			WeakArray<TRenderQueueElementStorage<t_>>(&subStorages[0], m_tests.getSize()), \
			&arr, \
			m_results->member_, \
			&m_results->ptrMember_); \
	}

	ANKI_VIS_COMBINE(RenderableQueueElement, m_renderables);
	ANKI_VIS_COMBINE(RenderableQueueElement, m_earlyZRenderables);
	ANKI_VIS_COMBINE(RenderableQueueElement, m_forwardShadingRenderables);
	ANKI_VIS_COMBINE_AND_PTR(PointLightQueueElement, m_pointLights, m_shadowPointLights);
	ANKI_VIS_COMBINE_AND_PTR(SpotLightQueueElement, m_spotLights, m_shadowSpotLights);
	ANKI_VIS_COMBINE(ReflectionProbeQueueElement, m_reflectionProbes);
	ANKI_VIS_COMBINE(LensFlareQueueElement, m_lensFlares);
	ANKI_VIS_COMBINE(DecalQueueElement, m_decals);

#undef ANKI_VIS_COMBINE
#undef ANKI_VIS_COMBINE_AND_PTR

#if ANKI_EXTRA_CHECKS
	for(PointLightQueueElement* light : m_results->m_shadowPointLights)
	{
		ANKI_ASSERT(light->hasShadow());
	}

	for(SpotLightQueueElement* light : m_results->m_shadowSpotLights)
	{
		ANKI_ASSERT(light->hasShadow());
	}
#endif

	// Sort some of the arrays
	std::sort(
		m_results->m_renderables.getBegin(), m_results->m_renderables.getEnd(), MaterialDistanceSortFunctor(20.0f));

	std::sort(m_results->m_earlyZRenderables.getBegin(),
		m_results->m_earlyZRenderables.getEnd(),
		DistanceSortFunctor<RenderableQueueElement>());

	std::sort(m_results->m_forwardShadingRenderables.getBegin(),
		m_results->m_forwardShadingRenderables.getEnd(),
		RevDistanceSortFunctor<RenderableQueueElement>());

	if(m_swRast)
	{
		m_swRast->~SoftwareRasterizer();
	}
}

template<typename T>
void CombineResultsTask::combineQueueElements(SceneFrameAllocator<U8>& alloc,
	WeakArray<TRenderQueueElementStorage<T>> subStorages,
	WeakArray<TRenderQueueElementStorage<U32>>* ptrSubStorages,
	WeakArray<T>& combined,
	WeakArray<T*>* ptrCombined)
{
	U totalElCount = subStorages[0].m_elementCount;
	U biggestSubStorageIdx = 0;
	for(U i = 1; i < subStorages.getSize(); ++i)
	{
		totalElCount += subStorages[i].m_elementCount;

		if(subStorages[i].m_elementStorage > subStorages[biggestSubStorageIdx].m_elementStorage)
		{
			biggestSubStorageIdx = i;
		}
	}

	if(totalElCount == 0)
	{
		return;
	}

	// Count ptrSubStorage elements
	T** ptrIt = nullptr;
	if(ptrSubStorages != nullptr)
	{
		ANKI_ASSERT(ptrCombined);
		U ptrTotalElCount = (*ptrSubStorages)[0].m_elementCount;

		for(U i = 1; i < ptrSubStorages->getSize(); ++i)
		{
			ptrTotalElCount += (*ptrSubStorages)[i].m_elementCount;
		}

		// Create the new storage
		if(ptrTotalElCount > 0)
		{
			ptrIt = alloc.newArray<T*>(ptrTotalElCount);
			*ptrCombined = WeakArray<T*>(ptrIt, ptrTotalElCount);
		}
	}

	T* it;
	if(totalElCount > subStorages[biggestSubStorageIdx].m_elementStorage)
	{
		// Can't reuse any of the existing storage, will allocate a brand new one

		it = alloc.newArray<T>(totalElCount);
		biggestSubStorageIdx = MAX_U;

		combined = WeakArray<T>(it, totalElCount);
	}
	else
	{
		// Will reuse existing storage

		it = subStorages[biggestSubStorageIdx].m_elements + subStorages[biggestSubStorageIdx].m_elementCount;

		combined = WeakArray<T>(subStorages[biggestSubStorageIdx].m_elements, totalElCount);
	}

	for(U i = 0; i < subStorages.getSize(); ++i)
	{
		if(subStorages[i].m_elementCount == 0)
		{
			continue;
		}

		// Copy the pointers
		if(ptrIt)
		{
			T* base = (i != biggestSubStorageIdx) ? it : subStorages[biggestSubStorageIdx].m_elements;

			for(U x = 0; x < (*ptrSubStorages)[i].m_elementCount; ++x)
			{
				ANKI_ASSERT((*ptrSubStorages)[i].m_elements[x] < subStorages[i].m_elementCount);

				*ptrIt = base + (*ptrSubStorages)[i].m_elements[x];

				++ptrIt;
			}

			ANKI_ASSERT(ptrIt <= ptrCombined->getEnd());
		}

		// Copy the elements
		if(i != biggestSubStorageIdx)
		{
			memcpy(it, subStorages[i].m_elements, sizeof(T) * subStorages[i].m_elementCount);
			it += subStorages[i].m_elementCount;
		}
	}
}

void doVisibilityTests(SceneNode& fsn, SceneGraph& scene, RenderQueue& rqueue)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_TESTS);

	ThreadHive& hive = scene.getThreadHive();
	scene.getSectorGroup().prepareForVisibilityTests();

	VisibilityContext ctx;
	ctx.m_scene = &scene;
	ctx.m_earlyZDist = scene.getEarlyZDistance();
	ctx.submitNewWork(fsn.getComponent<FrustumComponent>(), rqueue, hive);

	hive.waitAllTasks();
	ctx.m_testedFrcs.destroy(scene.getFrameAllocator());
}

} // end namespace anki
