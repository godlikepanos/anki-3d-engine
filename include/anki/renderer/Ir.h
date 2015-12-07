// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Renderer.h>
#include <anki/renderer/RenderingPass.h>
#include <anki/renderer/Clusterer.h>

namespace anki {

// Forward
struct IrShaderReflectionProbe;
class IrRunContext;
class IrTaskContext;
class ReflectionProbeComponent;

/// @addtogroup renderer
/// @{

/// Image based reflections.
class Ir: public RenderingPass
{
	friend class IrTask;

anki_internal:
	Ir(Renderer* r);

	~Ir();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);

	ANKI_USE_RESULT Error run(CommandBufferPtr cmdb);

	TexturePtr getCubemapArray() const
	{
		return m_cubemapArr;
	}

	DynamicBufferToken getProbesToken() const
	{
		return m_probesToken;
	}

	DynamicBufferToken getProbeIndicesToken() const
	{
		return m_indicesToken;
	}

	DynamicBufferToken getClustersToken() const
	{
		return m_clustersToken;
	}

	U getCubemapArrayMipmapCount() const
	{
		return m_cubemapArrMipCount;
	}

private:
	class CacheEntry
	{
	public:
		const SceneNode* m_node = nullptr;
		Timestamp m_timestamp = 0; ///< When last rendered.
	};

	Renderer m_nestedR;
	TexturePtr m_cubemapArr;
	U16 m_cubemapArrMipCount = 0;
	U16 m_cubemapArrSize = 0;
	U16 m_fbSize = 0;
	DArray<CacheEntry> m_cacheEntries;
	Barrier m_barrier;
	Clusterer m_clusterer;

	// Tokens
	DynamicBufferToken m_probesToken;
	DynamicBufferToken m_clustersToken;
	DynamicBufferToken m_indicesToken;

	/// Bin probes in clusters.
	void binProbes(U32 threadId, PtrSize threadsCount, IrRunContext& ctx);

	ANKI_USE_RESULT Error writeProbeAndRender(SceneNode& node,
		IrShaderReflectionProbe& probe);

	void binProbe(U probeIdx, IrRunContext& ctx, IrTaskContext& task) const;

	ANKI_USE_RESULT Error renderReflection(SceneNode& node,
		ReflectionProbeComponent& reflc, U cubemapIdx);

	static void writeIndicesAndCluster(U clusterIdx, Bool hasPrevCluster,
		IrRunContext& ctx);

	/// Find a cache entry to store the reflection.
	void findCacheEntry(SceneNode& node, U& entry, Bool& render);
};
/// @}

} // end namespace anki

