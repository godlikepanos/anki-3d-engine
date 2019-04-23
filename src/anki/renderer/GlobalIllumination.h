// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/renderer/TraditionalDeferredShading.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

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

	/// Given a probe return a volume render target that contains the irradiance.
	const Array<RenderTargetHandle, 6>& getProbeVolumeRenderTargets(
		const GlobalIlluminationProbeQueueElement& probe) const
	{
		const CacheEntry& entry = m_cacheEntries[probe.m_cacheEntryIndex];
		ANKI_ASSERT(entry.m_uuid == probe.m_uuid);
		return entry.m_rtHandles;
	}

private:
	class InternalContext;

	class CacheEntry
	{
	public:
		U64 m_uuid; ///< Probe UUID.
		Timestamp m_lastUsedTimestamp = 0; ///< When it was last seen by the renderer.
		Array<TexturePtr, 6> m_volumeTextures; ///< One for the 6 directions of the ambient dice.
		UVec3 m_volumeSize = UVec3(0u);
		Vec3 m_probeAabbMin = Vec3(0.0f);
		Vec3 m_probeAabbMax = Vec3(0.0f);
		U32 m_renderedCells = 0;
		Array<RenderTargetHandle, 6> m_rtHandles;
	};

	class
	{
	public:
		Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
		FramebufferDescription m_fbDescr;
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

	DynamicArray<CacheEntry> m_cacheEntries;
	HashMap<U64, U32> m_probeUuidToCacheEntryIdx;
	U32 m_tileSize = 0;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initShadowMapping(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);

	void runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;
	void runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;
	void runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx);
	void runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;

	void prepareProbes(RenderingContext& rctx,
		GlobalIlluminationProbeQueueElement*& probeToUpdateThisFrame,
		UVec3& probeToUpdateThisFrameCell);
};
/// @}

} // end namespace anki
