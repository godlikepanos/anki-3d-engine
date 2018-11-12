// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneGraph.h>
#include <anki/scene/SoftwareRasterizer.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/Octree.h>
#include <anki/util/Thread.h>
#include <anki/core/Trace.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

static const U32 MAX_SPATIALS_PER_VIS_TEST = 48; ///< Num of spatials to test in a single ThreadHive task.
static const U32 SW_RASTERIZER_WIDTH = 80;
static const U32 SW_RASTERIZER_HEIGHT = 50;

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

/// Material and distance sort.
class MaterialDistanceSortFunctor
{
public:
	MaterialDistanceSortFunctor(F32 distanceGranularity)
		: m_distGranularity(1.0f / distanceGranularity)
	{
	}

	Bool operator()(const RenderableQueueElement& a, const RenderableQueueElement& b)
	{
		const U aClass = a.m_distanceFromCamera * m_distGranularity;
		const U bClass = b.m_distanceFromCamera * m_distGranularity;

		if(aClass == bClass && a.m_callback == b.m_callback)
		{
			return a.m_mergeKey < b.m_mergeKey;
		}
		else
		{
			return a.m_distanceFromCamera < b.m_distanceFromCamera;
		}
	}

private:
	F32 m_distGranularity;
};

/// Storage for a single element type.
template<typename T, U INITIAL_STORAGE_SIZE = 32, U STORAGE_GROW_RATE = 4>
class TRenderQueueElementStorage
{
public:
	T* m_elements = nullptr;
	U32 m_elementCount = 0;
	U32 m_elementStorage = 0;

	T* newElement(SceneFrameAllocator<T> alloc)
	{
		if(ANKI_UNLIKELY(m_elementCount + 1 > m_elementStorage))
		{
			m_elementStorage = max(INITIAL_STORAGE_SIZE, m_elementStorage * STORAGE_GROW_RATE);

			const T* oldElements = m_elements;
			m_elements = alloc.allocate(m_elementStorage);

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
	TRenderQueueElementStorage<U32> m_shadowPointLights;
	TRenderQueueElementStorage<SpotLightQueueElement> m_spotLights;
	TRenderQueueElementStorage<U32> m_shadowSpotLights;
	TRenderQueueElementStorage<ReflectionProbeQueueElement> m_reflectionProbes;
	TRenderQueueElementStorage<LensFlareQueueElement> m_lensFlares;
	TRenderQueueElementStorage<DecalQueueElement> m_decals;
	TRenderQueueElementStorage<FogDensityQueueElement> m_fogDensityVolumes;

	Timestamp m_timestamp = 0;
};

static_assert(std::is_trivially_destructible<RenderQueueView>::value == true, "Should be trivially destructible");

/// Data common for all tasks.
class VisibilityContext
{
public:
	SceneGraph* m_scene = nullptr;
	Atomic<U32> m_testsCount = {0};

	F32 m_earlyZDist = -1.0f; ///< Cache this.

	List<const FrustumComponent*> m_testedFrcs;
	Mutex m_mtx;

	void submitNewWork(const FrustumComponent& frc, RenderQueue& result, ThreadHive& hive);
};

/// A context for a specific test of a frustum component.
/// @note Should be trivially destructible.
class FrustumVisibilityContext
{
public:
	VisibilityContext* m_visCtx = nullptr;
	const FrustumComponent* m_frc = nullptr;

	// S/W rasterizer members
	SoftwareRasterizer* m_r = nullptr;
	DynamicArray<Vec3> m_verts;
	Atomic<U32> m_rasterizedVertCount = {0}; ///< That will be used by the RasterizeTrianglesTask.

	// Visibility test members
	DynamicArray<RenderQueueView> m_queueViews; ///< Sub result. Will be combined later.
	ThreadHiveSemaphore* m_visTestsSignalSem = nullptr;

	// Gather results members
	RenderQueue* m_renderQueue = nullptr;
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
static_assert(
	std::is_trivially_destructible<FillRasterizerWithCoverageTask>::value == true, "Should be trivially destructible");

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
	Array<SpatialComponent*, MAX_SPATIALS_PER_VIS_TEST> m_spatials;
	U32 m_spatialCount = 0;

	/// Submit tasks to test the m_spatials.
	void flush(ThreadHive& hive);
};
static_assert(
	std::is_trivially_destructible<GatherVisiblesFromOctreeTask>::value == true, "Should be trivially destructible");

/// ThreadHive task that does the actual visibility tests.
class VisibilityTestTask
{
public:
	FrustumVisibilityContext* m_frcCtx = nullptr;

	Array<SpatialComponent*, MAX_SPATIALS_PER_VIS_TEST> m_spatialsToTest;
	U32 m_spatialToTestCount = 0;

	VisibilityTestTask(FrustumVisibilityContext* frcCtx)
		: m_frcCtx(frcCtx)
	{
		ANKI_ASSERT(m_frcCtx);
	}

	void test(ThreadHive& hive, U32 taskId);

private:
	ANKI_USE_RESULT Bool testAgainstRasterizer(const CollisionShape& cs, const Aabb& aabb) const
	{
		return (m_frcCtx->m_r) ? m_frcCtx->m_r->visibilityTest(cs, aabb) : true;
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
	static void combineQueueElements(SceneFrameAllocator<U8>& alloc,
		WeakArray<TRenderQueueElementStorage<T>> subStorages,
		WeakArray<TRenderQueueElementStorage<U32>>* ptrSubStorage,
		WeakArray<T>& combined,
		WeakArray<T*>* ptrCombined);
};
static_assert(std::is_trivially_destructible<CombineResultsTask>::value == true, "Should be trivially destructible");
/// @}

} // end namespace anki
