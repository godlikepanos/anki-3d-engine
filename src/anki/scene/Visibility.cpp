// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/VisibilityInternal.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/components/OccluderComponent.h>
#include <anki/scene/components/DecalComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/FogDensityComponent.h>
#include <anki/scene/components/LightComponent.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/util/Logger.h>
#include <anki/util/ThreadHive.h>

namespace anki
{

void VisibilityContext::submitNewWork(const FrustumComponent& frc, RenderQueue& rqueue, ThreadHive& hive)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_SUBMIT_WORK);

	// Check enabled and make sure that the results are null (this can happen on multiple on circular viewing)
	if(ANKI_UNLIKELY(!frc.anyVisibilityTestEnabled()))
	{
		return;
	}

	rqueue.m_cameraTransform = Mat4(frc.getFrustum().getTransform());
	rqueue.m_viewMatrix = frc.getViewMatrix();
	rqueue.m_projectionMatrix = frc.getProjectionMatrix();
	rqueue.m_viewProjectionMatrix = frc.getViewProjectionMatrix();
	rqueue.m_previousViewProjectionMatrix = frc.getPreviousViewProjectionMatrix();
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

	// Prepare the ctx
	FrustumVisibilityContext* frcCtx = alloc.newInstance<FrustumVisibilityContext>();
	frcCtx->m_visCtx = this;
	frcCtx->m_frc = &frc;
	frcCtx->m_queueViews.create(alloc, hive.getThreadCount());
	frcCtx->m_visTestsSignalSem = hive.newSemaphore(1);
	frcCtx->m_renderQueue = &rqueue;

	// Submit new work
	//

	// Software rasterizer task
	ThreadHiveSemaphore* prepareRasterizerSem = nullptr;
	if(frc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::OCCLUDERS) && frc.hasCoverageBuffer())
	{
		// Gather triangles task
		ThreadHiveTask fillDepthTask = ANKI_THREAD_HIVE_TASK({ self->fill(); },
			alloc.newInstance<FillRasterizerWithCoverageTask>(frcCtx),
			nullptr,
			hive.newSemaphore(1));

		hive.submitTasks(&fillDepthTask, 1);

		prepareRasterizerSem = fillDepthTask.m_signalSemaphore;
	}

	if(frc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::OCCLUDERS))
	{
		rqueue.m_fillCoverageBufferCallback = FrustumComponent::fillCoverageBufferCallback;
		rqueue.m_fillCoverageBufferCallbackUserData = static_cast<void*>(const_cast<FrustumComponent*>(&frc));
	}

	// Gather visibles from the octree. No need to signal anything because it will spawn new tasks
	ThreadHiveTask gatherTask = ANKI_THREAD_HIVE_TASK({ self->gather(hive); },
		alloc.newInstance<GatherVisiblesFromOctreeTask>(frcCtx),
		prepareRasterizerSem,
		nullptr);
	hive.submitTasks(&gatherTask, 1);

	// Combind results task
	ANKI_ASSERT(frcCtx->m_visTestsSignalSem);
	ThreadHiveTask combineTask = ANKI_THREAD_HIVE_TASK(
		{ self->combine(); }, alloc.newInstance<CombineResultsTask>(frcCtx), frcCtx->m_visTestsSignalSem, nullptr);
	hive.submitTasks(&combineTask, 1);
}

void FillRasterizerWithCoverageTask::fill()
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_FILL_DEPTH);

	auto alloc = m_frcCtx->m_visCtx->m_scene->getFrameAllocator();

	// Get the C-Buffer
	ConstWeakArray<F32> depthBuff;
	U32 width;
	U32 height;
	m_frcCtx->m_frc->getCoverageBufferInfo(depthBuff, width, height);
	ANKI_ASSERT(width > 0 && height > 0 && depthBuff.getSize() > 0);

	// Init the rasterizer
	m_frcCtx->m_r = alloc.newInstance<SoftwareRasterizer>();
	m_frcCtx->m_r->init(alloc);
	m_frcCtx->m_r->prepare(m_frcCtx->m_frc->getViewMatrix(), m_frcCtx->m_frc->getProjectionMatrix(), width, height);

	// Do the work
	m_frcCtx->m_r->fillDepthBuffer(depthBuff);
}

