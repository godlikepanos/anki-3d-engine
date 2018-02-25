// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Clusterer.h>

namespace anki
{

// Forward
class LightBinContext;

/// @addtogroup renderer
/// @{

/// @memberof LightBin
class LightBinOut
{
public:
	StagingGpuMemoryToken m_pointLightsToken;
	StagingGpuMemoryToken m_spotLightsToken;
	StagingGpuMemoryToken m_probesToken;
	StagingGpuMemoryToken m_decalsToken;
	StagingGpuMemoryToken m_clustersToken;
	StagingGpuMemoryToken m_lightIndicesToken;

	TextureViewPtr m_diffDecalTexView;
	TextureViewPtr m_specularRoughnessDecalTexView;
};

/// Bins lights and probes to clusters.
class LightBin
{
	friend class WriteLightsTask;

public:
	LightBin(const GenericMemoryPoolAllocator<U8>& alloc,
		U clusterCountX,
		U clusterCountY,
		U clusterCountZ,
		ThreadPool* threadPool,
		StagingGpuMemoryManager* stagingMem);

	~LightBin();

	ANKI_USE_RESULT Error bin(const Mat4& viewMat,
		const Mat4& projMat,
		const Mat4& viewProjMat,
		const Mat4& camTrf,
		const RenderQueue& rqueue,
		StackAllocator<U8> frameAlloc,
		U maxLightIndices,
		Bool shadowsEnabled,
		LightBinOut& out);

	const Clusterer& getClusterer() const
	{
		return m_clusterer;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	Clusterer m_clusterer;
	U32 m_clusterCount = 0;
	ThreadPool* m_threadPool = nullptr;
	StagingGpuMemoryManager* m_stagingMem = nullptr;
	Barrier m_barrier;

	void binLights(U32 threadId, PtrSize threadsCount, LightBinContext& ctx);

	void writeAndBinPointLight(
		const PointLightQueueElement& lightEl, LightBinContext& ctx, ClustererTestResult& testResult);

	void writeAndBinSpotLight(
		const SpotLightQueueElement& lightEl, LightBinContext& ctx, ClustererTestResult& testResult);

	void writeAndBinProbe(
		const ReflectionProbeQueueElement& probe, LightBinContext& ctx, ClustererTestResult& testResult);

	void writeAndBinDecal(const DecalQueueElement& decal, LightBinContext& ctx, ClustererTestResult& testResult);
};
/// @}

} // end namespace anki
