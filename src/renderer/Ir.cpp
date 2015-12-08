// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Ir.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Pps.h>
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

struct IrShaderReflectionProbe
{
	Vec3 m_pos;
	F32 m_radiusSq;
	F32 m_cubemapIndex;
	U32 _m_pading[3];
};

struct IrShaderCluster
{
	U32 m_indexOffset;
	U32 m_probeCount;
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
	Atomic<U32> m_probeCount = {0};
	Array<ClusterDataIndex, MAX_PROBES_PER_CLUSTER> m_probeIds;

	Bool operator==(const IrClusterData& b) const
	{
		const U probeCount = m_probeCount.load() % MAX_PROBES_PER_CLUSTER;
		const U bProbeCount = b.m_probeCount.load() % MAX_PROBES_PER_CLUSTER;

		if(probeCount != bProbeCount)
		{
			return false;
		}

		if(probeCount > 0)
		{
			if(memcmp(&m_probeIds[0], &b.m_probeIds[0],
				sizeof(m_probeIds[0]) * probeCount) != 0)
			{
				return false;
			}
		}

		return true;
	}

	/// Sort the indices from the smallest probe to the biggest.
	void sort()
	{
		const U probeCount = m_probeCount.load() % MAX_PROBES_PER_CLUSTER;
		if(probeCount > 1)
		{
			std::sort(m_probeIds.getBegin(),
				m_probeIds.getBegin() + probeCount,
				[](const ClusterDataIndex& a, const ClusterDataIndex& b)
			{
				ANKI_ASSERT(a.m_probeRadius > 0.0 && b.m_probeRadius > 0.0);
				return a.m_probeRadius < b.m_probeRadius;
			});
		}
	}
};

/// Context for the whole run.
class IrRunContext
{
public:
	Ir* m_ir ANKI_DBG_NULLIFY_PTR;

	DArray<IrClusterData> m_clusterData;
	SArray<IrShaderCluster> m_clusters;
	SArray<U32> m_indices;
	Atomic<U32> m_indexCount = {0};
	VisibilityTestResults* m_visRez ANKI_DBG_NULLIFY_PTR;

	/// An atomic that will help allocating the index buffer
	Atomic<U32> m_probeIndicesAllocate = {0};
	/// Same as m_probeIndicesAllocate
	Atomic<U32> m_clustersAllocate = {0};

	StackAllocator<U8> m_alloc;

	~IrRunContext()
	{
		// Deallocate. Watch the order
		m_clusterData.destroy(m_alloc);
	}
};

/// Thread specific context.
class IrTaskContext
{
public:
	ClustererTestResult m_clustererTestResult;
	SceneNode* m_node ANKI_DBG_NULLIFY_PTR;
};

/// Write the lights to the GPU buffers.
class IrTask: public ThreadPool::Task
{
public:
	IrRunContext* m_ctx ANKI_DBG_NULLIFY_PTR;

	Error operator()(U32 threadId, PtrSize threadsCount) override
	{
		m_ctx->m_ir->binProbes(threadId, threadsCount, *m_ctx);
		return ErrorCode::NONE;
	}
};

//==============================================================================
// Ir                                                                          =
//==============================================================================

//==============================================================================
Ir::Ir(Renderer* r)
	: RenderingPass(r)
	, m_barrier(r->getThreadPool().getThreadsCount())
{}

//==============================================================================
Ir::~Ir()
{
	m_cacheEntries.destroy(getAllocator());
}