void GatherVisiblesFromOctreeTask::gather(ThreadHive& hive)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_OCTREE);

	U testIdx = m_frcCtx->m_visCtx->m_testsCount.fetchAdd(1);

	// Walk the tree
	m_frcCtx->m_visCtx->m_scene->getOctree().walkTree(testIdx,
		[&](const Aabb& box) {
			Bool visible = m_frcCtx->m_frc->insideFrustum(box);
			if(visible && m_frcCtx->m_r)
			{
				visible = m_frcCtx->m_r->visibilityTest(box, box);
			}

			return visible;
		},
		[&](void* placeableUserData) {
			ANKI_ASSERT(placeableUserData);
			SpatialComponent* scomp = static_cast<SpatialComponent*>(placeableUserData);

			ANKI_ASSERT(m_spatialCount < m_spatials.getSize());

			m_spatials[m_spatialCount++] = scomp;

			if(m_spatialCount == m_spatials.getSize())
			{
				flush(hive);
			}
		});

	// Flush the remaining
	flush(hive);

	// Fire an additional dummy task to decrease the semaphore to zero
	ThreadHiveTask task = ANKI_THREAD_HIVE_TASK({}, this, nullptr, m_frcCtx->m_visTestsSignalSem);
	hive.submitTasks(&task, 1);
}

void GatherVisiblesFromOctreeTask::flush(ThreadHive& hive)
{
	if(m_spatialCount)
	{
		// Create the task
		VisibilityTestTask* vis =
			m_frcCtx->m_visCtx->m_scene->getFrameAllocator().newInstance<VisibilityTestTask>(m_frcCtx);
		memcpy(&vis->m_spatialsToTest[0], &m_spatials[0], sizeof(m_spatials[0]) * m_spatialCount);
		vis->m_spatialToTestCount = m_spatialCount;

		// Increase the semaphore to block the CombineResultsTask
		m_frcCtx->m_visTestsSignalSem->increaseSemaphore(1);

		// Submit task
		ThreadHiveTask task =
			ANKI_THREAD_HIVE_TASK({ self->test(hive, threadId); }, vis, nullptr, m_frcCtx->m_visTestsSignalSem);
		hive.submitTasks(&task, 1);

		// Clear count
		m_spatialCount = 0;
	}
}

