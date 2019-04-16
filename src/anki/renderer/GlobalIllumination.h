// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RendererObject.h>
#include <anki/renderer/TraditionalDeferredShading.h>

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

private:
	class
	{
	public:
		Array<RenderTargetDescription, GBUFFER_COLOR_ATTACHMENT_COUNT> m_colorRtDescrs;
		RenderTargetDescription m_depthRtDescr;
		FramebufferDescription m_fbDescr;
		U32 m_tileSize = 0;
	} m_gbuffer; ///< G-buffer pass.

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
		U64 m_probUuid;
		Timestamp m_lastUsedTimestamp = 0; ///< When it was rendered.
		Array<TexturePtr, 6> m_volumeTextures; ///< One for the 6 directions of the ambient dice.
		UVec3 m_volumeSize;
		U32 m_renderedCells;
	};

	DynamicArray<CacheEntry> m_cacheEntries;
	HashMap<U64, U32> m_probeUuidToCacheEntryIdx;

	class
	{
	public:
		RenderingContext* m_ctx ANKI_DEBUG_CODE(= nullptr);
	} m_runCtx;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initGBuffer(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initLightShading(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIrradiance(const ConfigSet& cfg);

	void run(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
