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
class ReflectionProbeComponent;

/// @addtogroup renderer
/// @{

/// Image based reflections.
class Ir : public RenderingPass
{
	friend class IrTask;

anki_internal:
	Ir(Renderer* r);

	~Ir();

	ANKI_USE_RESULT Error init(const ConfigSet& cfg);

	ANKI_USE_RESULT Error run(RenderingContext& ctx);

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
		Array<TexturePtr, MS_COLOR_ATTACHMENT_COUNT> m_gbufferColorRts;
		TexturePtr m_gbufferDepthRt;
		FramebufferPtr m_msFb;

		// IS
		FramebufferPtr m_isFb;

		// Irradiance
		FramebufferPtr m_irradianceFb;

		Bool created() const
		{
			return m_isFb.isCreated();
		}
	};

	class CacheEntry
	{
	public:
		const SceneNode* m_node = nullptr;
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

		ShaderResourcePtr m_lightVert;
		ShaderResourcePtr m_plightFrag;
		ShaderResourcePtr m_slightFrag;
		ShaderProgramPtr m_plightProg;
		ShaderProgramPtr m_slightProg;

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

		ShaderResourcePtr m_frag;
		ShaderProgramPtr m_prog;
	} m_irradiance;

	DynamicArray<CacheEntry> m_cacheEntries;

	// Other
	TextureResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	// Init
	ANKI_USE_RESULT Error initInternal(const ConfigSet& cfg);
	ANKI_USE_RESULT Error initIs();
	ANKI_USE_RESULT Error initIrradiance();
	void initFaceInfo(U cacheEntryIdx, U faceIdx);
	ANKI_USE_RESULT Error loadMesh(CString fname, BufferPtr& vert, BufferPtr& idx, U32& idxCount);

	// Rendering
	ANKI_USE_RESULT Error tryRender(RenderingContext& ctx, SceneNode& node, U& probesRendered);

	ANKI_USE_RESULT Error runMs(RenderingContext& rctx, FrustumComponent& frc, U layer, U faceIdx);
	void runIs(RenderingContext& rctx, FrustumComponent& frc, U layer, U faceIdx);
	void computeIrradiance(RenderingContext& rctx, U layer, U faceIdx);

	ANKI_USE_RESULT Error renderReflection(RenderingContext& ctx, SceneNode& node, U cubemapIdx);

	/// Find a cache entry to store the reflection.
	void findCacheEntry(SceneNode& node, U& entry, Bool& render);
};
/// @}

} // end namespace anki
