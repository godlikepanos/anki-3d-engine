// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/RenderingKey.h>
#include <anki/scene/Forward.h>
#include <anki/Gr.h>

namespace anki
{

// Forward
class Renderer;
class DrawContext;
class CompleteRenderingBuildInfo;

/// @addtogroup renderer
/// @{

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
	friend class SetupRenderableVariableVisitor;
	friend class RenderTask;

public:
	RenderableDrawer(Renderer* r)
		: m_r(r)
	{
	}

	~RenderableDrawer();

	ANKI_USE_RESULT Error drawRange(Pass pass,
		const Mat4& viewMat,
		const Mat4& viewProjMat,
		CommandBufferPtr cmdb,
		const VisibleNode* begin,
		const VisibleNode* end);

private:
	Renderer* m_r;

	ANKI_USE_RESULT Error flushDrawcall(DrawContext& ctx, CompleteRenderingBuildInfo& build);
	void setupUniforms(DrawContext& ctx, CompleteRenderingBuildInfo& build);

	ANKI_USE_RESULT Error drawSingle(DrawContext& ctx);
};
/// @}

} // end namespace anki
