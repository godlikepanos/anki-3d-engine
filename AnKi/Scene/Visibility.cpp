// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/VisibilityInternal.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/Components/RenderComponent.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/GenericGpuComputeJobComponent.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

static U8 computeLod(const FrustumComponent& frc, F32 distanceFromTheNearPlane)
{
	static_assert(kMaxLodCount == 3, "Wrong assumption");
	U8 lod;
	if(distanceFromTheNearPlane < 0.0f)
	{
		// In RT objects may fall behind the camera, use the max LOD on those
		lod = 2;
	}
	else if(distanceFromTheNearPlane <= frc.getLodDistance(0))
	{
		lod = 0;
	}
	else if(distanceFromTheNearPlane <= frc.getLodDistance(1))
	{
		lod = 1;
	}
	else
	{
		lod = 2;
	}

	return lod;
}

static Bool spatialInsideFrustum(const FrustumComponent& frc, const SpatialComponent& spc)
{
	switch(spc.getCollisionShapeType())
	{
	case CollisionShapeType::kOBB:
		return frc.insideFrustum(spc.getCollisionShape<Obb>());
		break;
	case CollisionShapeType::kAABB:
		return frc.insideFrustum(spc.getCollisionShape<Aabb>());
		break;
	case CollisionShapeType::kSphere:
		return frc.insideFrustum(spc.getCollisionShape<Sphere>());
		break;
	case CollisionShapeType::kConvexHull:
		return frc.insideFrustum(spc.getCollisionShape<ConvexHullShape>());
		break;
	default:
		ANKI_ASSERT(0);
		return false;
	}
}

/// Used to silent warnings
template<typename TComponent>
Bool getComponent(SceneNode& node, TComponent*& comp)
{
	comp = node.tryGetFirstComponentOfType<TComponent>();
	return comp != nullptr;
}

void VisibilityContext::submitNewWork(const FrustumComponent& frc, const FrustumComponent& primaryFrustum,
									  RenderQueue& rqueue, ThreadHive& hive)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_SUBMIT_WORK);

	// Check enabled and make sure that the results are null (this can happen on multiple on circular viewing)
	if(ANKI_UNLIKELY(frc.getEnabledVisibilityTests() == FrustumComponentVisibilityTestFlag::kNone))
	{
		return;
	}

	rqueue.m_cameraTransform = Mat3x4(frc.getWorldTransform());
	rqueue.m_viewMatrix = frc.getViewMatrix();
	rqueue.m_projectionMatrix = frc.getProjectionMatrix();
	rqueue.m_viewProjectionMatrix = frc.getViewProjectionMatrix();
	rqueue.m_previousViewProjectionMatrix = frc.getPreviousViewProjectionMatrix();
	rqueue.m_cameraNear = frc.getNear();
	rqueue.m_cameraFar = frc.getFar();
	if(frc.getFrustumType() == FrustumType::kPerspective)
	{
		rqueue.m_cameraFovX = frc.getFovX();
		rqueue.m_cameraFovY = frc.getFovY();
	}
	else
	{
		rqueue.m_cameraFovX = rqueue.m_cameraFovY = 0.0f;
	}
	rqueue.m_effectiveShadowDistance = frc.getEffectiveShadowDistance();

	StackMemoryPool& pool = m_scene->getFrameMemoryPool();

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
		m_testedFrcs.pushBack(pool, &frc);
	}

	// Prepare the ctx
	FrustumVisibilityContext* frcCtx = newInstance<FrustumVisibilityContext>(pool);
	frcCtx->m_visCtx = this;
	frcCtx->m_frc = &frc;
	frcCtx->m_primaryFrustum = &primaryFrustum;
	frcCtx->m_queueViews.create(pool, hive.getThreadCount());
	frcCtx->m_visTestsSignalSem = hive.newSemaphore(1);
	frcCtx->m_renderQueue = &rqueue;

	// Submit new work
	//

	// Software rasterizer task
	ThreadHiveSemaphore* prepareRasterizerSem = nullptr;
	if(!!(frc.getEnabledVisibilityTests() & FrustumComponentVisibilityTestFlag::kOccluders) && frc.hasCoverageBuffer())
	{
		// Gather triangles task
		ThreadHiveTask fillDepthTask =
			ANKI_THREAD_HIVE_TASK({ self->fill(); }, newInstance<FillRasterizerWithCoverageTask>(pool, frcCtx), nullptr,
								  hive.newSemaphore(1));

		hive.submitTasks(&fillDepthTask, 1);

		prepareRasterizerSem = fillDepthTask.m_signalSemaphore;
	}

	if(!!(frc.getEnabledVisibilityTests() & FrustumComponentVisibilityTestFlag::kOccluders))
	{
		rqueue.m_fillCoverageBufferCallback = FrustumComponent::fillCoverageBufferCallback;
		rqueue.m_fillCoverageBufferCallbackUserData = static_cast<void*>(const_cast<FrustumComponent*>(&frc));
	}

	// Gather visibles from the octree. No need to signal anything because it will spawn new tasks
	ThreadHiveTask gatherTask =
		ANKI_THREAD_HIVE_TASK({ self->gather(hive); }, newInstance<GatherVisiblesFromOctreeTask>(pool, frcCtx),
							  prepareRasterizerSem, nullptr);
	hive.submitTasks(&gatherTask, 1);

	// Combind results task
	ANKI_ASSERT(frcCtx->m_visTestsSignalSem);
	ThreadHiveTask combineTask = ANKI_THREAD_HIVE_TASK(
		{ self->combine(); }, newInstance<CombineResultsTask>(pool, frcCtx), frcCtx->m_visTestsSignalSem, nullptr);
	hive.submitTasks(&combineTask, 1);
}

