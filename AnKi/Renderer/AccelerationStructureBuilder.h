// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>

namespace anki {

/// @addtogroup renderer
/// @{
inline NumericCVar<F32>
	g_rayTracingExtendedFrustumDistanceCVar("R", "RayTracingExtendedFrustumDistance", 200.0f, 10.0f, 10000.0f,
											"Every object that its distance from the camera is bellow that value will take part in ray tracing");

inline NumericCVar<U32> g_lightGridCellCountXZCVar("R", "LightGridCellCountXZ", 64, 1, 1024, "The number of cells in the X and Z axis");
inline NumericCVar<U32> g_lightGridCellCountYCVar("R", "LightGridCellCountY", 4, 1, 1024, "The number of cells in the Y axis");
inline NumericCVar<F32> g_lightGridSizeXZCVar("R", "LightGridSizeXZ", 128.0f, 10.0f, 10000.0f,
											  "The size of the grid volume in the X and Z dimensions");
inline NumericCVar<F32> g_lightGridSizeYCVar("R", "LightGridSizeY", 64.0f, 10.0f, 10000.0f, "The size of the grid in the Y dimension");
inline NumericCVar<U32> g_lightIndexListSizeCVar("R", "LightIndexListSize", 64 * 1024, 128, 256 * 1024, "The light index list size");

/// @memberof AccelerationStructureBuilder
class AccelerationStructureVisibilityInfo
{
public:
	BufferHandle m_depedency; ///< Dependency for the buffer views bellow.

	BufferView m_visibleRenderablesBuffer;
	BufferView m_buildSbtIndirectArgsBuffer;
};

/// Builds acceleration structures and also bins lights to some sort of grid.
class AccelerationStructureBuilder : public RendererObject
{
public:
	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph(RenderingContext& ctx);

	AccelerationStructureHandle getAccelerationStructureHandle() const
	{
		return m_runCtx.m_tlasHandle;
	}

	void getVisibilityInfo(AccelerationStructureVisibilityInfo& asVisInfo, GpuVisibilityLocalLightsOutput& lightVisInfo) const
	{
		asVisInfo.m_depedency = m_runCtx.m_dependency;
		asVisInfo.m_visibleRenderablesBuffer = m_runCtx.m_visibleRenderablesBuff;
		asVisInfo.m_buildSbtIndirectArgsBuffer = m_runCtx.m_buildSbtIndirectArgsBuff;
		lightVisInfo = m_runCtx.m_lightVisInfo;
	}

	const LocalLightsGridConstants& getLocalLightsGridConstants() const
	{
		return m_runCtx.m_lightGridConsts;
	}

public:
	class
	{
	public:
		AccelerationStructurePtr m_tlas;
		AccelerationStructureHandle m_tlasHandle;

		BufferHandle m_dependency;
		BufferView m_visibleRenderablesBuff;
		BufferView m_buildSbtIndirectArgsBuff;

		GpuVisibilityLocalLightsOutput m_lightVisInfo;
		LocalLightsGridConstants m_lightGridConsts = {};
	} m_runCtx;
};
/// @}

} // namespace anki
