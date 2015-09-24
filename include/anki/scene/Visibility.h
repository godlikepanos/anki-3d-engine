// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
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
class MainRenderer;

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
	using Container = DArray<VisibleNode>;

	~VisibilityTestResults()
	{
		ANKI_ASSERT(0 && "It's supposed to be deallocated on frame start");
	}

	void create(
		SceneFrameAllocator<U8> alloc,
		U32 renderablesReservedSize,
		U32 lightsReservedSize,
		U32 lensFlaresReservedSize);

	void prepareMerge()
	{
		ANKI_ASSERT(m_renderablesCount == 0
			&& m_lightsCount == 0
			&& m_flaresCount == 0);
		m_renderablesCount = m_renderables.getSize();
		m_lightsCount = m_lights.getSize();
		m_flaresCount = m_flares.getSize();
	}

	VisibleNode* getRenderablesBegin()
	{
		return (m_renderablesCount) ? &m_renderables[0] : nullptr;
	}

	VisibleNode* getRenderablesEnd()
	{
		return (m_renderablesCount)
			? (&m_renderables[0] + m_renderablesCount) : nullptr;
	}

	VisibleNode* getLightsBegin()
	{
		return (m_lightsCount) ? &m_lights[0] : nullptr;
	}

	VisibleNode* getLightsEnd()
	{
		return (m_lightsCount) ? (&m_lights[0] + m_lightsCount) : nullptr;
	}

	VisibleNode* getLensFlaresBegin()
	{
		return (m_flaresCount) ? &m_flares[0] : nullptr;
	}

	VisibleNode* getLensFlaresEnd()
	{
		return (m_flaresCount) ? (&m_flares[0] + m_flaresCount) : nullptr;
	}

	U32 getRenderablesCount() const
	{
		return m_renderablesCount;
	}

	U32 getLightsCount() const
	{
		return m_lightsCount;
	}

	U32 getLensFlaresCount() const
	{
		return m_flaresCount;
	}

	void moveBackRenderable(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, m_renderables, m_renderablesCount, x);
	}

	void moveBackLight(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, m_lights, m_lightsCount, x);
	}

	void moveBackLensFlare(SceneFrameAllocator<U8> alloc, VisibleNode& x)
	{
		moveBack(alloc, m_flares, m_flaresCount, x);
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
	Container m_renderables;
	Container m_lights;
	Container m_flares;
	U32 m_renderablesCount = 0;
	U32 m_lightsCount = 0;
	U32 m_flaresCount = 0;

	Timestamp m_shapeUpdateTimestamp = 0;

	void moveBack(SceneFrameAllocator<U8> alloc,
		Container& c, U32& count, VisibleNode& x);
};

/// Do visibility tests bypassing portals
ANKI_USE_RESULT Error doVisibilityTests(
	SceneNode& frustumable,
	SceneGraph& scene,
	MainRenderer& renderer);
/// @}

} // end namespace anki

#endif
