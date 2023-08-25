// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Gr.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// @memberof RenderableDrawer.
class RenderableDrawerArguments
{
public:
	// The matrices are whatever the drawing needs. Sometimes they contain jittering and sometimes they don't.
	Mat3x4 m_viewMatrix;
	Mat3x4 m_cameraTransform;
	Mat4 m_viewProjectionMatrix;
	Mat4 m_previousViewProjectionMatrix;

	Sampler* m_sampler = nullptr;

	// For MDI
	RenderingTechnique m_renderingTechinuqe = RenderingTechnique::kCount;

	BufferOffsetRange m_mdiDrawCountsBuffer;
	BufferOffsetRange m_drawIndexedIndirectArgsBuffer;
	BufferOffsetRange m_instanceRateRenderablesBuffer;

	void fillMdi(const GpuVisibilityOutput& visOut)
	{
		m_mdiDrawCountsBuffer = visOut.m_mdiDrawCountsBuffer;
		m_drawIndexedIndirectArgsBuffer = visOut.m_drawIndexedIndirectArgsBuffer;
		m_instanceRateRenderablesBuffer = visOut.m_instanceRateRenderablesBuffer;
	}
};

/// It uses visibility data to issue drawcalls.
class RenderableDrawer : public RendererObject
{
public:
	RenderableDrawer() = default;

	~RenderableDrawer();

	Error init();

	/// Draw using multidraw indirect.
	/// @note It's thread-safe.
	void drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb);

private:
#if ANKI_STATS_ENABLED
	class
	{
	public:
		MultiframeReadbackToken m_readback;

		ShaderProgramResourcePtr m_statsProg;
		Array<ShaderProgramPtr, 3> m_updateStatsGrProgs;
		Array<ShaderProgramPtr, 3> m_resetCounterGrProgs;

		U64 m_frameIdx = kMaxU64;
		SpinLock m_mtx;

		Buffer* m_statsBuffer = nullptr;
		PtrSize m_statsBufferOffset = 0;
	} m_stats;
#endif

	void setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb);
};
/// @}

} // end namespace anki