void VisibilityTestTask::test(ThreadHive& hive, U32 taskId)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_TEST);

	const FrustumComponent& testedFrc = *m_frcCtx->m_frc;
	ANKI_ASSERT(testedFrc.anyVisibilityTestEnabled());

	const SceneNode& testedNode = testedFrc.getSceneNode();
	auto alloc = m_frcCtx->m_visCtx->m_scene->getFrameAllocator();

	Timestamp& timestamp = m_frcCtx->m_queueViews[taskId].m_timestamp;
	timestamp = testedNode.getComponentMaxTimestamp();

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

	const Bool wantsDecals = testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::DECALS);

	const Bool wantsFogDensityComponents =
		testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::FOG_DENSITY_COMPONENTS);

	const Bool wantsEarlyZ = testedFrc.visibilityTestsEnabled(FrustumComponentVisibilityTestFlag::EARLY_Z)
							 && m_frcCtx->m_visCtx->m_earlyZDist > 0.0f;

	// Iterate
	RenderQueueView& result = m_frcCtx->m_queueViews[taskId];
	for(U i = 0; i < m_spatialToTestCount; ++i)
	{
		SpatialComponent* spatialC = m_spatialsToTest[i];
		ANKI_ASSERT(spatialC);
		SceneNode& node = spatialC->getSceneNode();

		// Skip if it is the same
		if(ANKI_UNLIKELY(&testedNode == &node))
		{
			continue;
		}

		// Check what components the frustum needs
		Bool wantNode = false;

		const RenderComponent* rc = nullptr;
		if(wantsRenderComponents && (rc = node.tryGetComponent<RenderComponent>()))
		{
			wantNode = true;
		}

		if(wantsShadowCasters && (rc = node.tryGetComponent<RenderComponent>()) && rc->getCastsShadow())
		{
			wantNode = true;
		}

		const LightComponent* lc = nullptr;
		if(wantsLightComponents && (lc = node.tryGetComponent<LightComponent>()))
		{
			wantNode = true;
		}

		const LensFlareComponent* lfc = nullptr;
		if(wantsFlareComponents && (lfc = node.tryGetComponent<LensFlareComponent>()))
		{
			wantNode = true;
		}

		const ReflectionProbeComponent* reflc = nullptr;
		if(wantsReflectionProbes && (reflc = node.tryGetComponent<ReflectionProbeComponent>()))
		{
			wantNode = true;
		}

		DecalComponent* decalc = nullptr;
		if(wantsDecals && (decalc = node.tryGetComponent<DecalComponent>()))
		{
			wantNode = true;
		}

		const FogDensityComponent* fogc = nullptr;
		if(wantsFogDensityComponents && (fogc = node.tryGetComponent<FogDensityComponent>()))
		{
			wantNode = true;
		}

		if(ANKI_UNLIKELY(!wantNode))
		{
			// Skip node
			continue;
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
			}

			++spIdx;

			return Error::NONE;
		});
		(void)err;

		if(ANKI_UNLIKELY(count == 0))
		{
			continue;
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
			RenderableQueueElement* el;
			if(rc->isForwardShading())
			{
				el = result.m_forwardShadingRenderables.newElement(alloc);
			}
			else
			{
				el = result.m_renderables.newElement(alloc);
			}

			rc->setupRenderableQueueElement(*el);

			// Compute distance from the frustum
			const Plane& nearPlane = testedFrc.getFrustum().getPlanesWorldSpace()[FrustumPlaneType::NEAR];
			el->m_distanceFromCamera = max(0.0f, sps[0].m_sp->getAabb().testPlane(nearPlane));

			if(wantsEarlyZ && el->m_distanceFromCamera < m_frcCtx->m_visCtx->m_earlyZDist && !rc->isForwardShading())
			{
				RenderableQueueElement* el2 = result.m_earlyZRenderables.newElement(alloc);
				*el2 = *el;
			}
		}

		if(lc)
		{
			switch(lc->getLightComponentType())
			{
			case LightComponentType::POINT:
			{
				PointLightQueueElement* el = result.m_pointLights.newElement(alloc);
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

					U32* p = result.m_shadowPointLights.newElement(alloc);
					*p = result.m_pointLights.m_elementCount - 1;
				}
				else
				{
					zeroMemory(el->m_shadowRenderQueues);
				}

				break;
			}
			case LightComponentType::SPOT:
			{
				SpotLightQueueElement* el = result.m_spotLights.newElement(alloc);
				lc->setupSpotLightQueueElement(*el);

				if(lc->getShadowEnabled())
				{
					RenderQueue* a = alloc.newInstance<RenderQueue>();
					nextQueues = WeakArray<RenderQueue>(a, 1);
					el->m_shadowRenderQueue = a;

					U32* p = result.m_shadowSpotLights.newElement(alloc);
					*p = result.m_spotLights.m_elementCount - 1;
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

		if(lfc)
		{
			LensFlareQueueElement* el = result.m_lensFlares.newElement(alloc);
			lfc->setupLensFlareQueueElement(*el);
		}

		if(reflc)
		{
			ReflectionProbeQueueElement* el = result.m_reflectionProbes.newElement(alloc);
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

		if(decalc)
		{
			DecalQueueElement* el = result.m_decals.newElement(alloc);
			decalc->setupDecalQueueElement(*el);
		}

		if(fogc)
		{
			FogDensityQueueElement* el = result.m_fogDensityVolumes.newElement(alloc);
			fogc->setupFogDensityQueueElement(*el);
		}

		// Add more frustums to the list
		if(nextQueues.getSize() > 0)
		{
			count = 0;
			err = node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
				m_frcCtx->m_visCtx->submitNewWork(frc, nextQueues[count++], hive);
				return Error::NONE;
			});
			(void)err;
		}

		// Update timestamp
		timestamp = max(timestamp, node.getComponentMaxTimestamp());
	} // end for
}

void CombineResultsTask::combine()
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_COMBINE_RESULTS);

	auto alloc = m_frcCtx->m_visCtx->m_scene->getFrameAllocator();
	RenderQueue& results = *m_frcCtx->m_renderQueue;

	// Compute the timestamp
	const U threadCount = m_frcCtx->m_queueViews.getSize();
	results.m_shadowRenderablesLastUpdateTimestamp = 0;
	for(U i = 0; i < threadCount; ++i)
	{
		results.m_shadowRenderablesLastUpdateTimestamp =
			max(results.m_shadowRenderablesLastUpdateTimestamp, m_frcCtx->m_queueViews[i].m_timestamp);
	}
	ANKI_ASSERT(results.m_shadowRenderablesLastUpdateTimestamp);

