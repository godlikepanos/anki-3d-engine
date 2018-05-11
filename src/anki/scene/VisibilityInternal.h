// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Visibility.h>
#include <anki/scene/SectorNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SoftwareRasterizer.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/Octree.h>
#include <anki/util/Thread.h>
#include <anki/core/Trace.h>

namespace anki
{

// Forward
class FrustumComponent;
class ThreadHive;

/// @addtogroup scene
/// @{

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
};

static_assert(std::is_trivially_destructible<RenderQueueView>::value == true, "Should be trivially destructible");

/// Data common for all tasks.
class VisibilityContext
{
public:
	SceneGraph* m_scene = nullptr;
	Atomic<U32> m_testsCount = {0};

	F32 m_earlyZDist = -1.0f; ///< Cache this.

	List<FrustumComponent*> m_testedFrcs;
	Mutex m_mtx;

	void submitNewWork(FrustumComponent& frc, RenderQueue& result, ThreadHive& hive);
};

/// ThreadHive task to gather all visible triangles from the OccluderComponent.
class GatherVisibleTrianglesTask
{
public:
	WeakPtr<VisibilityContext> m_visCtx;
	WeakPtr<FrustumComponent> m_frc;

	static const U TRIANGLES_INITIAL_SIZE = 10 * 3;
	DynamicArray<Vec3> m_verts;
	U32 m_vertCount;

	SoftwareRasterizer m_r;

	Atomic<U32> m_rasterizedVertCount = {0}; ///< That will be used by the RasterizeTrianglesTask.

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		GatherVisibleTrianglesTask& self = *static_cast<GatherVisibleTrianglesTask*>(ud);
		self.gather();
	}

private:
	void gather();
};

/// ThreadHive task to rasterize triangles.
class RasterizeTrianglesTask
{
public:
	WeakPtr<GatherVisibleTrianglesTask> m_gatherTask;
	U32 m_taskIdx;
	U32 m_taskCount;

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		RasterizeTrianglesTask& self = *static_cast<RasterizeTrianglesTask*>(ud);
		self.rasterize();
	}

private:
	void rasterize();
};

/// ThreadHive task to get visible nodes from the octree.
class GatherVisiblesFromOctreeTask
{
public:
	VisibilityContext* m_visCtx ANKI_DBG_NULLIFY;
	FrustumComponent* m_frc ANKI_DBG_NULLIFY; ///< What to test against.
	WeakArray<OctreePlaceable*> m_octreePlaceables; ///< The results of the task.

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		GatherVisiblesFromOctreeTask& self = *static_cast<GatherVisiblesFromOctreeTask*>(ud);
		self.gather();
	}

private:
	void gather()
	{
		ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_OCTREE);
		U testIdx = m_visCtx->m_testsCount.fetchAdd(1);

		DynamicArrayAuto<OctreePlaceable*> arr(m_visCtx->m_scene->getFrameAllocator());
		m_visCtx->m_scene->getOctree().gatherVisible(m_frc->getFrustum(), testIdx, arr);

		if(arr.getSize() > 0)
		{
			OctreePlaceable** data;
			PtrSize size;
			PtrSize storage;
			arr.moveAndReset(data, size, storage);

			ANKI_ASSERT(data && size);
			m_octreePlaceables = WeakArray<OctreePlaceable*>(data, size);
		}
	}
};

/// ThreadHive task to get visible nodes from sectors.
class GatherVisiblesFromSectorsTask
{
public:
	WeakPtr<VisibilityContext> m_visCtx;
	SectorGroupVisibilityTestsContext m_sectorsCtx;
	WeakPtr<FrustumComponent> m_frc; ///< What to test against.
	SoftwareRasterizer* m_r;

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		GatherVisiblesFromSectorsTask& self = *static_cast<GatherVisiblesFromSectorsTask*>(ud);
		self.gather();
	}

private:
	void gather()
	{
		ANKI_TRACE_SCOPED_EVENT(SCENE_VISIBILITY_ITERATE_SECTORS);
		U testIdx = m_visCtx->m_testsCount.fetchAdd(1);

		m_visCtx->m_scene->getSectorGroup().findVisibleNodes(*m_frc, testIdx, m_r, m_sectorsCtx);
	}
};

/// ThreadHive task that does the actual visibility tests.
class VisibilityTestTask
{
public:
	WeakPtr<VisibilityContext> m_visCtx;
	WeakPtr<FrustumComponent> m_frc;
	WeakPtr<SectorGroupVisibilityTestsContext> m_sectorsCtx;
	U32 m_taskIdx;
	U32 m_taskCount;
	RenderQueueView m_result; ///< Sub result. Will be combined later.
	Timestamp m_timestamp = 0;
	SoftwareRasterizer* m_r ANKI_DBG_NULLIFY;

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		VisibilityTestTask& self = *static_cast<VisibilityTestTask*>(ud);
		self.test(hive);
	}

private:
	void test(ThreadHive& hive);
	void updateTimestamp(const SceneNode& node);

	ANKI_USE_RESULT Bool testAgainstRasterizer(const CollisionShape& cs, const Aabb& aabb) const
	{
		return (m_r) ? m_r->visibilityTest(cs, aabb) : true;
	}
};

/// Task that combines and sorts the results.
class CombineResultsTask
{
public:
	WeakPtr<VisibilityContext> m_visCtx;
	WeakPtr<FrustumComponent> m_frc;
	WeakArray<VisibilityTestTask> m_tests;

	WeakPtr<RenderQueue> m_results; ///< Where to store the results.

	SoftwareRasterizer* m_swRast = nullptr; ///< For cleanup.

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		CombineResultsTask& self = *static_cast<CombineResultsTask*>(ud);
		self.combine();
	}

private:
	void combine();

	template<typename T>
	static void combineQueueElements(SceneFrameAllocator<U8>& alloc,
		WeakArray<TRenderQueueElementStorage<T>> subStorages,
		WeakArray<TRenderQueueElementStorage<U32>>* ptrSubStorage,
		WeakArray<T>& combined,
		WeakArray<T*>* ptrCombined);
};
/// @}

} // end namespace anki
