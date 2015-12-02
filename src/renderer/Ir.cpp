// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Clusterer.h>
#include <anki/core/Config.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/core/Trace.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

struct ShaderReflectionProbe
{
	Vec3 m_pos;
	F32 m_radiusSq;
	F32 m_cubemapIndex;
	U32 _m_pading[3];
};

struct ShaderCluster
{
	/// If m_combo = 0xFFFFAAAA then FFFF is the probe index offset, AAAA the
	/// number of probes
	U32 m_combo;
};

static const U MAX_PROBES_PER_CLUSTER = 16;

/// Store the probe radius for sorting the indices.
class ClusterDataIndex
{
public:
	U32 m_index = 0;
	F32 m_probeRadius = 0.0;
};

class IrClusterData
{
public:
	U32 m_probeCount = 0;
	Array<ClusterDataIndex, MAX_PROBES_PER_CLUSTER> m_probeIds;

	Bool operator==(const IrClusterData& b) const
	{
		if(m_probeCount != b.m_probeCount)
		{
			return false;
		}

		if(m_probeCount > 0)
		{
			if(memcmp(&m_probeIds[0], &b.m_probeIds[0],
				sizeof(m_probeIds[0]) * m_probeCount) != 0)
			{
				return false;
			}
		}

		return true;
	}

	/// Sort the indices from the smallest probe to the biggest.
	void sort()
	{
		if(m_probeCount > 1)
		{
			std::sort(m_probeIds.getBegin(),
				m_probeIds.getBegin() + m_probeCount,
				[](const ClusterDataIndex& a, const ClusterDataIndex& b)
			{
				ANKI_ASSERT(a.m_probeRadius > 0.0 && b.m_probeRadius > 0.0);
				return a.m_probeRadius < b.m_probeRadius;
			});
		}
	}
};

class IrBuildContext
{
public:
	DArray<IrClusterData> m_clusterData;
	U32 m_indexCount = 0;
	ClustererTestResult m_clustererTestResult;
};

//==============================================================================
// Ir                                                                          =
//==============================================================================

//==============================================================================
Ir::~Ir()
{
	m_cacheEntries.destroy(getAllocator());
}

