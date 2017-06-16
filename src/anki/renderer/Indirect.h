// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderingPass.h>
#include <anki/renderer/Clusterer.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

// Forward
struct IrShaderReflectionProbe;
class IrRunContext;
class IrTaskContext;

/// @addtogroup renderer
/// @{

/// Probe reflections and irradiance.
class Indirect : public RenderingPass
{
	friend class IrTask;

anki_internal:
	Indirect(Renderer* r);

	~Indirect();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	void run(RenderingContext& ctx);

	U getReflectionTextureMipmapCount() const
	{
		return m_is.m_lightRtMipCount;
	}

	TexturePtr getIrradianceTexture() const
	{
		return m_irradiance.m_cubeArr;
	}

	TexturePtr getReflectionTexture() const
	{
		return m_is.m_lightRt;
	}

	TexturePtr getIntegrationLut() const
	{
		return m_integrationLut->getGrTexture();
	}

	SamplerPtr getIntegrationLutSampler() const
	{
		return m_integrationLutSampler;
	}

private:
	class FaceInfo
	{
	public:
		// MS
		Array<TexturePtr, GBUFFER_COLOR_ATTACHMENT_COUNT> m_gbufferColorRts;
		TexturePtr m_gbufferDepthRt;
		FramebufferPtr m_gbufferFb;

		// IS
		FramebufferPtr m_lightShadingFb;

		// Irradiance
		FramebufferPtr m_irradianceFb;

		Bool created() const
		{
			return m_lightShadingFb.isCreated();
		}
	};

	class CacheEntry : public IntrusiveHashMapEnabled<CacheEntry>
	{
	public:
		U64 m_nodeUuid;
		Timestamp m_timestamp = 0; ///< When last accessed.

		Array<FaceInfo, 6> m_faces;
	};

	static const U IRRADIANCE_TEX_SIZE = 16;
	static const U MAX_PROBE_RENDERS_PER_FRAME = 1;

	U16 m_cubemapArrSize = 0;
	U16 m_fbSize = 0;

	// IS
	class
	{
	public:
		TexturePtr m_lightRt; ///< Cube array.
		U32 m_lightRtMipCount = 0;

		ShaderProgramResourcePtr m_lightProg;
		ShaderProgramPtr m_plightGrProg;
		ShaderProgramPtr m_slightGrProg;

		BufferPtr m_plightPositions;
		BufferPtr m_plightIndices;
		U32 m_plightIdxCount;
		BufferPtr m_slightPositions;
		BufferPtr m_slightIndices;
		U32 m_slightIdxCount;
	} m_is;

	// Irradiance
	class
	{
	public:
		TexturePtr m_cubeArr;
		U32 m_cubeArrMipCount = 0;

		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_irradiance;

	DynamicArray<CacheEntry> m_cacheEntries;
	IntrusiveHashMap<U64, CacheEntry> m_uuidToCacheEntry;

	// Other
	TextureResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	// Init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIs();
	ANKI_USE_RESULT Error initIrradiance();
	void initFaceInfo(U cacheEntryIdx, U faceIdx);
	ANKI_USE_RESULT Error loadMesh(CString fname, BufferPtr& vert, BufferPtr& idx, U32& idxCount);

	void runMs(RenderingContext& rctx, const RenderQueue& rqueue, U layer, U faceIdx);
	void runIs(RenderingContext& rctx, const RenderQueue& rqueue, U layer, U faceIdx);
	void computeIrradiance(RenderingContext& rctx, U layer, U faceIdx);

	void renderReflection(const ReflectionProbeQueueElement& probeEl, RenderingContext& ctx, U cubemapIdx);

	/// Find a cache entry to store the reflection.
	void findCacheEntry(U64 nodeUuid, U& entry, Bool& render);
};
/// @}

} // end namespace anki