#define ANKI_VIS_COMBINE(t_, member_) \
	{ \
		Array<TRenderQueueElementStorage<t_>, 64> subStorages; \
		for(U i = 0; i < threadCount; ++i) \
		{ \
			subStorages[i] = m_frcCtx->m_queueViews[i].member_; \
		} \
		combineQueueElements<t_>(alloc, \
			WeakArray<TRenderQueueElementStorage<t_>>(&subStorages[0], threadCount), \
			nullptr, \
			results.member_, \
			nullptr); \
	}

#define ANKI_VIS_COMBINE_AND_PTR(t_, member_, ptrMember_) \
	{ \
		Array<TRenderQueueElementStorage<t_>, 64> subStorages; \
		Array<TRenderQueueElementStorage<U32>, 64> ptrSubStorages; \
		for(U i = 0; i < threadCount; ++i) \
		{ \
			subStorages[i] = m_frcCtx->m_queueViews[i].member_; \
			ptrSubStorages[i] = m_frcCtx->m_queueViews[i].ptrMember_; \
		} \
		WeakArray<TRenderQueueElementStorage<U32>> arr(&ptrSubStorages[0], threadCount); \
		combineQueueElements<t_>(alloc, \
			WeakArray<TRenderQueueElementStorage<t_>>(&subStorages[0], threadCount), \
			&arr, \
			results.member_, \
			&results.ptrMember_); \
	}

	ANKI_VIS_COMBINE(RenderableQueueElement, m_renderables);
	ANKI_VIS_COMBINE(RenderableQueueElement, m_earlyZRenderables);
	ANKI_VIS_COMBINE(RenderableQueueElement, m_forwardShadingRenderables);
	ANKI_VIS_COMBINE_AND_PTR(PointLightQueueElement, m_pointLights, m_shadowPointLights);
	ANKI_VIS_COMBINE_AND_PTR(SpotLightQueueElement, m_spotLights, m_shadowSpotLights);
	ANKI_VIS_COMBINE(ReflectionProbeQueueElement, m_reflectionProbes);
	ANKI_VIS_COMBINE(LensFlareQueueElement, m_lensFlares);
	ANKI_VIS_COMBINE(DecalQueueElement, m_decals);
	ANKI_VIS_COMBINE(FogDensityQueueElement, m_fogDensityVolumes);

#undef ANKI_VIS_COMBINE
#undef ANKI_VIS_COMBINE_AND_PTR

#if ANKI_EXTRA_CHECKS
	for(PointLightQueueElement* light : results.m_shadowPointLights)
	{
		ANKI_ASSERT(light->hasShadow());
	}

	for(SpotLightQueueElement* light : results.m_shadowSpotLights)
	{
		ANKI_ASSERT(light->hasShadow());
	}
#endif

	// Sort some of the arrays
	std::sort(results.m_renderables.getBegin(), results.m_renderables.getEnd(), MaterialDistanceSortFunctor(20.0f));

	std::sort(results.m_earlyZRenderables.getBegin(),
		results.m_earlyZRenderables.getEnd(),
		DistanceSortFunctor<RenderableQueueElement>());

	std::sort(results.m_forwardShadingRenderables.getBegin(),
		results.m_forwardShadingRenderables.getEnd(),
		RevDistanceSortFunctor<RenderableQueueElement>());

	// Cleanup
	if(m_frcCtx->m_r)
	{
		m_frcCtx->m_r->~SoftwareRasterizer();
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

void SceneGraph::doVisibilityTests(SceneNode& fsn, SceneGraph& scene, RenderQueue& rqueue)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_TESTS);

	ThreadHive& hive = scene.getThreadHive();

	VisibilityContext ctx;
	ctx.m_scene = &scene;
	ctx.m_earlyZDist = scene.getEarlyZDistance();
	ctx.submitNewWork(fsn.getComponent<FrustumComponent>(), rqueue, hive);

	hive.waitAllTasks();
	ctx.m_testedFrcs.destroy(scene.getFrameAllocator());
}

} // end namespace anki
