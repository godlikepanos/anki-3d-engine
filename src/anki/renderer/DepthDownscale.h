// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

// Forward
class DepthDownscale;

/// @addtogroup renderer
/// @{

/// Downscales the depth buffer a few times.
class DepthDownscale : public RendererObject
{
anki_internal:
	DepthDownscale(Renderer* r)
		: RendererObject(r)
	{
	}

	~DepthDownscale();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	RenderTargetHandle getHalfDepthColorRt() const
	{
		return m_runCtx.m_halfColorRt;
	}

	RenderTargetHandle getHalfDepthDepthRt() const
	{
		return m_runCtx.m_halfDepthRt;
	}

	RenderTargetHandle getQuarterColorRt() const
	{
		return m_runCtx.m_quarterRt;
	}

private:
	class
	{
	public:
		RenderTargetDescription m_depthRtDescr;
		RenderTargetDescription m_colorRtDescr;

		GraphicsRenderPassFramebufferDescription m_fbDescr;

		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_half; ///< Half depth pass.

	class
	{
	public:
		RenderTargetDescription m_colorRtDescr;

		GraphicsRenderPassFramebufferDescription m_fbDescr;

		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_quarter; ///< Quarter depth pass.

	class
	{
	public:
		RenderTargetHandle m_halfDepthRt;
		RenderTargetHandle m_halfColorRt;
		RenderTargetHandle m_quarterRt;
	} m_runCtx; ///< Run context.

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initHalf(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initQuarter(const ConfigSet& cfg);

	void runHalf(CommandBufferPtr& cmdb, const RenderGraph& rgraph);
	void runQuarter(CommandBufferPtr& cmdb, const RenderGraph& rgraph);

	/// A RenderPassWorkCallback for half depth main pass.
	static void runHalfCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		DepthDownscale* self = static_cast<DepthDownscale*>(userData);
		self->runHalf(cmdb, rgraph);
	}

	/// A RenderPassWorkCallback for half depth main pass.
	static void runQuarterCallback(void* userData,
		CommandBufferPtr cmdb,
		U32 secondLevelCmdbIdx,
		U32 secondLevelCmdbCount,
		const RenderGraph& rgraph)
	{
		ANKI_ASSERT(userData);
		DepthDownscale* self = static_cast<DepthDownscale*>(userData);
		self->runQuarter(cmdb, rgraph);
	}
};
/// @}

} // end namespace anki
