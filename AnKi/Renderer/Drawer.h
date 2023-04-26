// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Resource/RenderingKey.h>
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

	Buffer* m_mdiDrawCountsBuffer = nullptr;
	PtrSize m_mdiDrawCountsBufferOffset = 0;
	PtrSize m_mdiDrawCountsBufferRange = 0;

	Buffer* m_drawIndexedIndirectArgsBuffer = nullptr;
	PtrSize m_drawIndexedIndirectArgsBufferOffset = 0;
	PtrSize m_drawIndexedIndirectArgsBufferRange = 0;

	Buffer* m_instaceRateRenderablesBuffer = nullptr;
	PtrSize m_instaceRateRenderablesOffset = 0;
	PtrSize m_instaceRateRenderablesRange = 0;
};

/// It uses the render queue to batch and render.
class RenderableDrawer
{
	friend class RenderTask;

public:
	RenderableDrawer() = default;

	~RenderableDrawer();

	void drawRange(const RenderableDrawerArguments& args, const RenderableQueueElement* begin, const RenderableQueueElement* end,
				   CommandBuffer& cmdb);

	/// Draw using multidraw indirect.
	void drawMdi(const RenderableDrawerArguments& args, CommandBuffer& cmdb);

private:
	class Context;

	void setState(const RenderableDrawerArguments& args, CommandBuffer& cmdb);

	void flushDrawcall(Context& ctx, CommandBuffer& cmdb);

	void drawSingle(const RenderableQueueElement* renderEl, Context& ctx, CommandBuffer& cmdb);
};
/// @}

} // end namespace anki
