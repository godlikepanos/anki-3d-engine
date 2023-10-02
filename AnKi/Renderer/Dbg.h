// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Util/Enum.h>

namespace anki {

// Forward
extern BoolCVar g_dbgCVar;

/// @addtogroup renderer
/// @{

/// Debugging stage
class Dbg : public RendererObject
{
public:
	Dbg();

	~Dbg();

	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getRt() const
	{
		return m_runCtx.m_rt;
	}

	Bool getDepthTestEnabled() const
	{
		return m_depthTestOn;
	}

	void setDepthTestEnabled(Bool enable)
	{
		m_depthTestOn = enable;
	}

	void switchDepthTestEnabled()
	{
		m_depthTestOn = !m_depthTestOn;
	}

	Bool getDitheredDepthTestEnabled() const
	{
		return m_ditheredDepthTestOn;
	}

	void setDitheredDepthTestEnabled(Bool enable)
	{
		m_ditheredDepthTestOn = enable;
	}

	void switchDitheredDepthTestEnabled()
	{
		m_ditheredDepthTestOn = !m_ditheredDepthTestOn;
	}

private:
	RenderTargetDescription m_rtDescr;
	FramebufferDescription m_fbDescr;

	ImageResourcePtr m_giProbeImage;
	ImageResourcePtr m_pointLightImage;
	ImageResourcePtr m_spotLightImage;
	ImageResourcePtr m_decalImage;
	ImageResourcePtr m_reflectionImage;

	BufferPtr m_cubeVertsBuffer;
	BufferPtr m_cubeIndicesBuffer;

	ShaderProgramResourcePtr m_renderablesProg;
	ShaderProgramResourcePtr m_nonRenderablesProg;

	Bool m_depthTestOn : 1 = false;
	Bool m_ditheredDepthTestOn : 1 = false;

	class
	{
	public:
		RenderTargetHandle m_rt;
	} m_runCtx;

	void run(RenderPassWorkContext& rgraphCtx, const RenderingContext& ctx);

	void drawNonRenderable(GpuSceneNonRenderableObjectType type, U32 objCount, const RenderingContext& ctx, const ImageResource& image,
						   CommandBuffer& cmdb);
};
/// @}

} // end namespace anki
