// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Global illumination probe component. It's an axis aligned box divided into cells.
class GlobalIlluminationProbeComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::GLOBAL_ILLUMINATION_PROBE;

	GlobalIlluminationProbeComponent(U64 uuid)
		: SceneComponent(CLASS_TYPE)
		, m_uuid(uuid)
	{
		ANKI_ASSERT(uuid > 0);
	}

	ANKI_USE_RESULT Error init(ResourceManager& rsrc);

	/// Set the bounding box in world coordinates.
	void setBoundingBox(const Vec4& min, const Vec4& max)
	{
		ANKI_ASSERT(min.xyz() < max.xyz());
		m_aabbMin = min.xyz();
		m_aabbMax = max.xyz();
		updateMembers();
		m_dirty = true;
	}

	/// Set the cell size in meters.
	void setCellSize(F32 cellSize)
	{
		ANKI_ASSERT(cellSize > 0.0f);
		m_cellSize = cellSize;
		updateMembers();
		m_dirty = true;
	}

	F32 getCellSize() const
	{
		return m_cellSize;
	}

	Vec4 getAlignedBoundingBoxMin() const
	{
		return m_aabbMin.xyz0();
	}

	Vec4 getAlignedBoundingBoxMax() const
	{
		return m_aabbMax.xyz0();
	}

	/// Returns true if it's marked for update this frame.
	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	/// Get the cell position that will be rendered this frame.
	Vec4 getRenderPosition() const
	{
		ANKI_ASSERT(m_renderPosition > m_aabbMin && m_renderPosition < m_aabbMax);
		ANKI_ASSERT(m_markedForRendering);
		return m_renderPosition.xyz0();
	}

	void setupGlobalIlluminationProbeQueueElement(GlobalIlluminationProbeQueueElement& el)
	{
		el.m_uuid = m_uuid;
		el.m_feedbackCallback = giProbeQueueElementFeedbackCallback;
		el.m_debugDrawCallback = debugDrawCallback;
		el.m_userData = this;
		el.m_renderQueues = {};
		el.m_aabbMin = m_aabbMin;
		el.m_aabbMax = m_aabbMax;
		el.m_cellCounts = m_cellCounts;
		el.m_totalCellCount = m_cellCounts.x() * m_cellCounts.y() * m_cellCounts.z();
		el.m_cellSizes = (m_aabbMax - m_aabbMin) / Vec3(m_cellCounts);
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = m_dirty;
		m_dirty = false;
		return Error::NONE;
	}

private:
	ShaderProgramResourcePtr m_dbgProg;
	U64 m_uuid;
	Vec3 m_aabbMin = Vec3(-1.0f);
	Vec3 m_aabbMax = Vec3(+1.0f);
	Vec3 m_renderPosition = Vec3(0.0f);
	F32 m_cellSize = 4.0f; ///< Cell size in meters.
	UVec3 m_cellCounts = UVec3(2u);
	Bool m_markedForRendering = false;
	Bool m_dirty = true;

	static void giProbeQueueElementFeedbackCallback(
		Bool fillRenderQueuesOnNextFrame, void* userData, const Vec4& eyeWorldPosition)
	{
		ANKI_ASSERT(userData);
		GlobalIlluminationProbeComponent& self = *static_cast<GlobalIlluminationProbeComponent*>(userData);
		ANKI_ASSERT(eyeWorldPosition.xyz() > self.m_aabbMin && eyeWorldPosition.xyz() < self.m_aabbMax);
		self.m_markedForRendering = fillRenderQueuesOnNextFrame;
		self.m_renderPosition = eyeWorldPosition.xyz();
	}

	static void debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		debugDraw(ctx, userData);
	}

	static void debugDraw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);

	/// Recalc come values.
	void updateMembers()
	{
		for(U i = 0; i < 3; ++i)
		{
			ANKI_ASSERT(m_aabbMax[i] > m_aabbMin[i]);

			const F32 dist = m_aabbMax[i] - m_aabbMin[i];
			m_cellCounts[i] = U32(ceil(dist / m_cellSize));
			ANKI_ASSERT(m_cellCounts[i] > 0);
		}
	}
};
/// @}

} // end namespace anki
