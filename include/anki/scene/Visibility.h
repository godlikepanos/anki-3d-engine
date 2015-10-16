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
		U32 lensFlaresReservedSize);

	void prepareMerge();

	VisibleNode* getRenderablesBegin()
	{
		return (getRenderablesCount())
			? &m_groups[RENDERABLES].m_nodes[0] : nullptr;
	}

	VisibleNode* getRenderablesEnd()
	{
		return (getRenderablesCount())
			? (&m_groups[RENDERABLES].m_nodes[0] + getRenderablesCount())
			: nullptr;
	}

	VisibleNode* getLightsBegin()
	{
		return (getLightsCount()) ? &m_groups[LIGHTS].m_nodes[0] : nullptr;
	}

	VisibleNode* getLightsEnd()
	{
		return (getLightsCount())
			? (&m_groups[LIGHTS].m_nodes[0] + getLightsCount()) : nullptr;
	}

	VisibleNode* getLensFlaresBegin()
	{
		return (getLensFlaresCount()) ? &m_groups[FLARES].m_nodes[0] : nullptr;
	}

	VisibleNode* getLensFlaresEnd()
	{
		return (getLensFlaresCount())
			? (&m_groups[FLARES].m_nodes[0] + getLightsCount()) : nullptr;
	}

	VisibleNode* getReflectionProbesBegin()
	{
		return (getReflectionProbeCount())
			? &m_groups[REFLECTION_PROBES].m_nodes[0] : nullptr;
	}

	VisibleNode* getReflectionProbesEnd()
	{
		return (getReflectionProbeCount())
			? (&m_groups[REFLECTION_PROBES].m_nodes[0]
			+ getReflectionProbeCount())
			: nullptr;
	}

	U32 getRenderablesCount() const
	{
		return m_groups[RENDERABLES].m_count;
	}

	U32 getLightsCount() const
	{
		return m_groups[LIGHTS].m_count;
	}

	U32 getLensFlaresCount() const
	{
		return m_groups[FLARES].m_count;
	}

	U32 getReflectionProbeCount() const
	{
		return m_groups[REFLECTION_PROBES].m_count;
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

	Timestamp getShapeUpdateTimestamp() const
	{
		return m_shapeUpdateTimestamp;
	}

	void setShapeUpdateTimestamp(Timestamp t)
	{
		m_shapeUpdateTimestamp = t;
	}

private:
	using Container = DArray<VisibleNode>;

	enum GroupType
	{
		RENDERABLES,
		LIGHTS,
		FLARES,
		REFLECTION_PROBES,
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

	void moveBack(SceneFrameAllocator<U8> alloc, GroupType, VisibleNode& x);
};

/// Do visibility tests bypassing portals
ANKI_USE_RESULT Error doVisibilityTests(
	SceneNode& frustumable,
	SceneGraph& scene,
	const Renderer& renderer);
/// @}

} // end namespace anki

