// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/renderer/TraditionalDeferredShading.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/collision/Forward.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// Ambient global illumination passes.
///
/// It builds a volume clipmap with ambient GI information.
class GlobalIllumination : public RendererObject
{
public:
	GlobalIllumination(Renderer* r)
		: RendererObject(r)
		, m_lightShading(r)
	{
	}

	~GlobalIllumination();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Return the volume RT given a cache entry index.
	const RenderTargetHandle& getVolumeRenderTarget(const GlobalIlluminationProbeQueueElement& probe) const;

	/// Set the render graph dependencies.
	void setRenderGraphDependencies(RenderingContext& ctx, RenderPassDescriptionBase& pass,
									TextureUsageBit usage) const;

	/// Bind the volume textures to a command buffer.
	void bindVolumeTextures(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx, U32 set, U32 binding) const;

private:
	class InternalContext;

	class CacheEntry
	{
	public:
		U64 m_uuid; ///< Probe UUID.
		Timestamp m_lastUsedTimestamp = 0; ///< When it was last seen by the renderer.
		TexturePtr m_volumeTex; ///< Contains the 6 directions.
		UVec3 m_volumeSize = UVec3(0u);
		Vec3 m_probeAabbMin = Vec3(0.0f);
		Vec3 m_probeAabbMax = Vec3(0.0f);
		U32 m_renderedCells = 0;
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

	InternalContext* m_giCtx = nullptr;
	DynamicArray<CacheEntry> m_cacheEntries;
	HashMap<U64, U32> m_probeUuidToCacheEntryIdx;
	U32 m_tileSize = 0;
	U32 m_maxVisibleProbes = 0;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initShadowMapping(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);

	void runGBufferInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;
	void runShadowmappingInThread(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx) const;
	void runLightShading(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx);
	void runIrradiance(RenderPassWorkContext& rgraphCtx, InternalContext& giCtx);

	void prepareProbes(InternalContext& giCtx);
};
/// @}

} // end namespace anki