//==============================================================================
Error Ir::init(const ConfigSet& config)
{
	ANKI_LOGI("Initializing IR (Image Reflections)");
	m_fbSize = config.getNumber("ir.rendererSize");

	if(m_fbSize < Renderer::TILE_SIZE)
	{
		ANKI_LOGE("Too low ir.rendererSize");
		return ErrorCode::USER_DATA;
	}

	m_cubemapArrSize = config.getNumber("ir.cubemapTextureArraySize");

	if(m_cubemapArrSize < 2)
	{
		ANKI_LOGE("Too low ir.cubemapTextureArraySize");
		return ErrorCode::USER_DATA;
	}

	m_cacheEntries.create(getAllocator(), m_cubemapArrSize);

	// Init the renderer
	Config nestedRConfig;
	nestedRConfig.set("dbg.enabled", false);
	nestedRConfig.set("is.sm.bilinearEnabled", true);
	nestedRConfig.set("is.groundLightEnabled", false);
	nestedRConfig.set("is.sm.enabled", false);
	nestedRConfig.set("is.sm.maxLights", 8);
	nestedRConfig.set("is.sm.poissonEnabled", false);
	nestedRConfig.set("is.sm.resolution", 16);
	nestedRConfig.set("lf.maxFlares", 8);
	nestedRConfig.set("pps.enabled", true);
	nestedRConfig.set("pps.bloom.enabled", true);
	nestedRConfig.set("pps.ssao.enabled", false);
	nestedRConfig.set("renderingQuality", 1.0);
	nestedRConfig.set("clusterSizeZ", 4);
	nestedRConfig.set("width", m_fbSize);
	nestedRConfig.set("height", m_fbSize);
	nestedRConfig.set("lodDistance", 10.0);
	nestedRConfig.set("samples", 1);
	nestedRConfig.set("ir.enabled", false); // Very important to disable that
	nestedRConfig.set("sslr.enabled", false); // And that

	ANKI_CHECK(m_nestedR.init(&m_r->getThreadPool(),
		&m_r->getResourceManager(), &m_r->getGrManager(), m_r->getAllocator(),
		m_r->getFrameAllocator(), nestedRConfig, m_r->getGlobalTimestampPtr()));
	m_nestedR.getPps().setFog(Vec3(1.0, 0.0, 1.0), 0.0);

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

	// Init the clusterer
	F32 reflQuality = config.getNumber("refl.renderingQuality");
	U width =
		getAlignedRoundUp(Renderer::TILE_SIZE, m_r->getWidth() * reflQuality);
	U height =
		getAlignedRoundUp(Renderer::TILE_SIZE, m_r->getHeight() * reflQuality);

	U tileCountX = width / Renderer::TILE_SIZE;
	U tileCountY = height / Renderer::TILE_SIZE;
	U clusterCountZ = config.getNumber("ir.clusterSizeZ");

	m_clusterer.init(getAllocator(), tileCountX, tileCountY, clusterCountZ);

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

	//
	// Prepare clusterer
	//
	m_clusterer.prepare(m_r->getThreadPool(), frc);

	//
	// Perform some initialization
	//
	IrRunContext ctx;

	ctx.m_visRez = &visRez;
	ctx.m_ir = this;
	ctx.m_alloc = getFrameAllocator();

	// Allocate temp CPU mem
	ctx.m_clusterData.create(getFrameAllocator(),
		m_clusterer.getClusterCount());

	//
	// Render and populate probes GPU mem
	//

	// Probes GPU mem
	void* data = getGrManager().allocateFrameHostVisibleMemory(
		sizeof(IrShaderReflectionProbe) * visRez.getReflectionProbeCount()
		+ sizeof(Mat3x4) + sizeof(Vec4),
		BufferUsage::STORAGE, m_probesToken);

	Mat3x4* invViewRotation = static_cast<Mat3x4*>(data);
	*invViewRotation =
		Mat3x4(frc.getViewMatrix().getInverse().getRotationPart());

	Vec4* nearClusterDivisor = reinterpret_cast<Vec4*>(invViewRotation + 1);
	nearClusterDivisor->x() = frc.getFrustum().getNear();
	nearClusterDivisor->y() = m_clusterer.getShaderMagicValue();
	nearClusterDivisor->z() = 0.0;
	nearClusterDivisor->w() = 0.0;

	SArray<IrShaderReflectionProbe> probes(
		reinterpret_cast<IrShaderReflectionProbe*>(nearClusterDivisor + 1),
		visRez.getReflectionProbeCount());

	// Render some of the probes
	const VisibleNode* it = visRez.getReflectionProbesBegin();
	const VisibleNode* end = visRez.getReflectionProbesEnd();

	U probeIdx = 0;
	while(it != end)
	{
		// Write and render probe
		ANKI_CHECK(
			writeProbeAndRender(*it->m_node, probes[probeIdx]));

		++it;
		++probeIdx;
	}
	ANKI_ASSERT(probeIdx == visRez.getReflectionProbeCount());

	//
	// Start the jobs that can run in parallel
	//
	ThreadPool& threadPool = m_r->getThreadPool();
	Array<IrTask, ThreadPool::MAX_THREADS> tasks;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		tasks[i].m_ctx = &ctx;
		threadPool.assignNewTask(i, &tasks[i]);
	}

	// Sync
	ANKI_CHECK(threadPool.waitForAllThreadsToFinish());

	// Bye
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
	return ErrorCode::NONE;
}

//==============================================================================
void Ir::binProbes(U32 threadId, PtrSize threadsCount, IrRunContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDER_IR);
	IrTaskContext task;

	//
	// Bin the probes
	//

	PtrSize start, end;
	ThreadPool::Task::choseStartEnd(threadId, threadsCount,
		ctx.m_visRez->getReflectionProbeCount(), start, end);

	// Init clusterer test result for this thread
	if(start < end)
	{
		m_clusterer.initTestResults(getFrameAllocator(),
			task.m_clustererTestResult);
	}

	for(auto i = start; i < end; i++)
	{
		VisibleNode* vnode = ctx.m_visRez->getReflectionProbesBegin() + i;
		SceneNode& node = *vnode->m_node;

		task.m_node = &node;

		// Bin it to temp clusters
		binProbe(i, ctx, task);
	}

	//
	// Write the clusters
	//

	// Allocate the cluster buffer. First come first served
	U who = ctx.m_clustersAllocate.fetchAdd(1);
	if(who == 0)
	{
		void* mem = getGrManager().allocateFrameHostVisibleMemory(
			m_clusterer.getClusterCount() * sizeof(IrShaderCluster),
			BufferUsage::STORAGE, m_clustersToken);

		ctx.m_clusters = SArray<IrShaderCluster>(
			static_cast<IrShaderCluster*>(mem), m_clusterer.getClusterCount());
	}

	// Use the same trick to allocate the indices
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
	m_barrier.wait();
	ANKI_TRACE_START_EVENT(RENDER_IR);

	who = ctx.m_probeIndicesAllocate.fetchAdd(1);
	if(who == 0)
	{
		// Set it to zero in order to reuse it
		U indexCount = ctx.m_indexCount.exchange(0);
		if(indexCount > 0)
		{
			void* mem = getGrManager().allocateFrameHostVisibleMemory(
				indexCount * sizeof(U32), BufferUsage::STORAGE,
				m_indicesToken);

			ctx.m_indices = SArray<U32>(static_cast<U32*>(mem), indexCount);
		}
		else
		{
			m_indicesToken.markUnused();
		}
	}

	// Sync
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
	m_barrier.wait();
	ANKI_TRACE_START_EVENT(RENDER_IR);

	ThreadPool::Task::choseStartEnd(threadId, threadsCount,
		m_clusterer.getClusterCount(), start, end);

	for(auto i = start; i < end; i++)
	{
		Bool hasPrevCluster = (i != start);
		writeIndicesAndCluster(i, hasPrevCluster, ctx);
	}
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
}

