// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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

namespace anki
{

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
			if(memcmp(&m_probeIds[0],
				   &b.m_probeIds[0],
				   sizeof(m_probeIds[0]) * probeCount)
				!= 0)
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
				[](const ClusterDataIndex& a, const ClusterDataIndex& b) {
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
class IrTask : public ThreadPool::Task
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
{
}

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

	if(m_fbSize < TILE_SIZE)
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
	nestedRConfig.set("is.groundLightEnabled", false);
	nestedRConfig.set("sm.enabled", false);
	nestedRConfig.set("lf.maxFlares", 8);
	nestedRConfig.set("pps.enabled", false);
	nestedRConfig.set("renderingQuality", 1.0);
	nestedRConfig.set("clusterSizeZ", 16);
	nestedRConfig.set("width", m_fbSize);
	nestedRConfig.set("height", m_fbSize);
	nestedRConfig.set("lodDistance", 10.0);
	nestedRConfig.set("samples", 1);
	nestedRConfig.set("ir.enabled", false); // Very important to disable that
	nestedRConfig.set("sslr.enabled", false); // And that

	ANKI_CHECK(m_nestedR.init(&m_r->getThreadPool(),
		&m_r->getResourceManager(),
		&m_r->getGrManager(),
		m_r->getAllocator(),
		m_r->getFrameAllocator(),
		nestedRConfig,
		m_r->getGlobalTimestampPtr()));

	// Init the textures
	TextureInitInfo texinit;

	texinit.m_width = m_fbSize;
	texinit.m_height = m_fbSize;
	texinit.m_depth = m_cubemapArrSize;
	texinit.m_type = TextureType::CUBE_ARRAY;
	texinit.m_format = Is::RT_PIXEL_FORMAT;
	texinit.m_mipmapsCount = MAX_U8;
	texinit.m_samples = 1;
	texinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	texinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	m_envCubemapArr = getGrManager().newInstance<Texture>(texinit);

	texinit.m_width = IRRADIANCE_SIZE;
	texinit.m_height = IRRADIANCE_SIZE;
	m_irradianceCubemapArr = getGrManager().newInstance<Texture>(texinit);

	m_cubemapArrMipCount = computeMaxMipmapCount(m_fbSize, m_fbSize);

	// Create irradiance stuff
	ANKI_CHECK(initIrradiance());

	// Load split sum integration LUT
	ANKI_CHECK(getResourceManager().loadResource(
		"engine_data/SplitSumIntegration.ankitex", m_integrationLut));

	SamplerInitInfo sinit;
	sinit.m_minMagFilter = SamplingFilter::LINEAR;
	sinit.m_mipmapFilter = SamplingFilter::BASE;
	sinit.m_minLod = 0.0;
	sinit.m_maxLod = 1.0;
	sinit.m_repeat = false;
	m_integrationLutSampler = getGrManager().newInstance<Sampler>(sinit);

	// Init the resource group
	ResourceGroupInitInfo rcInit;
	rcInit.m_textures[0].m_texture = m_envCubemapArr;
	rcInit.m_textures[1].m_texture = m_irradianceCubemapArr;

	rcInit.m_textures[2].m_texture = m_integrationLut->getGrTexture();
	rcInit.m_textures[2].m_sampler = m_integrationLutSampler;

	rcInit.m_storageBuffers[0].m_dynamic = true;
	rcInit.m_storageBuffers[1].m_dynamic = true;
	rcInit.m_storageBuffers[2].m_dynamic = true;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::initIrradiance()
{
	// Create the shader
	StringAuto pps(getFrameAllocator());
	pps.sprintf("#define CUBEMAP_SIZE %u\n", IRRADIANCE_SIZE);

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_computeIrradianceFrag,
		"shaders/Irradiance.frag.glsl",
		pps.toCString(),
		"r_ir_"));

	// Create the ppline
	ColorStateInfo colorInf;
	colorInf.m_attachmentCount = 1;
	colorInf.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;

	m_r->createDrawQuadPipeline(m_computeIrradianceFrag->getGrShader(),
		colorInf,
		m_computeIrradiancePpline);

	// Create the resources
	ResourceGroupInitInfo rcInit;
	rcInit.m_uniformBuffers[0].m_dynamic = true;
	rcInit.m_textures[0].m_texture = m_envCubemapArr;

	m_computeIrradianceResources =
		getGrManager().newInstance<ResourceGroup>(rcInit);

	return ErrorCode::NONE;
}