void FillRasterizerWithCoverageTask::fill()
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_FILL_DEPTH);

	StackMemoryPool& pool = m_frcCtx->m_visCtx->m_scene->getFrameMemoryPool();

	// Get the C-Buffer
	ConstWeakArray<F32> depthBuff;
	U32 width;
	U32 height;
	m_frcCtx->m_frc->getCoverageBufferInfo(depthBuff, width, height);
	ANKI_ASSERT(width > 0 && height > 0 && depthBuff.getSize() > 0);

	// Init the rasterizer
	if(true)
	{
		m_frcCtx->m_r = newInstance<SoftwareRasterizer>(pool);
		m_frcCtx->m_r->init(&pool);
		m_frcCtx->m_r->prepare(Mat4(m_frcCtx->m_frc->getPreviousViewMatrix(1), Vec4(0.0f, 0.0f, 0.0f, 1.0f)),
							   m_frcCtx->m_frc->getPreviousProjectionMatrix(1), width, height);

		// Do the work
		m_frcCtx->m_r->fillDepthBuffer(depthBuff);
	}
}

void GatherVisiblesFromOctreeTask::gather(ThreadHive& hive)
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_OCTREE);

	U32 testIdx = m_frcCtx->m_visCtx->m_testsCount.fetchAdd(1);

	// Walk the tree
	m_frcCtx->m_visCtx->m_scene->getOctree().walkTree(
		testIdx,
		[&](const Aabb& box) {
			Bool visible = m_frcCtx->m_frc->insideFrustum(box);
			if(visible && m_frcCtx->m_r)
			{
				visible = m_frcCtx->m_r->visibilityTest(box);
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
	GatherVisiblesFromOctreeTask* pself = this; // MSVC workaround
	ThreadHiveTask task = ANKI_THREAD_HIVE_TASK({}, pself, nullptr, m_frcCtx->m_visTestsSignalSem);
	hive.submitTasks(&task, 1);
}

void GatherVisiblesFromOctreeTask::flush(ThreadHive& hive)
{
	if(m_spatialCount)
	{
		// Create the task
		VisibilityTestTask* vis =
			newInstance<VisibilityTestTask>(m_frcCtx->m_visCtx->m_scene->getFrameMemoryPool(), m_frcCtx);
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
	const FrustumComponentVisibilityTestFlag frustumFlags = testedFrc.getEnabledVisibilityTests();
	ANKI_ASSERT(frustumFlags != FrustumComponentVisibilityTestFlag::kNone);
	ANKI_ASSERT(m_frcCtx->m_primaryFrustum);
	const FrustumComponent& primaryFrc = *m_frcCtx->m_primaryFrustum;

	const SceneNode& testedNode = testedFrc.getSceneNode();
	StackMemoryPool& pool = m_frcCtx->m_visCtx->m_scene->getFrameMemoryPool();

	Timestamp& timestamp = m_frcCtx->m_queueViews[taskId].m_timestamp;
	timestamp = testedNode.getComponentMaxTimestamp();

	const Bool wantsEarlyZ =
		!!(frustumFlags & FrustumComponentVisibilityTestFlag::kEarlyZ) && m_frcCtx->m_visCtx->m_earlyZDist > 0.0f;

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
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kRenderComponents) && getComponent(node, rc);

		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kShadowCasterRenderComponents)
					&& getComponent(node, rc) && !!(rc->getFlags() & RenderComponentFlag::kCastsShadow);

		const RenderComponent* rtRc = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kAllRayTracing) && getComponent(node, rtRc)
					&& rtRc->getSupportsRayTracing();

		const LightComponent* lc = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kLights) && getComponent(node, lc);

		const LensFlareComponent* lfc = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kLensFlares) && getComponent(node, lfc);

		const ReflectionProbeComponent* reflc = nullptr;
		wantNode |=
			!!(frustumFlags & FrustumComponentVisibilityTestFlag::kReflectionProbes) && getComponent(node, reflc);

		DecalComponent* decalc = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kDecals) && getComponent(node, decalc);

		const FogDensityComponent* fogc = nullptr;
		wantNode |=
			!!(frustumFlags & FrustumComponentVisibilityTestFlag::kFogDensityVolumes) && getComponent(node, fogc);

		GlobalIlluminationProbeComponent* giprobec = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kGlobalIlluminationProbes)
					&& getComponent(node, giprobec);

		GenericGpuComputeJobComponent* computec = nullptr;
		wantNode |=
			!!(frustumFlags & FrustumComponentVisibilityTestFlag::kGenericComputeJobs) && getComponent(node, computec);

		UiComponent* uic = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kUi) && getComponent(node, uic);

		SkyboxComponent* skyboxc = nullptr;
		wantNode |= !!(frustumFlags & FrustumComponentVisibilityTestFlag::kSkybox) && getComponent(node, skyboxc);

		if(ANKI_UNLIKELY(!wantNode))
		{
			// Skip node
			continue;
		}

		const SpatialComponent* spatialc = node.tryGetFirstComponentOfType<SpatialComponent>();
		if(ANKI_UNLIKELY(spatialc == nullptr))
		{
			continue;
		}

		if(!spatialc->getAlwaysVisible()
		   && (!spatialInsideFrustum(testedFrc, *spatialc) || !testAgainstRasterizer(spatialc->getAabbWorldSpace())))
		{
			continue;
		}

		WeakArray<RenderQueue> nextQueues;
		WeakArray<FrustumComponent> nextQueueFrustumComponents; // Optional

		node.iterateComponentsOfType<RenderComponent>([&](const RenderComponent& rc) {
			RenderableQueueElement* el;
			if(!!(rc.getFlags() & RenderComponentFlag::kForwardShading))
			{
				el = result.m_forwardShadingRenderables.newElement(pool);
			}
			else
			{
				el = result.m_renderables.newElement(pool);
			}

			rc.setupRenderableQueueElement(*el);

			// Compute distance from the frustum
			const Plane& nearPlane = primaryFrc.getViewPlanes()[FrustumPlaneType::kNear];
			el->m_distanceFromCamera = !!(rc.getFlags() & RenderComponentFlag::kSortLast)
										   ? primaryFrc.getFar()
										   : max(0.0f, testPlane(nearPlane, spatialc->getAabbWorldSpace()));

			el->m_lod = computeLod(primaryFrc, el->m_distanceFromCamera);

			// Add to early Z
			if(wantsEarlyZ && el->m_distanceFromCamera < m_frcCtx->m_visCtx->m_earlyZDist
			   && !(rc.getFlags() & RenderComponentFlag::kForwardShading))
			{
				RenderableQueueElement* el2 = result.m_earlyZRenderables.newElement(pool);
				*el2 = *el;
			}

			// Add to RT
			if(rtRc)
			{
				RayTracingInstanceQueueElement* el = result.m_rayTracingInstances.newElement(pool);

				// Compute the LOD
				const Plane& nearPlane = primaryFrc.getViewPlanes()[FrustumPlaneType::kNear];
				const F32 dist = testPlane(nearPlane, spatialc->getAabbWorldSpace());
				rc.setupRayTracingInstanceQueueElement(computeLod(primaryFrc, dist), *el);
			}
		});

		if(lc)
		{
			// Check if it casts shadow
			Bool castsShadow = lc->getShadowEnabled();
			if(castsShadow && lc->getLightComponentType() != LightComponentType::kDirectional)
			{
				// Extra check

				// Compute distance from the frustum
				const Plane& nearPlane = primaryFrc.getViewPlanes()[FrustumPlaneType::kNear];
				const F32 distFromFrustum = max(0.0f, testPlane(nearPlane, spatialc->getAabbWorldSpace()));

				castsShadow = distFromFrustum < primaryFrc.getEffectiveShadowDistance();
			}

			switch(lc->getLightComponentType())
			{
			case LightComponentType::kPoint:
			{
				PointLightQueueElement* el = result.m_pointLights.newElement(pool);
				lc->setupPointLightQueueElement(*el);

				if(castsShadow && !!(frustumFlags & FrustumComponentVisibilityTestFlag::kPointLightShadowsEnabled))
				{
					RenderQueue* a = newArray<RenderQueue>(pool, 6);
					nextQueues = WeakArray<RenderQueue>(a, 6);

					el->m_shadowRenderQueues[0] = &nextQueues[0];
					el->m_shadowRenderQueues[1] = &nextQueues[1];
					el->m_shadowRenderQueues[2] = &nextQueues[2];
					el->m_shadowRenderQueues[3] = &nextQueues[3];
					el->m_shadowRenderQueues[4] = &nextQueues[4];
					el->m_shadowRenderQueues[5] = &nextQueues[5];
				}
				else
				{
					zeroMemory(el->m_shadowRenderQueues);
				}

				break;
			}
			case LightComponentType::kSpot:
			{
				SpotLightQueueElement* el = result.m_spotLights.newElement(pool);
				lc->setupSpotLightQueueElement(*el);

				if(castsShadow && !!(frustumFlags & FrustumComponentVisibilityTestFlag::kSpotLightShadowsEnabled))
				{
					RenderQueue* a = newInstance<RenderQueue>(pool);
					nextQueues = WeakArray<RenderQueue>(a, 1);
					el->m_shadowRenderQueue = a;
				}
				else
				{
					el->m_shadowRenderQueue = nullptr;
				}

				break;
			}
			case LightComponentType::kDirectional:
			{
				ANKI_ASSERT(lc->getShadowEnabled() == true && "Only with shadow for now");

				U32 cascadeCount = max(testedFrc.getShadowCascadeCount(), U32(castsShadow));
				if(!castsShadow)
				{
					cascadeCount = 0;
				}
				else
				{
					cascadeCount = max<U32>(testedFrc.getShadowCascadeCount(), 1);
				}
				ANKI_ASSERT(cascadeCount <= kMaxShadowCascades);

				// Create some dummy frustum components and initialize them
				WeakArray<FrustumComponent> cascadeFrustumComponents(
					(cascadeCount) ? static_cast<FrustumComponent*>(
						pool.allocate(cascadeCount * sizeof(FrustumComponent), alignof(FrustumComponent)))
								   : nullptr,
					cascadeCount);
				for(U32 i = 0; i < cascadeCount; ++i)
				{
					::new(&cascadeFrustumComponents[i]) FrustumComponent(&node);
					cascadeFrustumComponents[i].setFrustumType(FrustumType::kOrthographic);
				}

				lc->setupDirectionalLightQueueElement(testedFrc, result.m_directionalLight, cascadeFrustumComponents);

				nextQueues = WeakArray<RenderQueue>(
					(cascadeCount) ? newArray<RenderQueue>(pool, cascadeCount) : nullptr, cascadeCount);
				for(U32 i = 0; i < cascadeCount; ++i)
				{
					result.m_directionalLight.m_shadowRenderQueues[i] = &nextQueues[i];
				}

				// Despite the fact that it's the same light it will have different properties if viewed by different
				// cameras. If the renderer finds the same UUID it will think it's cached and use wrong shadow tiles.
				// That's why we need to change its UUID and bind it to the frustum that is currently viewing the light
				result.m_directionalLight.m_uuid = testedNode.getUuid();

				// Manually update the dummy components
				for(U32 i = 0; i < cascadeCount; ++i)
				{
					cascadeFrustumComponents[i].setEnabledVisibilityTests(
						FrustumComponentVisibilityTestFlag::kShadowCasterRenderComponents);
					Bool updated;
					SceneComponentUpdateInfo scUpdateInfo(0.0, 1.0);
					scUpdateInfo.m_node = &node;
					[[maybe_unused]] const Error err = cascadeFrustumComponents[i].update(scUpdateInfo, updated);
					ANKI_ASSERT(updated == true && !err);
				}

				nextQueueFrustumComponents = cascadeFrustumComponents;

				break;
			}
			default:
				ANKI_ASSERT(0);
			}
		}

		if(lfc && lfc->isLoaded())
		{
			LensFlareQueueElement* el = result.m_lensFlares.newElement(pool);
			lfc->setupLensFlareQueueElement(*el);
		}

		if(reflc)
		{
			ReflectionProbeQueueElement* el = result.m_reflectionProbes.newElement(pool);
			reflc->setupReflectionProbeQueueElement(*el);

			if(reflc->getMarkedForRendering())
			{
				RenderQueue* a = newArray<RenderQueue>(pool, 6);
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
				el->m_renderQueues = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
			}
		}

		if(decalc)
		{
			DecalQueueElement* el = result.m_decals.newElement(pool);
			decalc->setupDecalQueueElement(*el);
		}

		if(fogc)
		{
			FogDensityQueueElement* el = result.m_fogDensityVolumes.newElement(pool);
			fogc->setupFogDensityQueueElement(*el);
		}

		if(giprobec)
		{
			GlobalIlluminationProbeQueueElement* el = result.m_giProbes.newElement(pool);
			giprobec->setupGlobalIlluminationProbeQueueElement(*el);

			if(giprobec->getMarkedForRendering())
			{
				RenderQueue* a = newArray<RenderQueue>(pool, 6);
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
				el->m_renderQueues = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
			}
		}

		if(computec)
		{
			GenericGpuComputeJobQueueElement* el = result.m_genericGpuComputeJobs.newElement(pool);
			computec->setupGenericGpuComputeJobQueueElement(*el);
		}

		if(uic)
		{
			UiQueueElement* el = result.m_uis.newElement(pool);
			uic->setupUiQueueElement(*el);
		}

		if(skyboxc)
		{
			skyboxc->setupSkyboxQueueElement(result.m_skybox);
			result.m_skyboxSet = true;
		}

		// Add more frustums to the list
		if(nextQueues.getSize() > 0)
		{
			U32 count = 0;

			if(ANKI_LIKELY(nextQueueFrustumComponents.getSize() == 0))
			{
				node.iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
					m_frcCtx->m_visCtx->submitNewWork(frc, primaryFrc, nextQueues[count++], hive);
				});
			}
			else
			{
				for(FrustumComponent& frc : nextQueueFrustumComponents)
				{
					m_frcCtx->m_visCtx->submitNewWork(frc, primaryFrc, nextQueues[count++], hive);
				}
			}
		}

		// Update timestamp
		timestamp = max(timestamp, node.getComponentMaxTimestamp());
	} // end for
}

