// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(RenderingContext& ctx);

	U getCubemapArrayMipmapCount() const
	{
		return m_cubemapArrMipCount;
	}

	TexturePtr getIrradianceTexture() const
	{
		return m_irradianceCubemapArr;
	}

	TexturePtr getReflectionTexture() const
	{
		return m_envCubemapArr;
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
	class CacheEntry
	{
	public:
		const SceneNode* m_node = nullptr;
		Timestamp m_timestamp = 0; ///< When last rendered.
	};

	static const U IRRADIANCE_SIZE = 32;

	Renderer m_nestedR;
	TexturePtr m_envCubemapArr;
	U16 m_cubemapArrMipCount = 0;
	U16 m_cubemapArrSize = 0;
	U16 m_fbSize = 0;
	DArray<CacheEntry> m_cacheEntries;

	// Irradiance
	TexturePtr m_irradianceCubemapArr;
	ShaderResourcePtr m_computeIrradianceFrag;
	PipelinePtr m_computeIrradiancePpline;
	ResourceGroupPtr m_computeIrradianceResources;

	// Other
	TextureResourcePtr m_integrationLut;
	SamplerPtr m_integrationLutSampler;

	ANKI_USE_RESULT Error initIrradiance();

	/// Bin probes in clusters.
	void binProbes(U32 threadId, PtrSize threadsCount, IrRunContext& ctx);

	ANKI_USE_RESULT Error tryRender(RenderingContext& ctx, SceneNode& node);

	void binProbe(U probeIdx, IrRunContext& ctx, IrTaskContext& task) const;

	ANKI_USE_RESULT Error renderReflection(RenderingContext& ctx,
		SceneNode& node,
		ReflectionProbeComponent& reflc,
		U cubemapIdx);

	static void writeIndicesAndCluster(
		U clusterIdx, Bool hasPrevCluster, IrRunContext& ctx);

	/// Find a cache entry to store the reflection.
	void findCacheEntry(SceneNode& node, U& entry, Bool& render);
};
/// @}

} // end namespace anki
