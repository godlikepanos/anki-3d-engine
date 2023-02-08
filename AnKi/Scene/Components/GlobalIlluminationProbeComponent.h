// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Collision/Aabb.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Global illumination probe component. It's an axis aligned box divided into cells.
class GlobalIlluminationProbeComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GlobalIlluminationProbeComponent)

public:
	GlobalIlluminationProbeComponent(SceneNode* node);

	~GlobalIlluminationProbeComponent();

	/// Set the bounding box size.
	void setBoxVolumeSize(const Vec3& sizeXYZ)
	{
		m_halfSize = sizeXYZ / 2.0f;
		updateMembers();
		m_shapeDirty = true;
	}

	Vec3 getBoxVolumeSize() const
	{
		return m_halfSize * 2.0f;
	}

	/// Set the cell size in meters.
	void setCellSize(F32 cellSize)
	{
		ANKI_ASSERT(cellSize > 0.0f);
		m_cellSize = cellSize;
		updateMembers();
		m_shapeDirty = true;
	}

	F32 getCellSize() const
	{
		return m_cellSize;
	}

	F32 getFadeDistance() const
	{
		return m_fadeDistance;
	}

	void setFadeDistance(F32 dist)
	{
		m_fadeDistance = max(0.0f, dist);
	}

	/// Returns true if it's marked for update this frame.
	Bool getMarkedForRendering() const
	{
		return m_markedForRendering;
	}

	/// Get the cell position that will be rendered this frame.
	Vec3 getRenderPosition() const
	{
		ANKI_ASSERT(m_renderPosition > -m_halfSize + m_worldPos && m_renderPosition < m_halfSize + m_worldPos);
		ANKI_ASSERT(m_markedForRendering);
		return m_renderPosition;
	}

	WeakArray<Frustum> getFrustums()
	{
		return m_frustums;
	}

	void setupGlobalIlluminationProbeQueueElement(GlobalIlluminationProbeQueueElement& el)
	{
		el.m_uuid = m_uuid;
		el.m_feedbackCallback = giProbeQueueElementFeedbackCallback;
		el.m_feedbackCallbackUserData = this;
		el.m_renderQueues = {};
		el.m_aabbMin = -m_halfSize + m_worldPos;
		el.m_aabbMax = m_halfSize + m_worldPos;
		el.m_cellCounts = m_cellCounts;
		el.m_totalCellCount = m_cellCounts.x() * m_cellCounts.y() * m_cellCounts.z();
		el.m_cellSizes = (m_halfSize * 2.0f) / Vec3(m_cellCounts);
		el.m_fadeDistance = m_fadeDistance;
	}

private:
	U64 m_uuid;
	Vec3 m_halfSize = Vec3(0.5f);
	Vec3 m_worldPos = Vec3(0.0f);
	Vec3 m_renderPosition = Vec3(0.0f);
	UVec3 m_cellCounts = UVec3(2u);
	F32 m_cellSize = 4.0f; ///< Cell size in meters.
	F32 m_fadeDistance = 0.2f;

	Array<Frustum, 6> m_frustums;

	Spatial m_spatial;

	Bool m_markedForRendering : 1 = false;
	Bool m_shapeDirty : 1 = true;

	static void giProbeQueueElementFeedbackCallback(Bool fillRenderQueuesOnNextFrame, void* userData,
													const Vec4& eyeWorldPosition)
	{
		ANKI_ASSERT(userData);
		GlobalIlluminationProbeComponent& self = *static_cast<GlobalIlluminationProbeComponent*>(userData);
		ANKI_ASSERT(!(fillRenderQueuesOnNextFrame
					  && (eyeWorldPosition.xyz() < -self.m_halfSize + self.m_worldPos
						  || eyeWorldPosition.xyz() > self.m_halfSize + self.m_worldPos)));
		self.m_markedForRendering = fillRenderQueuesOnNextFrame;
		self.m_renderPosition = eyeWorldPosition.xyz();
	}

	/// Recalc come values.
	void updateMembers()
	{
		const Vec3 dist = m_halfSize * 2.0f;
		m_cellCounts = UVec3(dist / m_cellSize);
		m_cellCounts = m_cellCounts.max(UVec3(1));
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	void onDestroy(SceneNode& node);
};
/// @}

} // end namespace anki