void CombineResultsTask::combine()
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_VIS_COMBINE_RESULTS);

	StackMemoryPool& pool = m_frcCtx->m_visCtx->m_scene->getFrameMemoryPool();
	RenderQueue& results = *m_frcCtx->m_renderQueue;

	// Compute the timestamp
	const U32 threadCount = m_frcCtx->m_queueViews.getSize();
	results.m_shadowRenderablesLastUpdateTimestamp = 0;
	for(U32 i = 0; i < threadCount; ++i)
	{
		results.m_shadowRenderablesLastUpdateTimestamp =
			max(results.m_shadowRenderablesLastUpdateTimestamp, m_frcCtx->m_queueViews[i].m_timestamp);
	}
	ANKI_ASSERT(results.m_shadowRenderablesLastUpdateTimestamp);

#define ANKI_VIS_COMBINE(t_, member_) \
	{ \
		Array<TRenderQueueElementStorage<t_>, 64> subStorages; \
		for(U32 i = 0; i < threadCount; ++i) \
		{ \
			subStorages[i] = m_frcCtx->m_queueViews[i].member_; \
		} \
		combineQueueElements<t_>(pool, WeakArray<TRenderQueueElementStorage<t_>>(&subStorages[0], threadCount), \
								 nullptr, results.member_, nullptr); \
	}

	ANKI_VIS_COMBINE(RenderableQueueElement, m_renderables);
	ANKI_VIS_COMBINE(RenderableQueueElement, m_earlyZRenderables);
	ANKI_VIS_COMBINE(RenderableQueueElement, m_forwardShadingRenderables);
	ANKI_VIS_COMBINE(PointLightQueueElement, m_pointLights);
	ANKI_VIS_COMBINE(SpotLightQueueElement, m_spotLights);
	ANKI_VIS_COMBINE(ReflectionProbeQueueElement, m_reflectionProbes);
	ANKI_VIS_COMBINE(LensFlareQueueElement, m_lensFlares);
	ANKI_VIS_COMBINE(DecalQueueElement, m_decals);
	ANKI_VIS_COMBINE(FogDensityQueueElement, m_fogDensityVolumes);
	ANKI_VIS_COMBINE(GlobalIlluminationProbeQueueElement, m_giProbes);
	ANKI_VIS_COMBINE(GenericGpuComputeJobQueueElement, m_genericGpuComputeJobs);
	ANKI_VIS_COMBINE(RayTracingInstanceQueueElement, m_rayTracingInstances);
	ANKI_VIS_COMBINE(UiQueueElement, m_uis);

	for(U32 i = 0; i < threadCount; ++i)
	{
		if(m_frcCtx->m_queueViews[i].m_directionalLight.m_uuid != 0)
		{
			results.m_directionalLight = m_frcCtx->m_queueViews[i].m_directionalLight;
		}

		if(m_frcCtx->m_queueViews[i].m_skyboxSet)
		{
			results.m_skybox = m_frcCtx->m_queueViews[i].m_skybox;
		}
	}

