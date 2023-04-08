// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

	WeakArray<Frustum> getFrustums()
	{
		return m_frustums;
	}

	void setupGlobalIlluminationProbeQueueElement(GlobalIlluminationProbeQueueElement& el)
	{
		el.m_aabbMin = -m_halfSize + m_worldPos;
		el.m_aabbMax = m_halfSize + m_worldPos;
		el.m_cellCounts = m_cellCounts;
		el.m_totalCellCount = m_totalCellCount;
		el.m_cellSizes = (m_halfSize * 2.0f) / Vec3(m_cellCounts);
		el.m_fadeDistance = m_fadeDistance;
		el.m_volumeTextureBindlessIndex = m_volTexBindlessIdx;
		el.m_index = m_gpuSceneIndex;
	}

	void setupGlobalIlluminationProbeQueueElementForRefresh(GlobalIlluminationProbeQueueElementForRefresh& el)
	{
		ANKI_ASSERT(m_cellIdxToRefresh < m_totalCellCount);
		el.m_volumeTexture = m_volTex.get();
		unflatten3dArrayIndex(m_cellCounts.x(), m_cellCounts.y(), m_cellCounts.z(), m_cellIdxToRefresh,
							  el.m_cellToRefresh.x(), el.m_cellToRefresh.y(), el.m_cellToRefresh.z());
		el.m_cellCounts = m_cellCounts;
	}

	Bool needsRefresh() const
	{
		return m_cellIdxToRefresh < m_totalCellCount;
	}

	void progressRefresh()
	{
		++m_cellIdxToRefresh;
	}

private:
	Vec3 m_halfSize = Vec3(0.5f);
	Vec3 m_worldPos = Vec3(0.0f);
	UVec3 m_cellCounts = UVec3(2u);
	U32 m_totalCellCount = 8u;
	F32 m_cellSize = 4.0f; ///< Cell size in meters.
	F32 m_fadeDistance = 0.2f;

	TexturePtr m_volTex;
	TextureViewPtr m_volView;
	U32 m_volTexBindlessIdx = 0;

	U32 m_gpuSceneIndex = kMaxU32;

	Array<Frustum, 6> m_frustums;

	Spatial m_spatial;

	ShaderProgramResourcePtr m_clearTextureProg;

	U32 m_cellIdxToRefresh = 0;

	Bool m_shapeDirty = true;

	/// Recalc come values.
	void updateMembers()
	{
		const Vec3 dist = m_halfSize * 2.0f;
		m_cellCounts = UVec3(dist / m_cellSize);
		m_cellCounts = m_cellCounts.max(UVec3(1));
		m_totalCellCount = m_cellCounts.x() * m_cellCounts.y() * m_cellCounts.z();
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
