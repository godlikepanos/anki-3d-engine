// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SoftwareRasterizer.h>
#include <AnKi/Scene/Octree.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

/// @addtogroup scene
/// @{

constexpr U32 kMaxSpatialsPerVisTest = 48; ///< Num of spatials to test in a single ThreadHive task.

class FrameMemoryPoolWrapper
{
public:
	StackMemoryPool* operator&()
	{
		return &SceneGraph::getSingleton().getFrameMemoryPool();
	}

	operator StackMemoryPool&()
	{
		return SceneGraph::getSingleton().getFrameMemoryPool();
	}

	void* allocate(PtrSize size, PtrSize alignmentBytes)
	{
		return SceneGraph::getSingleton().getFrameMemoryPool().allocate(size, alignmentBytes);
	}

	void free(void* ptr)
	{
		SceneGraph::getSingleton().getFrameMemoryPool().free(ptr);
	}
};

/// Sort objects on distance
template<typename T>
class DistanceSortFunctor
{
public:
	Bool operator()(const T& a, const T& b)
	{
		return a.m_distanceFromCamera < b.m_distanceFromCamera;
	}
};

template<typename T>
class RevDistanceSortFunctor
{
public:
	Bool operator()(const T& a, const T& b)
	{
		return a.m_distanceFromCamera > b.m_distanceFromCamera;
	}
};

/// Sorts first by LOD and then by material (merge key).
class MaterialDistanceSortFunctor
{
public:
	Bool operator()(const RenderableQueueElement& a, const RenderableQueueElement& b)
	{
		return a.m_mergeKey < b.m_mergeKey;
	}
};

/// Storage for a single element type.
template<typename T, U32 kInitialStorage = 32, U32 kStorageGrowRate = 4>
class TRenderQueueElementStorage
{
public:
	T* m_elements = nullptr;
	U32 m_elementCount = 0;
	U32 m_elementStorage = 0;

	T* newElement()
	{
		if(m_elementCount + 1 > m_elementStorage) [[unlikely]]
		{
			m_elementStorage = max(kInitialStorage, m_elementStorage * kStorageGrowRate);

			const T* oldElements = m_elements;
			m_elements = static_cast<T*>(
				SceneGraph::getSingleton().getFrameMemoryPool().allocate(m_elementStorage * sizeof(T), alignof(T)));

			if(oldElements)
			{
				memcpy(m_elements, oldElements, sizeof(T) * m_elementCount);
			}
		}

		return &m_elements[m_elementCount++];
	}
};

class RenderQueueView
{
public:
	TRenderQueueElementStorage<RenderableQueueElement> m_renderables; ///< Deferred shading or shadow renderables.
	TRenderQueueElementStorage<RenderableQueueElement> m_forwardShadingRenderables;
	TRenderQueueElementStorage<RenderableQueueElement> m_earlyZRenderables;
	TRenderQueueElementStorage<PointLightQueueElement> m_pointLights;
	TRenderQueueElementStorage<SpotLightQueueElement> m_spotLights;
	DirectionalLightQueueElement m_directionalLight;
	TRenderQueueElementStorage<ReflectionProbeQueueElement> m_reflectionProbes;
	TRenderQueueElementStorage<LensFlareQueueElement> m_lensFlares;
	TRenderQueueElementStorage<DecalQueueElement> m_decals;
	TRenderQueueElementStorage<FogDensityQueueElement> m_fogDensityVolumes;
	TRenderQueueElementStorage<GlobalIlluminationProbeQueueElement> m_giProbes;
	TRenderQueueElementStorage<GenericGpuComputeJobQueueElement> m_genericGpuComputeJobs;
	TRenderQueueElementStorage<RayTracingInstanceQueueElement> m_rayTracingInstances;
	TRenderQueueElementStorage<UiQueueElement> m_uis;
	SkyboxQueueElement m_skybox;
	Bool m_skyboxSet = false;

	Timestamp m_timestamp = 0;

	RenderQueueView()
	{
		zeroMemory(m_directionalLight);
		zeroMemory(m_skybox);
	}
};

static_assert(std::is_trivially_destructible<RenderQueueView>::value == true, "Should be trivially destructible");

class FrustumFlags
{
public:
	Bool m_gatherModelComponents : 1 = false;
	Bool m_gatherShadowCasterModelComponents : 1 = false;
	Bool m_gatherRayTracingModelComponents : 1 = false;
	Bool m_gatherParticleComponents : 1 = false;
	Bool m_gatherProbeComponents : 1 = false;
	Bool m_gatherLightComponents : 1 = false;
	Bool m_gatherLensFlareComponents : 1 = false;
	Bool m_gatherDecalComponents : 1 = false;
	Bool m_gatherFogDensityComponents : 1 = false;
	Bool m_gatherUiComponents : 1 = false;
	Bool m_gatherSkyComponents : 1 = false;

