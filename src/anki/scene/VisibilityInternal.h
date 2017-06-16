// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Visibility.h>
#include <anki/scene/Sector.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SoftwareRasterizer.h>
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

/// Storage for a single element type.
template<typename T, U INITIAL_STORAGE_SIZE = 32, U STORAGE_GROW_RATE = 4>
class TRenderQueueElementStorage
{
public:
	T* m_elements = nullptr;
	U32 m_elementCount = 0;
	U32 m_elementStorage = 0;

	Timestamp m_lastUpdateTimestamp;

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
	TRenderQueueElementStorage<PointLightQueueElement> m_pointLights;
	TRenderQueueElementStorage<SpotLightQueueElement> m_spotLights;
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

	SoftwareRasterizer m_r; // TODO This will never be destroyed

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
		ANKI_TRACE_START_EVENT(SCENE_VISIBILITY_ITERATE_SECTORS);
		U testIdx = m_visCtx->m_testsCount.fetchAdd(1);

		m_visCtx->m_scene->getSectorGroup().findVisibleNodes(*m_frc, testIdx, m_r, m_sectorsCtx);
		ANKI_TRACE_STOP_EVENT(SCENE_VISIBILITY_ITERATE_SECTORS);
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

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		CombineResultsTask& self = *static_cast<CombineResultsTask*>(ud);
		self.combine();
	}

private:
	void combine();

	template<typename T>
	void combineQueueElements(
		SceneFrameAllocator<U8>& alloc, WeakArray<TRenderQueueElementStorage<T>*> subStorages, WeakArray<T>& combined);
};
/// @}

} // end namespace anki
