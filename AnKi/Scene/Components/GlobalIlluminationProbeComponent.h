// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Frustum.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Collision/Aabb.h>

namespace anki {

// Global illumination probe component. It's an axis aligned box divided into cells.
class GlobalIlluminationProbeComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GlobalIlluminationProbeComponent)

public:
	GlobalIlluminationProbeComponent(SceneNode* node);

	~GlobalIlluminationProbeComponent();

	// Set the cell size in meters.
	void setCellSize(F32 cellSize)
	{
		if(ANKI_EXPECT(cellSize > 0.0f) && m_cellSize != cellSize)
		{
			m_cellSize = cellSize;
			m_dirty = true;
		}
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
		if(ANKI_EXPECT(dist > 0.0f) && m_fadeDistance != dist)
		{
			m_fadeDistance = dist;
			m_dirty = true;
		}
	}

	ANKI_INTERNAL Vec3 getBoxVolumeSize() const
	{
		return m_halfSize * 2.0f;
	}

	// Check if any of the probe's cells need to be re-rendered.
	Bool getCellsNeedsRefresh() const
	{
		return m_cellsRefreshedCount < m_totalCellCount;
	}

	ANKI_INTERNAL U32 getNextCellForRefresh() const
	{
		ANKI_ASSERT(getCellsNeedsRefresh());
		return m_cellsRefreshedCount;
	}

	// Add to the number of texels that got refreshed this frame.
	ANKI_INTERNAL void incrementRefreshedCells(U32 cellCount)
	{
		ANKI_ASSERT(getCellsNeedsRefresh());
		m_cellsRefreshedCount += cellCount;
		ANKI_ASSERT(m_cellsRefreshedCount <= m_totalCellCount);
		m_dirty = true;
	}

	// The radius around the probe's center that can infuence the rendering of the env texture.
	ANKI_INTERNAL F32 getRenderRadius() const;

	ANKI_INTERNAL F32 getShadowsRenderRadius() const;

	ANKI_INTERNAL const Vec3& getWorldPosition() const
	{
		return m_worldPos;
	}

	ANKI_INTERNAL const UVec3& getCellCountsPerDimension() const
	{
		return m_cellCounts;
	}

	ANKI_INTERNAL U32 getCellCount() const
	{
		return m_totalCellCount;
	}

	ANKI_INTERNAL Texture& getVolumeTexture() const
	{
		return *m_volTex;
	}

	ANKI_INTERNAL const GpuSceneArrays::GlobalIlluminationProbe::Allocation& getGpuSceneAllocation() const
	{
		return m_gpuSceneProbe;
	}

private:
	Vec3 m_halfSize = Vec3(0.5f);
	Vec3 m_worldPos = Vec3(0.0f);
	UVec3 m_cellCounts = UVec3(2u);
	U32 m_totalCellCount = 8u;
	F32 m_cellSize = 4.0f; // Cell size in meters.
	F32 m_fadeDistance = 0.2f;

	TexturePtr m_volTex;
	U32 m_volTexBindlessIdx = 0;

	GpuSceneArrays::GlobalIlluminationProbe::Allocation m_gpuSceneProbe;

	ShaderProgramResourcePtr m_clearTextureProg;

	U32 m_cellsRefreshedCount = 0;

	Bool m_dirty = true;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;
};

} // end namespace anki
