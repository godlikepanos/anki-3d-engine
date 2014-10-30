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
	SceneNode* m_node = nullptr;
	/// An array of the visible spatials
	U8* m_spatialIndices = nullptr;
	U8 m_spatialsCount = 0;

	VisibleNode() = default;

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
	using Container = SceneFrameDArray<VisibleNode>;

	~VisibilityTestResults()
	{
		ANKI_ASSERT(0 && "It's supposed to be deallocated on frame start");
	}

	ANKI_USE_RESULT Error create(
		SceneAllocator<U8> alloc,
		U32 renderablesReservedSize,
		U32 lightsReservedSize)
	{
		Error err = m_renderables.create(alloc, renderablesReservedSize);
		if(!err)
		{
			err = m_lights.create(alloc, lightsReservedSize);
		}
		return err;
	}

	VisibleNode* getRenderablesBegin()
	{
		return &m_renderables[0];
	}

	VisibleNode* getRenderablesEnd()
	{
		return &m_renderables[0] + m_renderablesCount;
	}

	VisibleNode* getLightsBegin()
	{
		return &m_lights[0];
	}

	VisibleNode* getLightsEnd()
	{
		return &m_lights[0] + m_lightsCount;
	}

	ANKI_USE_RESULT Error moveBackRenderable(
		SceneAllocator<U8> alloc, VisibleNode& x)
	{
		return moveBack(alloc, m_renderables, m_renderablesCount, x);
	}

	ANKI_USE_RESULT Error moveBackLight(
		SceneAllocator<U8> alloc, VisibleNode& x)
	{
		return moveBack(alloc, m_lights, m_lightsCount, x);
	}

private:
	Container m_renderables;
	Container m_lights;
	U32 m_renderablesCount = 0;
	U32 m_lightsCount = 0;

	ANKI_USE_RESULT Error moveBack(
		SceneAllocator<U8> alloc, Container& c, U32& count, VisibleNode& x);
};

/// Sort spatial scene nodes on distance
class DistanceSortFunctor
{
public:
	Vec4 m_origin;

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
class DistanceSortJob: public Threadpool::Task					
{
public:
	U32 m_nodesCount;
	VisibilityTestResults::Container::iterator m_nodes;
	Vec4 m_origin;

	void operator()(U32 /*threadId*/, PtrSize /*threadsCount*/)
	{
		DistanceSortFunctor comp;
		comp.m_origin = m_origin;
		std::sort(m_nodes, m_nodes + m_nodesCount, comp);
	}
};

/// Thread job to short renderable scene nodes by material
class MaterialSortJob: public Threadpool::Task
{
public:
	U32 m_nodesCount;
	VisibilityTestResults::Container::iterator m_nodes;

	void operator()(U32 /*threadId*/, PtrSize /*threadsCount*/)
	{
		std::sort(m_nodes, m_nodes + m_nodesCount, MaterialSortFunctor());
	}
};

/// Do visibility tests bypassing portals 
ANKI_USE_RESULT Error doVisibilityTests(
	SceneNode& frustumable, 
	SceneGraph& scene, 
	Renderer& renderer);

/// @}

} // end namespace anki

#endif