//==============================================================================
Error Ir::run(RenderingContext& rctx)
{
	ANKI_TRACE_START_EVENT(RENDER_IR);
	FrustumComponent& frc = *rctx.m_frustumComponent;
	VisibilityTestResults& visRez = frc.getVisibilityTestResults();

	if(visRez.getCount(VisibilityGroupType::REFLECTION_PROBES)
		> m_cubemapArrSize)
	{
		ANKI_LOGW("Increase the ir.cubemapTextureArraySize");
	}

	//
	// Perform some initialization
	//
	IrRunContext ctx;

	ctx.m_visRez = &visRez;
	ctx.m_ir = this;
	ctx.m_alloc = getFrameAllocator();

	// Allocate temp CPU mem
	ctx.m_clusterData.create(
		getFrameAllocator(), m_r->getClusterer().getClusterCount());

	//
	// Render and populate probes GPU mem
	//

	// Probes GPU mem
	void* data = getGrManager().allocateFrameHostVisibleMemory(
		sizeof(IrShaderReflectionProbe)
				* visRez.getCount(VisibilityGroupType::REFLECTION_PROBES)
			+ sizeof(Mat3x4)
			+ sizeof(Vec4),
		BufferUsage::STORAGE,
		m_probesToken);

	Mat3x4* invViewRotation = static_cast<Mat3x4*>(data);
	*invViewRotation =
		Mat3x4(frc.getViewMatrix().getInverse().getRotationPart());

	Vec4* nearClusterDivisor = reinterpret_cast<Vec4*>(invViewRotation + 1);
	nearClusterDivisor->x() = frc.getFrustum().getNear();
	nearClusterDivisor->y() = m_r->getClusterer().getShaderMagicValue();
	nearClusterDivisor->z() = 0.0;
	nearClusterDivisor->w() = 0.0;

	SArray<IrShaderReflectionProbe> probes(
		reinterpret_cast<IrShaderReflectionProbe*>(nearClusterDivisor + 1),
		visRez.getCount(VisibilityGroupType::REFLECTION_PROBES));

	// Render some of the probes
	const VisibleNode* it =
		visRez.getBegin(VisibilityGroupType::REFLECTION_PROBES);
	const VisibleNode* end =
		visRez.getEnd(VisibilityGroupType::REFLECTION_PROBES);

	U probeIdx = 0;
	while(it != end)
	{
		// Write and render probe
		ANKI_CHECK(writeProbeAndRender(rctx, *it->m_node, probes[probeIdx]));

		++it;
		++probeIdx;
	}
	ANKI_ASSERT(
		probeIdx == visRez.getCount(VisibilityGroupType::REFLECTION_PROBES));

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
	ThreadPool::Task::choseStartEnd(threadId,
		threadsCount,
		ctx.m_visRez->getCount(VisibilityGroupType::REFLECTION_PROBES),
		start,
		end);

	// Init clusterer test result for this thread
	if(start < end)
	{
		m_r->getClusterer().initTestResults(
			getFrameAllocator(), task.m_clustererTestResult);
	}

	for(auto i = start; i < end; i++)
	{
		VisibleNode* vnode =
			ctx.m_visRez->getBegin(VisibilityGroupType::REFLECTION_PROBES) + i;
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
			m_r->getClusterer().getClusterCount() * sizeof(IrShaderCluster),
			BufferUsage::STORAGE,
			m_clustersToken);

		ctx.m_clusters =
			SArray<IrShaderCluster>(static_cast<IrShaderCluster*>(mem),
				m_r->getClusterer().getClusterCount());
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
				indexCount * sizeof(U32), BufferUsage::STORAGE, m_indicesToken);

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

	ThreadPool::Task::choseStartEnd(threadId,
		threadsCount,
		m_r->getClusterer().getClusterCount(),
		start,
		end);

	for(auto i = start; i < end; i++)
	{
		Bool hasPrevCluster = (i != start);
		writeIndicesAndCluster(i, hasPrevCluster, ctx);
	}
	ANKI_TRACE_STOP_EVENT(RENDER_IR);
}

