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

class RevDistanceSortFunctor
{
public:
	Bool operator()(const VisibleNode& a, const VisibleNode& b)
	{
		ANKI_ASSERT(a.m_node && b.m_node);
		return a.m_frustumDistanceSquared > b.m_frustumDistanceSquared;
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
class VisibilityContext
{
public:
	SceneGraph* m_scene = nullptr;
	Atomic<U32> m_testsCount = {0};

	List<FrustumComponent*> m_testedFrcs;
	Mutex m_mtx;

	void submitNewWork(FrustumComponent& frc, ThreadHive& hive);
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
	WeakPtr<VisibilityTestResults> m_result;
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

	/// Thread hive task.
	static void callback(void* ud, U32 threadId, ThreadHive& hive)
	{
		CombineResultsTask& self = *static_cast<CombineResultsTask*>(ud);
		self.combine();
	}

private:
	void combine();
};
/// @}

} // end namespace anki