//==============================================================================
Error Ir::init(const ConfigSet& initializer)
{
	ANKI_LOGI("Initializing IR (Image Reflections)");
	m_fbSize = initializer.getNumber("ir.rendererSize");

	if(m_fbSize < Renderer::TILE_SIZE)
	{
		ANKI_LOGE("Too low ir.rendererSize");
		return ErrorCode::USER_DATA;
	}

	m_cubemapArrSize = initializer.getNumber("ir.cubemapTextureArraySize");

	if(m_cubemapArrSize < 2)
	{
		ANKI_LOGE("Too low ir.cubemapTextureArraySize");
		return ErrorCode::USER_DATA;
	}

	m_cacheEntries.create(getAllocator(), m_cubemapArrSize);

	// Init the renderer
	Config config;
	config.set("dbg.enabled", false);
	config.set("is.sm.bilinearEnabled", true);
	config.set("is.groundLightEnabled", false);
	config.set("is.sm.enabled", false);
	config.set("is.sm.maxLights", 8);
	config.set("is.sm.poissonEnabled", false);
	config.set("is.sm.resolution", 16);
	config.set("lf.maxFlares", 8);
	config.set("pps.enabled", true);
	config.set("pps.bloom.enabled", true);
	config.set("pps.ssao.enabled", false);
	config.set("pps.sslr.enabled", false);
	config.set("renderingQuality", 1.0);
	config.set("clusterSizeZ", 4); // XXX A bug if more. Fix it
	config.set("width", m_fbSize);
	config.set("height", m_fbSize);
	config.set("lodDistance", 10.0);
	config.set("samples", 1);
	config.set("ir.enabled", false); // Very important to disable that

	ANKI_CHECK(m_nestedR.init(&m_r->getThreadPool(),
		&m_r->getResourceManager(), &m_r->getGrManager(), m_r->getAllocator(),
		m_r->getFrameAllocator(), config, m_r->getGlobalTimestampPtr()));

	// Init the texture
	TextureInitializer texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_depth = m_cubemapArrSize;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_format = Pps::RT_PIXEL_FORMAT;
	texinit.m_mipmapsCount = MAX_U8;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	texinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	m_cubemapArr = getGrManager().newInstance<Texture>(texinit);

	m_cubemapArrMipCount = computeMaxMipmapCount(m_fbSize, m_fbSize);

	getGrManager().finish();
	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::run(CommandBufferPtr cmdb)
{
	ANKI_TRACE_START_EVENT(RENDER_IR);
	FrustumComponent& frc = m_r->getActiveFrustumComponent();
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	if(visRez.getReflectionProbeCount() > m_cubemapArrSize)
	{
		ANKI_LOGW("Increase the ir.cubemapTextureArraySize");
	}

	IrBuildContext ctx;

	//
	// Perform some allocations
	//

	// Allocate temp mem for clusters
	ctx.m_clusterData.create(getFrameAllocator(), m_r->getClusterCount());

	// Probes
	void* data = getGrManager().allocateFrameHostVisibleMemory(
		sizeof(ShaderReflectionProbe) * visRez.getReflectionProbeCount()
		+ sizeof(Mat3x4),
		BufferUsage::STORAGE, m_probesToken);

	Mat3x4* invViewRotation = static_cast<Mat3x4*>(data);
	*invViewRotation =
		Mat3x4(frc.getViewMatrix().getInverse().getRotationPart());

	SArray<ShaderReflectionProbe> probes(
		reinterpret_cast<ShaderReflectionProbe*>(invViewRotation + 1),
		visRez.getReflectionProbeCount());

	//
	// Render and bin the probes
	//
	const VisibleNode* it = visRez.getReflectionProbesBegin();
	const VisibleNode* end = visRez.getReflectionProbesEnd();

	m_r->getClusterer().initTestResults(getFrameAllocator(),
		ctx.m_clustererTestResult);

	U probeIdx = 0;
	while(it != end)
	{
		// Render the probe
		ANKI_CHECK(renderReflection(*it->m_node, probes[probeIdx]));

		// Bin the probe
		binProbe(*it->m_node, probeIdx, ctx);

		++it;
		++probeIdx;
	}
	ANKI_ASSERT(probeIdx == visRez.getReflectionProbeCount());

	//
	// Populate the index buffer and the clusters
	//
	populateIndexAndClusterBuffers(ctx);

	// Bye
	ctx.m_clusterData.destroy(getFrameAllocator());
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
	return ErrorCode::NONE;
}

//==============================================================================
void Ir::populateIndexAndClusterBuffers(IrBuildContext& ctx)
{
	// Allocate GPU mem for indices
	SArray<U32> indices;
	if(ctx.m_indexCount > 0)
	{
		void* mem = getGrManager().allocateFrameHostVisibleMemory(
			ctx.m_indexCount * sizeof(U32), BufferUsage::STORAGE,
			m_indicesToken);

		indices = SArray<U32>(static_cast<U32*>(mem), ctx.m_indexCount);
	}
	else
	{
		m_indicesToken.markUnused();
	}

	U indexCount = 0;

	// Allocate GPU mem for clusters
	void* mem = getGrManager().allocateFrameHostVisibleMemory(
		m_r->getClusterCount() * sizeof(ShaderCluster), BufferUsage::STORAGE,
		m_clustersToken);
	SArray<ShaderCluster> clusters(static_cast<ShaderCluster*>(mem),
		m_r->getClusterCount());

	for(U i = 0; i < m_r->getClusterCount(); ++i)
	{
		IrClusterData& cdata = ctx.m_clusterData[i];
		ShaderCluster& cluster = clusters[i];

		if(cdata.m_probeCount > 0)
		{
			// Sort to satisfy the probe hierarchy
			cdata.sort();

			// Check if the cdata is the same for the previous
			if(i > 0 && cdata == ctx.m_clusterData[i - 1])
			{
				// Same data
				cluster.m_combo = clusters[i - 1].m_combo;
			}
			else
			{
				// Have to store the indices
				cluster.m_combo = (indexCount << 16) | cdata.m_probeCount;
				for(U j = 0; j < cdata.m_probeCount; ++j)
				{
					indices[indexCount] = cdata.m_probeIds[j].m_index;
					++indexCount;
				}
			}
		}
		else
		{
			cluster.m_combo = 0;
		}
	}
}

//==============================================================================
void Ir::binProbe(const SceneNode& node, U index, IrBuildContext& ctx)
{
	const SpatialComponent& sp = node.getComponent<SpatialComponent>();
	const ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	m_r->getClusterer().bin(sp.getSpatialCollisionShape(), sp.getAabb(),
		ctx.m_clustererTestResult);

	// Bin to the correct tiles
	auto it = ctx.m_clustererTestResult.getClustersBegin();
	auto end = ctx.m_clustererTestResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i =
			m_r->getTileCountXY().x() * (z * m_r->getTileCountXY().y() + y) + x;

		auto& cluster = ctx.m_clusterData[i];

		i = cluster.m_probeCount % MAX_PROBES_PER_CLUSTER;
		++cluster.m_probeCount;
		cluster.m_probeIds[i].m_index = index;
		cluster.m_probeIds[i].m_probeRadius = reflc.getRadius();

		++ctx.m_indexCount;
	}
}

