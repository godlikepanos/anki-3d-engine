// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <shaders/glsl_cpp_common/ClusteredShading.h>

namespace anki
{

// Forward
class ThreadHiveSemaphore;

/// @addtogroup renderer
/// @{

/// @memberof ClusterBin
class ClusterBinIn
{
public:
	ThreadHive* m_threadHive ANKI_DBG_NULLIFY;

	const RenderQueue* m_renderQueue ANKI_DBG_NULLIFY;

	Bool m_shadowsEnabled ANKI_DBG_NULLIFY;

	U32 m_maxLightIndices ANKI_DBG_NULLIFY;

	StackAllocator<U8> m_frameAlloc;
	StagingGpuMemoryManager* m_stagingMem;
};

/// @memberof ClusterBin
class ClusterBinOut
{
public:
	StagingGpuMemoryToken m_pointLightsToken;
	StagingGpuMemoryToken m_spotLightsToken;
	StagingGpuMemoryToken m_probesToken;
	StagingGpuMemoryToken m_decalsToken;
	StagingGpuMemoryToken m_clustersToken;
	StagingGpuMemoryToken m_indicesToken;

	TextureViewPtr m_diffDecalTexView;
	TextureViewPtr m_specularRoughnessDecalTexView;

	ClustererMagicValues m_shaderMagicValues;
};

/// Bins lights, probes, decals etc to clusters.
class ClusterBin
{
public:
	ClusterBin(const GenericMemoryPoolAllocator<U8>& alloc, U32 clusterCountX, U32 clusterCountY, U32 clusterCountZ);

	~ClusterBin();

	void bin(ClusterBinIn& in, ClusterBinOut& out);

private:
	class BinCtx;

	GenericMemoryPoolAllocator<U8> m_alloc;

	Array<U32, 3> m_clusterCounts = {};
	U32 m_totalClusterCount = 0;

	void prepare(BinCtx& ctx);

	void processNextCluster(BinCtx& ctx) const;

	void writeTypedObjectsToGpuBuffers(BinCtx& ctx) const;

	static void writeTypedObjectsToGpuBuffersCallback(
		void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore);

	static void processNextClusterCallback(
		void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore);
};
/// @}

} // end namespace anki