//==============================================================================
Error Ir::writeProbeAndRender(
	RenderingContext& ctx, SceneNode& node, IrShaderReflectionProbe& probe)
{
	const FrustumComponent& frc = *ctx.m_frustumComponent;
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
		ANKI_CHECK(renderReflection(ctx, node, reflc, entry));
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
	m_r->getClusterer().bin(sp.getSpatialCollisionShape(),
		sp.getAabb(),
		task.m_clustererTestResult);

	// Bin to the correct tiles
	auto it = task.m_clustererTestResult.getClustersBegin();
	auto end = task.m_clustererTestResult.getClustersEnd();
	for(; it != end; ++it)
	{
		U x = (*it)[0];
		U y = (*it)[1];
		U z = (*it)[2];

		U i = m_r->getClusterer().getClusterCountX()
				* (z * m_r->getClusterer().getClusterCountY() + y)
			+ x;

		auto& cluster = ctx.m_clusterData[i];

		i = cluster.m_probeCount.fetchAdd(1) % MAX_PROBES_PER_CLUSTER;
		cluster.m_probeIds[i].m_index = probeIdx;
		cluster.m_probeIds[i].m_probeRadius = reflc.getRadius();
	}

	ctx.m_indexCount.fetchAdd(task.m_clustererTestResult.getClusterCount());
}

//==============================================================================
void Ir::writeIndicesAndCluster(
	U clusterIdx, Bool hasPrevCluster, IrRunContext& ctx)
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
Error Ir::renderReflection(RenderingContext& ctx,
	SceneNode& node,
	ReflectionProbeComponent& reflc,
	U cubemapIdx)
{
	ANKI_TRACE_INC_COUNTER(RENDERER_REFLECTIONS, 1);
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Get the frustum components
	Array<FrustumComponent*, 6> frustumComponents;
	U count = 0;
	Error err = node.iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& frc) -> Error {
			frustumComponents[count++] = &frc;
			return ErrorCode::NONE;
		});
	(void)err;
	ANKI_ASSERT(count == 6);

	// Render cubemap
	for(U i = 0; i < 6; ++i)
	{
		// Render
		RenderingContext nestedCtx(ctx.m_tempAllocator);
		nestedCtx.m_frustumComponent = frustumComponents[i];
		nestedCtx.m_commandBuffer = cmdb;

		ANKI_CHECK(m_nestedR.render(nestedCtx));

		// Copy env texture
		cmdb->copyTextureToTexture(m_nestedR.getIs().getRt(),
			0,
			0,
			m_envCubemapArr,
			6 * cubemapIdx + i,
			0);

		// Gen mips of env tex
		cmdb->generateMipmaps(m_envCubemapArr, 6 * cubemapIdx + i);
	}

	// Compute irradiance
	cmdb->setViewport(0, 0, IRRADIANCE_SIZE, IRRADIANCE_SIZE);
	for(U i = 0; i < 6; ++i)
	{
		DynamicBufferInfo dinf;
		UVec4* faceIdxArrayIdx =
			static_cast<UVec4*>(getGrManager().allocateFrameHostVisibleMemory(
				sizeof(UVec4), BufferUsage::UNIFORM, dinf.m_uniformBuffers[0]));
		faceIdxArrayIdx->x() = i;
		faceIdxArrayIdx->y() = cubemapIdx;

		cmdb->bindResourceGroup(m_computeIrradianceResources, 0, &dinf);

		FramebufferInitInfo fbinit;
		fbinit.m_colorAttachmentsCount = 1;
		fbinit.m_colorAttachments[0].m_texture = m_irradianceCubemapArr;
		fbinit.m_colorAttachments[0].m_arrayIndex = cubemapIdx;
		fbinit.m_colorAttachments[0].m_faceIndex = i;
		fbinit.m_colorAttachments[0].m_format = Is::RT_PIXEL_FORMAT;
		fbinit.m_colorAttachments[0].m_loadOperation =
			AttachmentLoadOperation::DONT_CARE;
		FramebufferPtr fb = getGrManager().newInstance<Framebuffer>(fbinit);
		cmdb->bindFramebuffer(fb);

		cmdb->bindPipeline(m_computeIrradiancePpline);
		m_r->drawQuad(cmdb);
		cmdb->generateMipmaps(m_irradianceCubemapArr, 6 * cubemapIdx + i);
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
		canditate->m_timestamp = m_r->getFrameCount();
		it = canditate;
		render = false;
	}
	else if(empty)
	{
		ANKI_ASSERT(empty->m_node == nullptr);
		empty->m_node = &node;
		empty->m_timestamp = m_r->getFrameCount();

		it = empty;
		render = true;
	}
	else if(kick)
	{
		kick->m_node = &node;
		kick->m_timestamp = m_r->getFrameCount();

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
