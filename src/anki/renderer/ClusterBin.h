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
class Config;

/// @addtogroup renderer
/// @{

/// @memberof ClusterBin
class ClusterBinIn
{
public:
	ThreadHive* m_threadHive ANKI_DBG_NULLIFY;
	StackAllocator<U8> m_tempAlloc;

	const RenderQueue* m_renderQueue ANKI_DBG_NULLIFY;

	StagingGpuMemoryManager* m_stagingMem ANKI_DBG_NULLIFY;

	Bool m_shadowsEnabled ANKI_DBG_NULLIFY;
};

/// @memberof ClusterBin
class ClusterBinOut
{
public:
	StagingGpuMemoryToken m_pointLightsToken;
	StagingGpuMemoryToken m_spotLightsToken;
	StagingGpuMemoryToken m_probesToken;
	StagingGpuMemoryToken m_decalsToken;
	StagingGpuMemoryToken m_fogDensityVolumesToken;
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
	~ClusterBin();

	void init(HeapAllocator<U8> alloc, U32 clusterCountX, U32 clusterCountY, U32 clusterCountZ, const ConfigSet& cfg);

	void bin(ClusterBinIn& in, ClusterBinOut& out);

private:
	class BinCtx;
	class TileCtx;

	HeapAllocator<U8> m_alloc;

	Array<U32, 3> m_clusterCounts = {};
	U32 m_totalClusterCount = 0;
	U32 m_indexCount = 0;
	U32 m_avgObjectsPerCluster = 0;

	DynamicArray<Vec4> m_clusterEdges; ///< Cache those for opt. [tileCount][K+1][4]
	Vec4 m_prevUnprojParams = Vec4(0.0f); ///< To check if m_tiles is dirty.

	void prepare(BinCtx& ctx);

	void binTile(U32 tileIdx, BinCtx& ctx, TileCtx& tileCtx);

	void writeTypedObjectsToGpuBuffers(BinCtx& ctx) const;
};
/// @}

} // end namespace anki