//==============================================================================
Error Ir::writeProbeAndRender(SceneNode& node, IrShaderReflectionProbe& probe)
{
	const FrustumComponent& frc = m_r->getActiveFrustumComponent();
	ReflectionProbeComponent& reflc =
		node.getComponent<ReflectionProbeComponent>();

	Bool render = false;
	U entry;
	findCacheEntry(node, entry, render);

	// Write shader var
	probe.m_pos = (frc.getViewMatrix() * reflc.getPosition().xyz1()).xyz();
	probe.m_radiusSq = reflc.getRadius() * reflc.getRadius();
	probe.m_cubemapIndex = entry;

	if(reflc.getMarkedForRendering())
	{
		reflc.setMarkedForRendering(false);
		ANKI_CHECK(renderReflection(node, reflc, entry));
	}

	// If you need to render it mark it for the next frame
	if(render)
	{
		reflc.setMarkedForRendering(true);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Ir::binProbe(U probeIdx, IrRunContext& ctx, IrTaskContext& task) const
{
	const SpatialComponent& sp = task.m_node->getComponent<SpatialComponent>();
	const ReflectionProbeComponent& reflc =
		task.m_node->getComponent<ReflectionProbeComponent>();

	// Perform the expensive tests
	m_clusterer.bin(sp.getSpatialCollisionShape(), sp.getAabb(),
		task.m_clustererTestResult);

	// Bin to the correct tiles
	auto it = task.m_clustererTestResult.getClustersBegin();
	auto end = task.m_clustererTestResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i = m_clusterer.getClusterCountX()
			* (z * m_clusterer.getClusterCountY() + y) + x;

		auto& cluster = ctx.m_clusterData[i];

		i = cluster.m_probeCount.fetchAdd(1) % MAX_PROBES_PER_CLUSTER;
		cluster.m_probeIds[i].m_index = probeIdx;
		cluster.m_probeIds[i].m_probeRadius = reflc.getRadius();
	}

	ctx.m_indexCount.fetchAdd(task.m_clustererTestResult.getClusterCount());
}

//==============================================================================
void Ir::writeIndicesAndCluster(U clusterIdx, Bool hasPrevCluster,
	IrRunContext& ctx)
{
	IrClusterData& cdata = ctx.m_clusterData[clusterIdx];
	IrShaderCluster& cluster = ctx.m_clusters[clusterIdx];

	const U probeCount = cdata.m_probeCount.load() % MAX_PROBES_PER_CLUSTER;
	if(probeCount > 0)
	{
		// Sort to satisfy the probe hierarchy
		cdata.sort();

		// Check if the cdata is the same for the previous
		if(hasPrevCluster && cdata == ctx.m_clusterData[clusterIdx - 1])
		{
			// Same data
			cluster = ctx.m_clusters[clusterIdx - 1];
		}
		else
		{
			// Have to store the indices
			U idx = ctx.m_indexCount.fetchAdd(probeCount);

			cluster.m_indexOffset = idx;
			cluster.m_probeCount = probeCount;
			for(U j = 0; j < probeCount; ++j)
			{
				ctx.m_indices[idx] = cdata.m_probeIds[j].m_index;
				++idx;
			}
		}
	}
	else
	{
		cluster.m_indexOffset = 0;
		cluster.m_probeCount = 0;
	}
}

//==============================================================================
Error Ir::renderReflection(SceneNode& node, ReflectionProbeComponent& reflc,
	U cubemapIdx)
{
	ANKI_TRACE_INC_COUNTER(RENDERER_REFLECTIONS, 1);

	// Render cubemap
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
			m_nestedR.getPps().getRt(), 0, 0, m_cubemapArr,
			6 * cubemapIdx + i, 0);

		// Gen mips
		cmdb[cmdb.getSize() - 1]->generateMipmaps(m_cubemapArr,
			6 * cubemapIdx + i);

		// Flush
		for(U j = 0; j < cmdb.getSize(); ++j)
		{
			cmdb[j]->flush();
		}
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

