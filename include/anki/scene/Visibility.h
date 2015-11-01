// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/collision/Forward.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/RenderComponent.h>
#include <anki/util/NonCopyable.h>

namespace anki {

// Forward
class Renderer;

/// @addtogroup scene
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
class VisibleNode
{
public:
	SceneNode* m_node = nullptr;
	/// An array of the visible spatials
	U8* m_spatialIndices = nullptr;
	U8 m_spatialsCount = 0;

	VisibleNode()
	{}

	VisibleNode(const VisibleNode& other)
	{
		*this = other;
	}

	VisibleNode& operator=(const VisibleNode& other)
	{
		m_node = other.m_node;
		m_spatialIndices = other.m_spatialIndices;
		m_spatialsCount = other.m_spatialsCount;
		return *this;
	}

	U8 getSpatialIndex(U i) const
	{
		ANKI_ASSERT(m_spatialsCount != 0 && i < m_spatialsCount);
		return m_spatialIndices[i];
	}
};

/// Its actually a container for visible entities. It should be per frame
class VisibilityTestResults
{
public:
	~VisibilityTestResults()
	{
		ANKI_ASSERT(0 && "It's supposed to be deallocated on frame start");
	}

	void create(
		SceneFrameAllocator<U8> alloc,
		U32 renderablesReservedSize,
		U32 lightsReservedSize,
		U32 lensFlaresReservedSize,
		U32 reflectionProbesReservedSize);

	void prepareMerge();

	VisibleNode* getRenderablesBegin()
	{
		return getBegin(RENDERABLES);
	}

	VisibleNode* getRenderablesEnd()
	{
		return getEnd(RENDERABLES);
	}

	VisibleNode* getLightsBegin()
	{
		return getBegin(LIGHTS);
	}

	VisibleNode* getLightsEnd()
	{
		return getEnd(LIGHTS);
	}

	VisibleNode* getLensFlaresBegin()
	{
		return getBegin(FLARES);
	}

	VisibleNode* getLensFlaresEnd()
	{
		return getEnd(FLARES);
	}

	VisibleNode* getReflectionProbesBegin()
	{
		return getBegin(REFLECTION_PROBES);
	}

	VisibleNode* getReflectionProbesEnd()
	{
		return getEnd(REFLECTION_PROBES);
	}

	VisibleNode* getReflectionProxiesBegin()
	{
		return getBegin(REFLECTION_PROXIES);
	}

	VisibleNode* getReflectionProxiesEnd()
	{
		return getEnd(REFLECTION_PROXIES);
	}

	U32 getRenderablesCount() const
	{
		return getCount(RENDERABLES);
	}

	U32 getLightsCount() const
	{
		return getCount(LIGHTS);
	}

	U32 getLensFlaresCount() const
	{
		return getCount(FLARES);
	}

	U32 getReflectionProbeCount() const
	{
		return getCount(REFLECTION_PROBES);
	}

	U32 getReflectionProxyCount() const
	{
		return getCount(REFLECTION_PROBES);
	}

	void moveBackRenderable(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, RENDERABLES, x);
	}

	void moveBackLight(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, LIGHTS, x);
	}

	void moveBackLensFlare(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, FLARES, x);
	}

	void moveBackReflectionProbe(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, REFLECTION_PROBES, x);
	}

	void moveBackReflectionProxy(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, REFLECTION_PROXIES, x);
	}

	Timestamp getShapeUpdateTimestamp() const
	{
		return m_shapeUpdateTimestamp;
	}

	void setShapeUpdateTimestamp(Timestamp t)
	{
		m_shapeUpdateTimestamp = t;
	}

	void combineWith(SceneFrameAllocator<U8> alloc,
		SArray<VisibilityTestResults*>& results);

private:
	using Container = DArray<VisibleNode>;

	enum GroupType
	{
		RENDERABLES,
		LIGHTS,
		FLARES,
		REFLECTION_PROBES,
		REFLECTION_PROXIES,
		TYPE_COUNT
	};

	class Group
	{
	public:
		Container m_nodes;
		U32 m_count = 0;
	};

	Array<Group, TYPE_COUNT> m_groups;

	Timestamp m_shapeUpdateTimestamp = 0;

	U32 getCount(GroupType type) const
	{
		return m_groups[type].m_count;
	}

	VisibleNode* getBegin(GroupType type)
	{
		return (getCount(type)) ? &m_groups[type].m_nodes[0] : nullptr;
	}

	VisibleNode* getEnd(GroupType type)
	{
		return (getCount(type))
			? (&m_groups[type].m_nodes[0] + getCount(type)) : nullptr;
	}

	void moveBack(SceneFrameAllocator<U8> alloc, GroupType, VisibleNode& x);
};

/// Do visibility tests bypassing portals
ANKI_USE_RESULT Error doVisibilityTests(
	SceneNode& frustumable,
	SceneGraph& scene,
	const Renderer& renderer);
/// @}

} // end namespace anki

