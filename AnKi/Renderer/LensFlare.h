// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Lens flare rendering pass. Part of forward shading.
class LensFlare : public RendererObject
{
public:
	Error init();

	void runDrawFlares(const RenderingContext& ctx, CommandBufferPtr& cmdb);

	void populateRenderGraph(RenderingContext& ctx);

	/// Get it to set a dependency.
	BufferHandle getIndirectDrawBuffer() const
	{
		return m_runCtx.m_indirectBuffHandle;
	}

private:
	// Occlusion test
	BufferPtr m_indirectBuff;
	ShaderProgramResourcePtr m_updateIndirectBuffProg;
	ShaderProgramPtr m_updateIndirectBuffGrProg;

	// Sprite billboards
	ShaderProgramResourcePtr m_realProg;
	ShaderProgramPtr m_realGrProg;
	U8 m_maxSpritesPerFlare;
	U8 m_maxFlares;
	U16 m_maxSprites;

	class
	{
	public:
		BufferHandle m_indirectBuffHandle;
	} m_runCtx;

	Error initSprite();
	Error initOcclusion();

	Error initInternal();

	void updateIndirectInfo(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