	Bool m_coverageBuffer : 1 = false;
	Bool m_earlyZ : 1 = false;
	Bool m_nonDirectionalLightsCastShadow : 1 = false;
	Bool m_directionalLightsCastShadow : 1 = false;
};

class VisibilityFrustum : public FrustumFlags
{
public:
	Frustum* m_frustum = nullptr;
};

/// Data common for all tasks.
class VisibilityContext
{
public:
	Atomic<U32> m_testsCount = {0};

	List<const Frustum*, FrameMemoryPoolWrapper> m_testedFrustums;
	Mutex m_testedFrustumsMtx;

	void submitNewWork(const VisibilityFrustum& frustum, const VisibilityFrustum& primaryFrustum, RenderQueue& result,
					   ThreadHive& hive);
};

/// A context for a specific test of a frustum component.
/// @note Should be trivially destructible.
class FrustumVisibilityContext
{
public:
	VisibilityContext* m_visCtx = nullptr;

	VisibilityFrustum m_frustum; ///< This is the frustum to be tested.
	VisibilityFrustum m_primaryFrustum; ///< This is the primary camera frustum.

	// S/W rasterizer members
	SoftwareRasterizer* m_r = nullptr;
	DynamicArray<Vec3, FrameMemoryPoolWrapper> m_verts;
	Atomic<U32> m_rasterizedVertCount = {0}; ///< That will be used by the RasterizeTrianglesTask.

	// Visibility test members
	DynamicArray<RenderQueueView, FrameMemoryPoolWrapper> m_queueViews; ///< Sub result. Will be combined later.
	ThreadHiveSemaphore* m_visTestsSignalSem = nullptr;

	// Gather results members
	RenderQueue* m_renderQueue = nullptr;

	ReflectionProbeQueueElementForRefresh* m_reflectionProbeForRefresh = nullptr;
	Atomic<U32> m_reflectionProbesForRefreshCount = {0};

	Atomic<U32> m_giProbesForRefreshCount = {0};
	GlobalIlluminationProbeQueueElementForRefresh* m_giProbeForRefresh = nullptr;
};

/// ThreadHive task to set the depth map of the S/W rasterizer.
class FillRasterizerWithCoverageTask
{
public:
	FrustumVisibilityContext* m_frcCtx = nullptr;

	FillRasterizerWithCoverageTask(FrustumVisibilityContext* frcCtx)
		: m_frcCtx(frcCtx)
	{
		ANKI_ASSERT(m_frcCtx);
	}

	void fill();
};
static_assert(std::is_trivially_destructible<FillRasterizerWithCoverageTask>::value == true,
			  "Should be trivially destructible");

/// ThreadHive task to get visible nodes from the octree.
class GatherVisiblesFromOctreeTask
{
public:
	FrustumVisibilityContext* m_frcCtx = nullptr;

	GatherVisiblesFromOctreeTask(FrustumVisibilityContext* frcCtx)
		: m_frcCtx(frcCtx)
	{
		ANKI_ASSERT(m_frcCtx);
	}

	void gather(ThreadHive& hive);

private:
	Array<Spatial*, kMaxSpatialsPerVisTest> m_spatials;
	U32 m_spatialCount = 0;

	/// Submit tasks to test the m_spatials.
	void flush(ThreadHive& hive);
};
static_assert(std::is_trivially_destructible<GatherVisiblesFromOctreeTask>::value == true,
			  "Should be trivially destructible");

/// ThreadHive task that does the actual visibility tests.
class VisibilityTestTask
{
public:
	FrustumVisibilityContext* m_frcCtx = nullptr;

	Array<Spatial*, kMaxSpatialsPerVisTest> m_spatialsToTest;
	U32 m_spatialToTestCount = 0;

	VisibilityTestTask(FrustumVisibilityContext* frcCtx)
		: m_frcCtx(frcCtx)
	{
		ANKI_ASSERT(m_frcCtx);
	}

	void test(ThreadHive& hive, U32 taskId);

private:
	[[nodiscard]] Bool testAgainstRasterizer(const Aabb& aabb) const
	{
		return (m_frcCtx->m_r) ? m_frcCtx->m_r->visibilityTest(aabb) : true;
	}
};
static_assert(std::is_trivially_destructible<VisibilityTestTask>::value == true, "Should be trivially destructible");

/// Task that combines and sorts the results.
class CombineResultsTask
{
public:
	FrustumVisibilityContext* m_frcCtx = nullptr;

	CombineResultsTask(FrustumVisibilityContext* frcCtx)
		: m_frcCtx(frcCtx)
	{
		ANKI_ASSERT(m_frcCtx);
	}

	void combine();

private:
	template<typename T>
	static void combineQueueElements(WeakArray<TRenderQueueElementStorage<T>> subStorages,
									 WeakArray<TRenderQueueElementStorage<U32>>* ptrSubStorage, WeakArray<T>& combined,
									 WeakArray<T*>* ptrCombined);
};
static_assert(std::is_trivially_destructible<CombineResultsTask>::value == true, "Should be trivially destructible");
/// @}

} // end namespace anki
