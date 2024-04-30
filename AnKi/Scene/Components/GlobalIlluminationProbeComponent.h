// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/GpuSceneArray.h>
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
		m_shapeDirty = true;
	}

	/// Check if any of the probe's cells need to be re-rendered.
	Bool getCellsNeedsRefresh() const
	{
		return m_cellsRefreshedCount < m_totalCellCount;
	}

	U32 getNextCellForRefresh() const
	{
		ANKI_ASSERT(getCellsNeedsRefresh());
		return m_cellsRefreshedCount;
	}

	/// Add to the number of texels that got refreshed this frame.
	void incrementRefreshedCells(U32 cellCount)
	{
		ANKI_ASSERT(getCellsNeedsRefresh());
		m_cellsRefreshedCount += cellCount;
		ANKI_ASSERT(m_cellsRefreshedCount <= m_totalCellCount);
		m_refreshDirty = true;
	}

	U32 getUuid() const
	{
		return m_uuid;
	}

	/// The radius around the probe's center that can infuence the rendering of the env texture.
	F32 getRenderRadius() const;

	F32 getShadowsRenderRadius() const;

	const Vec3& getWorldPosition() const
	{
		return m_worldPos;
	}

	const UVec3& getCellCountsPerDimension() const
	{
		return m_cellCounts;
	}

	U32 getCellCount() const
	{
		return m_totalCellCount;
	}

	Texture& getVolumeTexture() const
	{
		return *m_volTex;
	}

private:
	Vec3 m_halfSize = Vec3(0.5f);
	Vec3 m_worldPos = Vec3(0.0f);
	UVec3 m_cellCounts = UVec3(2u);
	U32 m_totalCellCount = 8u;
	F32 m_cellSize = 4.0f; ///< Cell size in meters.
	F32 m_fadeDistance = 0.2f;

	TexturePtr m_volTex;
	U32 m_volTexBindlessIdx = 0;

	GpuSceneArrays::GlobalIlluminationProbe::Allocation m_gpuSceneProbe;

	ShaderProgramResourcePtr m_clearTextureProg;

	U32 m_uuid = 0;

	U32 m_cellsRefreshedCount = 0;

	Bool m_shapeDirty = true;
	Bool m_refreshDirty = true;

	/// Recalc come values.
	void updateMembers()
	{
		const Vec3 dist = m_halfSize * 2.0f;
		m_cellCounts = UVec3(dist / m_cellSize);
		m_cellCounts = m_cellCounts.max(UVec3(1));
		m_totalCellCount = m_cellCounts.x() * m_cellCounts.y() * m_cellCounts.z();
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
