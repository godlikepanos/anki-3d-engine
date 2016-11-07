// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Clusterer.h>

namespace anki
{

// Forward
class MoveComponent;
class LightBinContext;
class SpatialComponent;
class LightComponent;

/// @addtogroup renderer
/// @{

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
		GrManager* gr);

	~LightBin();

	ANKI_USE_RESULT Error bin(FrustumComponent& frc,
		StackAllocator<U8> frameAlloc,
		U maxLightIndices,
		Bool shadowsEnabled,
		TransientMemoryToken& pointLightsToken,
		TransientMemoryToken& spotLightsToken,
		TransientMemoryToken* probesToken,
		TransientMemoryToken& decalsToken,
		TransientMemoryToken& clustersToken,
		TransientMemoryToken& lightIndicesToken,
		TexturePtr& diffuseDecalTexAtlas,
		TexturePtr& normalRoughnessDecalTexAtlas);

	const Clusterer& getClusterer() const
	{
		return m_clusterer;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	Clusterer m_clusterer;
	U32 m_clusterCount = 0;
	ThreadPool* m_threadPool = nullptr;
	GrManager* m_gr = nullptr;
	Barrier m_barrier;

	void binLights(U32 threadId, PtrSize threadsCount, LightBinContext& ctx);

	I writePointLight(
		const LightComponent& light, const MoveComponent& move, const FrustumComponent& camfrc, LightBinContext& ctx);

	I writeSpotLight(const LightComponent& lightc,
		const MoveComponent& lightMove,
		const FrustumComponent* lightFrc,
		const MoveComponent& camMove,
		const FrustumComponent& camFrc,
		LightBinContext& ctx);

	void binLight(const SpatialComponent& sp,
		const LightComponent& lightc,
		U pos,
		U lightType,
		LightBinContext& ctx,
		ClustererTestResult& testResult) const;

	void writeAndBinProbe(
		const FrustumComponent& camFrc, const SceneNode& node, LightBinContext& ctx, ClustererTestResult& testResult);

	void writeAndBinDecal(
		const MoveComponent& camMovec, const SceneNode& node, LightBinContext& ctx, ClustererTestResult& testResult);
};
/// @}

} // end namespace anki