#undef ANKI_VIS_COMBINE

	const Bool isShadowFrustum = !!(m_frcCtx->m_frc->getEnabledVisibilityTests()
									& FrustumComponentVisibilityTestFlag::kShadowCasterRenderComponents);

	// Sort some of the arrays
	if(!isShadowFrustum)
	{
		std::sort(results.m_renderables.getBegin(), results.m_renderables.getEnd(), MaterialDistanceSortFunctor());

		std::sort(results.m_earlyZRenderables.getBegin(), results.m_earlyZRenderables.getEnd(),
				  DistanceSortFunctor<RenderableQueueElement>());

		std::sort(results.m_forwardShadingRenderables.getBegin(), results.m_forwardShadingRenderables.getEnd(),
				  RevDistanceSortFunctor<RenderableQueueElement>());
	}

	std::sort(results.m_giProbes.getBegin(), results.m_giProbes.getEnd());

	// Sort the ligths as well because some rendering effects expect the same order from frame to frame
	std::sort(results.m_pointLights.getBegin(), results.m_pointLights.getEnd(),
			  [](const PointLightQueueElement& a, const PointLightQueueElement& b) -> Bool {
				  if(a.hasShadow() != b.hasShadow())
				  {
					  return a.hasShadow() < b.hasShadow();
				  }
				  else
				  {
					  return a.m_uuid < b.m_uuid;
				  }
			  });

	std::sort(results.m_spotLights.getBegin(), results.m_spotLights.getEnd(),
			  [](const SpotLightQueueElement& a, const SpotLightQueueElement& b) -> Bool {
				  if(a.hasShadow() != b.hasShadow())
				  {
					  return a.hasShadow() > b.hasShadow();
				  }
				  else
				  {
					  return a.m_uuid < b.m_uuid;
				  }
			  });

	// Cleanup
	if(m_frcCtx->m_r)
	{
		m_frcCtx->m_r->~SoftwareRasterizer();
	}
}

