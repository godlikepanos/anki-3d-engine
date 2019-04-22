// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/renderer/TraditionalDeferredShading.h>

namespace anki
{

// Forward
class GiProbeQueueElement;

/// @addtogroup renderer
/// @{

/// Global illumination.
class GlobalIllumination : public RendererObject
{
anki_internal:
	GlobalIllumination(Renderer* r)
		: RendererObject(r)
		, m_lightShading(r)
	{
	}

	~GlobalIllumination();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

private:
	class
	{
	public:
		Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
		FramebufferDescription m_fbDescr;
		U32 m_tileSize = 0;
	} m_gbuffer; ///< G-buffer pass.

	class
	{
	public:
		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;
		SamplerPtr m_shadowSampler;
	} m_shadowMapping;

	class LS
	{
	public:
		RenderTargetDescription m_rtDescr;
		FramebufferDescription m_fbDescr;
		TraditionalDeferredLightShading m_deferred;
		U32 m_tileSize = 0;

		LS(Renderer* r)
			: m_deferred(r)
		{
		}
	} m_lightShading; ///< Light shading.

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradiance; ///< Irradiance.

	class CacheEntry
	{
	public:
		U64 m_uuid; ///< Probe UUID.
		Timestamp m_lastUsedTimestamp = 0; ///< When it was last seen by the renderer.
		Array<TexturePtr, 6> m_volumeTextures; ///< One for the 6 directions of the ambient dice.
		UVec3 m_volumeSize = UVec3(0u);
		U32 m_renderedCells = 0;
		Array<RenderTargetHandle, 6> m_rtHandles;
	};

	DynamicArray<CacheEntry> m_cacheEntries;
	HashMap<U64, U32> m_probeUuidToCacheEntryIdx;

	class
	{
	public:
		RenderingContext* m_ctx ANKI_DEBUG_CODE(= nullptr);
		GiProbeQueueElement* m_probe ANKI_DEBUG_CODE(= nullptr);
		Array<RenderTargetHandle, GBUFFER_COLOR_ATTACHMENT_COUNT> m_gbufferColorRts;
		RenderTargetHandle m_gbufferDepthRt;
		RenderTargetHandle m_shadowsRt;
	} m_ctx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initShadowMapping(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);

	void runGBufferInThread(RenderPassWorkContext& rgraphCtx) const;
	void runShadowmappingInThread(RenderPassWorkContext& rgraphCtx) const;
	void runLightShading(RenderPassWorkContext& rgraphCtx);
	void runIrradiance(RenderPassWorkContext& rgraphCtx) const;

	void prepareProbes(RenderingContext& rctx,
		GiProbeQueueElement*& probeToUpdateThisFrame,
		U32& probeToUpdateThisFrameCacheEntryIdx,
		Vec4& cellWorldPosition);
};
/// @}

} // end namespace anki
