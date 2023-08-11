// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Resource/RenderingKey.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>
#include <AnKi/Gr.h>

namespace anki {

// Forward
class Renderer;
class RenderableQueueElement;

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

	Sampler* m_sampler;

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

/// It uses the render queue to batch and render.
class RenderableDrawer
{
	friend class RenderTask;

public:
	RenderableDrawer() = default;

	~RenderableDrawer();

	/// Draw using multidraw indirect.
	void drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb);

private:
	void setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb);
};
/// @}

} // end namespace anki