template<typename T>
void CombineResultsTask::combineQueueElements(StackMemoryPool& pool,
											  WeakArray<TRenderQueueElementStorage<T>> subStorages,
											  WeakArray<TRenderQueueElementStorage<U32>>* ptrSubStorages,
											  WeakArray<T>& combined, WeakArray<T*>* ptrCombined)
{
	U32 totalElCount = subStorages[0].m_elementCount;
	U32 biggestSubStorageIdx = 0;
	for(U32 i = 1; i < subStorages.getSize(); ++i)
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
		U32 ptrTotalElCount = (*ptrSubStorages)[0].m_elementCount;

		for(U32 i = 1; i < ptrSubStorages->getSize(); ++i)
		{
			ptrTotalElCount += (*ptrSubStorages)[i].m_elementCount;
		}

		// Create the new storage
		if(ptrTotalElCount > 0)
		{
			ptrIt = newArray<T*>(pool, ptrTotalElCount);
			*ptrCombined = WeakArray<T*>(ptrIt, ptrTotalElCount);
		}
	}

	T* it;
	if(totalElCount > subStorages[biggestSubStorageIdx].m_elementStorage)
	{
		// Can't reuse any of the existing storage, will allocate a brand new one

		it = newArray<T>(pool, totalElCount);
		biggestSubStorageIdx = kMaxU32;

		combined = WeakArray<T>(it, totalElCount);
	}
	else
	{
		// Will reuse existing storage

		it = subStorages[biggestSubStorageIdx].m_elements + subStorages[biggestSubStorageIdx].m_elementCount;

		combined = WeakArray<T>(subStorages[biggestSubStorageIdx].m_elements, totalElCount);
	}

	for(U32 i = 0; i < subStorages.getSize(); ++i)
	{
		if(subStorages[i].m_elementCount == 0)
		{
			continue;
		}

		// Copy the pointers
		if(ptrIt)
		{
			T* base = (i != biggestSubStorageIdx) ? it : subStorages[biggestSubStorageIdx].m_elements;

			for(U32 x = 0; x < (*ptrSubStorages)[i].m_elementCount; ++x)
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
	ctx.m_earlyZDist = scene.getConfig().getSceneEarlyZDistance();
	const FrustumComponent& mainFrustum = fsn.getFirstComponentOfType<FrustumComponent>();
	ctx.submitNewWork(mainFrustum, mainFrustum, rqueue, hive);

	const FrustumComponent* extendedFrustum = fsn.tryGetNthComponentOfType<FrustumComponent>(1);
	if(extendedFrustum)
	{
		// This is the frustum for RT.
		ANKI_ASSERT(
			!(extendedFrustum->getEnabledVisibilityTests() & ~FrustumComponentVisibilityTestFlag::kAllRayTracing));

		rqueue.m_rayTracingQueue = newInstance<RenderQueue>(scene.getFrameMemoryPool());
		ctx.submitNewWork(*extendedFrustum, mainFrustum, *rqueue.m_rayTracingQueue, hive);
	}

	hive.waitAllTasks();
	ctx.m_testedFrcs.destroy(scene.getFrameMemoryPool());
}

} // end namespace anki
