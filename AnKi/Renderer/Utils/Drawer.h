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

	UVec4 m_viewport; ///< Only used for information purposes.

	Sampler* m_sampler = nullptr;

	// For MDI
	RenderingTechnique m_renderingTechinuqe = RenderingTechnique::kCount;

	BufferOffsetRange m_mdiDrawCountsBuffer;
	BufferOffsetRange m_drawIndexedIndirectArgsBuffer;
	BufferOffsetRange m_instanceRateRenderablesBuffer;

	BufferOffsetRange m_taskShaderIndirectArgsBuffer;
	BufferOffsetRange m_taskShaderPayloadsBuffer;

	void fillMdi(const GpuVisibilityOutput& visOut)
	{
		m_mdiDrawCountsBuffer = visOut.m_mdiDrawCountsBuffer;
		m_drawIndexedIndirectArgsBuffer = visOut.m_drawIndexedIndirectArgsBuffer;
		m_instanceRateRenderablesBuffer = visOut.m_instanceRateRenderablesBuffer;
		m_taskShaderIndirectArgsBuffer = visOut.m_taskShaderIndirectArgsBuffer;
		m_taskShaderPayloadsBuffer = visOut.m_taskShaderPayloadBuffer;
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
	void setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb);
};
/// @}

} // end namespace anki
