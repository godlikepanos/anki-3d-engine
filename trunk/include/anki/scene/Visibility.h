// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_VISIBILITY_TEST_RESULTS_H
#define ANKI_SCENE_VISIBILITY_TEST_RESULTS_H

#include "anki/scene/Common.h"
#include "anki/collision/Forward.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/RenderComponent.h"
#include "anki/core/Threadpool.h"
#include "anki/util/NonCopyable.h"

namespace anki {

// Forward
class Renderer;

/// @addtogroup Scene
/// @{

/// Visibility test type
enum VisibilityTest
{
	VT_RENDERABLES = 1 << 0,
	VT_ONLY_SHADOW_CASTERS = 1 << 1,
	VT_LIGHTS = 1 << 2
};

/// Visible by
enum VisibleBy
{
	VB_NONE = 0,
	VB_CAMERA = 1 << 0,
	VB_LIGHT = 1 << 1
};

/// Visible node pointer with some more info
/// @note Keep this structore as small as possible
class VisibleNode: public NonCopyable
{
public:
	SceneNode* m_node;
	/// An array of the visible spatials
	U8* m_spatialIndices;
	U8 m_spatialsCount;

	VisibleNode()
		: m_node(nullptr), m_spatialIndices(nullptr), m_spatialsCount(0)
	{}

	VisibleNode(VisibleNode&& other)
	{
		*this = std::move(other);
	}

	VisibleNode& operator=(VisibleNode&& other)
	{
		m_node = other.m_node;
		m_spatialIndices = other.m_spatialIndices;
		m_spatialsCount = other.m_spatialsCount;

		other.m_node = nullptr;
		other.m_spatialIndices = nullptr;
		other.m_spatialsCount = 0;

		return *this;
	}

	U8 getSpatialIndex(U i)
	{
		ANKI_ASSERT(m_spatialsCount != 0 && i < m_spatialsCount);
		return m_spatialIndices[i];
	}
};

/// Its actually a container for visible entities. It should be per frame
class VisibilityTestResults
{
public:
	typedef SceneFrameVector<VisibleNode> Container;

	Container m_renderables;
	Container m_lights;

	VisibilityTestResults(const SceneAllocator<U8>& frameAlloc,
		U32 renderablesReservedSize = 
			ANKI_FRUSTUMABLE_AVERAGE_VISIBLE_RENDERABLES_COUNT,
		U32 lightsReservedSize =
			ANKI_FRUSTUMABLE_AVERAGE_VISIBLE_LIGHTS_COUNT)
		: m_renderables(frameAlloc), m_lights(frameAlloc)
	{
		m_renderables.reserve(renderablesReservedSize);
		m_lights.reserve(lightsReservedSize);
	}
};

/// Sort spatial scene nodes on distance
class DistanceSortFunctor
{
public:
	Vec3 m_origin;

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

/// Thread job to short scene nodes by distance
class DistanceSortJob: public ThreadpoolTask					
{
public:
	U32 m_nodesCount;
	VisibilityTestResults::Container::iterator m_nodes;
	Vec3 m_origin;

	void operator()(ThreadId /*threadId*/, U /*threadsCount*/)
	{
		DistanceSortFunctor comp;
		comp.m_origin = m_origin;
		std::sort(m_nodes, m_nodes + m_nodesCount, comp);
	}
};

/// Thread job to short renderable scene nodes by material
class MaterialSortJob: public ThreadpoolTask
{
public:
	U32 m_nodesCount;
	VisibilityTestResults::Container::iterator m_nodes;

	void operator()(ThreadId /*threadId*/, U /*threadsCount*/)
	{
		std::sort(m_nodes, m_nodes + m_nodesCount, MaterialSortFunctor());
	}
};

/// Do visibility tests bypassing portals 
void doVisibilityTests(SceneNode& frustumable, SceneGraph& scene, 
	Renderer& renderer);

/// @}

} // end namespace anki

#endif