//==============================================================================
Error Ir::renderReflection(SceneNode& node, ShaderReflectionProbe& shaderProb)
{
	const FrustumComponent& frc = m_r->getActiveFrustumComponent();
	ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	// Get cache entry
	Bool render = false;
	U entry;
	findCacheEntry(node, entry, render);

	// Write shader var
	shaderProb.m_pos = (frc.getViewMatrix() * reflc.getPosition().xyz1()).xyz();
	shaderProb.m_radiusSq = reflc.getRadius() * reflc.getRadius();
	shaderProb.m_cubemapIndex = entry;

	// Render cubemap
	if(reflc.getMarkedForRendering())
	{
		for(U i = 0; i < 6; ++i)
		{
			Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT> cmdb;
			for(U j = 0; j < cmdb.getSize(); ++j)
			{
				cmdb[j] = getGrManager().newInstance<CommandBuffer>();
			}

			// Render
			ANKI_CHECK(m_nestedR.render(node, i, cmdb));

			// Copy textures
			cmdb[cmdb.getSize() - 1]->copyTextureToTexture(
				m_nestedR.getPps().getRt(), 0, 0, m_cubemapArr, 6 * entry + i,
				0);

			// Gen mips
			cmdb[cmdb.getSize() - 1]->generateMipmaps(m_cubemapArr,
				6 * entry + i);

			// Flush
			for(U j = 0; j < cmdb.getSize(); ++j)
			{
				cmdb[j]->flush();
			}
		}

		reflc.setMarkedForRendering(false);
	}

	// If you need to render it mark it for the next frame
	if(render)
	{
		reflc.setMarkedForRendering(true);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Ir::findCacheEntry(SceneNode& node, U& entry, Bool& render)
{
	CacheEntry* it = m_cacheEntries.getBegin();
	const CacheEntry* const end = m_cacheEntries.getEnd();

	CacheEntry* canditate = nullptr;
	CacheEntry* empty = nullptr;
	CacheEntry* kick = nullptr;
	Timestamp kickTime = MAX_TIMESTAMP;

	while(it != end)
	{
		if(it->m_node == &node)
		{
			// Already there
			canditate = it;
			break;
		}
		else if(empty == nullptr && it->m_node == nullptr)
		{
			// Found empty
			empty = it;
		}
		else if(it->m_timestamp < kickTime)
		{
			// Found one to kick
			kick = it;
			kickTime = it->m_timestamp;
		}

		++it;
	}

	if(canditate)
	{
		// Update timestamp
		canditate->m_timestamp = getGlobalTimestamp();
		it = canditate;
		render = false;
	}
	else if(empty)
	{
		ANKI_ASSERT(empty->m_node == nullptr);
		empty->m_node = &node;
		empty->m_timestamp = getGlobalTimestamp();

		it = empty;
		render = true;
	}
	else if(kick)
	{
		kick->m_node = &node;
		kick->m_timestamp = getGlobalTimestamp();

		it = kick;
		render = true;
	}
	else
	{
		ANKI_ASSERT(0);
	}

	entry = it - m_cacheEntries.getBegin();
}

} // end namespace anki

