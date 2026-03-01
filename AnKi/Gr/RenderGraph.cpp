// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/DynamicBitSet.h>
#include <AnKi/Core/Common.h>

namespace anki {

#define ANKI_DBG_RENDER_GRAPH 0

static inline U32 getTextureSurfOrVolCount(const TextureInternalPtr& tex)
{
	return tex->getMipmapCount() * tex->getLayerCount() * (textureTypeIsCube(tex->getTextureType()) ? 6 : 1);
}

template<typename T>
static void allocateButNotConstructArray(StackMemoryPool& pool, U32 count, WeakArray<T>& out)
{
	if(count)
	{
		void* mem = pool.allocate(sizeof(T) * count, alignof(T));
		out = {static_cast<T*>(mem), count};
	}
	else
	{
		out = {};
	}
}

void RenderPassBase::newTextureDependency(RenderTargetHandle handle, TextureUsageBit usage, const TextureSubresourceDesc& subresource)
{
	TextureDependency& newDep = *m_rtDeps.emplaceBack();
	newDep.m_handle = handle;
	newDep.m_subresource = subresource;
	newDep.m_usage = usage;

	// Sanitize a bit
	const RenderGraphBuilder::RT& rt = m_descr->m_renderTargets[newDep.m_handle.m_idx];
	if(newDep.m_subresource.m_depthStencilAspect == DepthStencilAspectBit::kNone)
	{
		if(rt.m_importedTex.isCreated() && !!rt.m_importedTex->getDepthStencilAspect())
		{
			newDep.m_subresource.m_depthStencilAspect = rt.m_importedTex->getDepthStencilAspect();
		}
		else if(!rt.m_importedTex.isCreated() && getFormatInfo(rt.m_initInfo.m_format).isDepthStencil())
		{
			newDep.m_subresource.m_depthStencilAspect = getFormatInfo(rt.m_initInfo.m_format).m_depthStencil;
		}
	}

	// Try to derive the usage by that dep
	m_descr->m_renderTargets[newDep.m_handle.m_idx].m_usageDerivedByDeps |= newDep.m_usage;

	// Checks
#if ANKI_ASSERTIONS_ENABLED
	if((!rt.m_importedTex.isCreated() && !!getFormatInfo(rt.m_initInfo.m_format).m_depthStencil)
	   || (rt.m_importedTex.isCreated() && !!rt.m_importedTex->getDepthStencilAspect()))
	{
		ANKI_ASSERT(!!m_rtDeps.getBack().m_subresource.m_depthStencilAspect
					&& "Dependencies of depth/stencil resources should have a valid DS aspect");
	}
#endif
}

void RenderPassBase::newBufferDependency(BufferHandle handle, BufferUsageBit usage)
{
	ANKI_ASSERT(!!(m_descr->m_buffers[handle.m_idx].m_importedBuff->getBufferUsage() & usage));

	BufferDependency& dep = *m_buffDeps.emplaceBack();
	dep.m_handle = handle;
	dep.m_usage = usage;
}

void RenderPassBase::newAccelerationStructureDependency(AccelerationStructureHandle handle, AccelerationStructureUsageBit usage)
{
	ASDependency& dep = *m_asDeps.emplaceBack();
	dep.m_handle = handle;
	dep.m_usage = usage;
}

void GraphicsRenderPass::setRenderpassInfo(ConstWeakArray<GraphicsRenderPassTargetDesc> colorRts, const GraphicsRenderPassTargetDesc* depthStencilRt,
										   const RenderTargetHandle* vrsRt, U8 vrsRtTexelSizeX, U8 vrsRtTexelSizeY)
{
	m_graphicsPassExtra->m_colorRtCount = U8(colorRts.getSize());
	for(U32 i = 0; i < m_graphicsPassExtra->m_colorRtCount; ++i)
	{
		m_graphicsPassExtra->m_rts[i].m_handle = colorRts[i].m_handle;
		ANKI_ASSERT(!colorRts[i].m_subresource.m_depthStencilAspect);
		m_graphicsPassExtra->m_rts[i].m_subresource = colorRts[i].m_subresource;
		m_graphicsPassExtra->m_rts[i].m_loadOperation = colorRts[i].m_loadOperation;
		m_graphicsPassExtra->m_rts[i].m_storeOperation = colorRts[i].m_storeOperation;
		m_graphicsPassExtra->m_rts[i].m_clearValue = colorRts[i].m_clearValue;
	}

	if(depthStencilRt)
	{
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_handle = depthStencilRt->m_handle;
		ANKI_ASSERT(!!depthStencilRt->m_subresource.m_depthStencilAspect);
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_subresource = depthStencilRt->m_subresource;
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_loadOperation = depthStencilRt->m_loadOperation;
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_storeOperation = depthStencilRt->m_storeOperation;
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_stencilLoadOperation = depthStencilRt->m_stencilLoadOperation;
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_stencilStoreOperation = depthStencilRt->m_stencilStoreOperation;
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_clearValue = depthStencilRt->m_clearValue;
	}

	if(vrsRt)
	{
		m_graphicsPassExtra->m_rts[kMaxColorRenderTargets + 1].m_handle = *vrsRt;
		m_graphicsPassExtra->m_vrsRtTexelSizeX = vrsRtTexelSizeX;
		m_graphicsPassExtra->m_vrsRtTexelSizeY = vrsRtTexelSizeY;
	}

	m_graphicsPassExtra->m_hasRenderpass = true;
}

RenderTargetHandle RenderGraphBuilder::importRenderTarget(Texture* tex, TextureUsageBit usage)
{
	for([[maybe_unused]] const RT& rt : m_renderTargets)
	{
		ANKI_ASSERT(rt.m_importedTex.tryGet() != tex && "Already imported");
	}

	RT& rt = *m_renderTargets.emplaceBack();
	rt.m_importedTex.reset(tex);
	rt.m_importedLastKnownUsage = usage;
	rt.m_usageDerivedByDeps = TextureUsageBit::kNone;
	rt.setName(tex->getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

RenderTargetHandle RenderGraphBuilder::importRenderTarget(Texture* tex)
{
	RenderTargetHandle out = importRenderTarget(tex, TextureUsageBit::kNone);
	m_renderTargets.getBack().m_importedAndUndefinedUsage = true;
	return out;
}

RenderTargetHandle RenderGraphBuilder::newRenderTarget(const RenderTargetDesc& initInf)
{
	ANKI_ASSERT(initInf.m_hash && "Forgot to call RenderTargetDesc::bake");
	ANKI_ASSERT(initInf.m_usage == TextureUsageBit::kNone && "Don't need to supply the usage. Render grap will find it");

	for([[maybe_unused]] auto it : m_renderTargets)
	{
		ANKI_ASSERT(it.m_hash != initInf.m_hash && "There is another RT descriptor with the same hash. Rendergraph's RT recycler will get confused");
	}

	RT& rt = *m_renderTargets.emplaceBack();
	rt.m_initInfo = initInf;
	rt.m_hash = initInf.m_hash;
	rt.m_importedLastKnownUsage = TextureUsageBit::kNone;
	rt.m_usageDerivedByDeps = TextureUsageBit::kNone;
	rt.setName(initInf.getName());

	RenderTargetHandle out;
	out.m_idx = m_renderTargets.getSize() - 1;
	return out;
}

BufferHandle RenderGraphBuilder::importBuffer(const BufferView& buff, BufferUsageBit crntUsage)
{
	ANKI_ASSERT(buff.isValid());

	for([[maybe_unused]] const BufferRsrc& bb : m_buffers)
	{
		ANKI_ASSERT(!buff.overlaps(BufferView(bb.m_importedBuff.get(), bb.m_offset, bb.m_range)) && "Range already imported");
	}

	BufferRsrc& b = *m_buffers.emplaceBack();
	b.setName(buff.getBuffer().getName());
	b.m_usage = crntUsage;
	b.m_importedBuff.reset(&buff.getBuffer());
	b.m_offset = buff.getOffset();
	b.m_range = buff.getRange();

	BufferHandle out;
	out.m_idx = m_buffers.getSize() - 1;
	return out;
}

AccelerationStructureHandle RenderGraphBuilder::importAccelerationStructure(AccelerationStructure* as, AccelerationStructureUsageBit crntUsage)
{
	for([[maybe_unused]] const AS& a : m_as)
	{
		ANKI_ASSERT(a.m_importedAs.get() != as && "Already imported");
	}

	AS& a = *m_as.emplaceBack();
	a.setName(as->getName());
	a.m_importedAs.reset(as);
	a.m_usage = crntUsage;

	AccelerationStructureHandle handle;
	handle.m_idx = m_as.getSize() - 1;
	return handle;
}

// Contains some extra things for render targets.
class RenderGraph::RT
{
public:
	WeakArray<TextureUsageBit> m_surfOrVolUsages;
	DynamicArray<U16, MemoryPoolPtrWrapper<StackMemoryPool>> m_lastBatchThatTransitionedIt;

	// Depedencies in SoA
	DynamicArray<U16, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependentPasses;
	DynamicArray<TextureUsageBit, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependencyUsages;
	DynamicArray<TextureSubresourceDesc, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependencySubresources;

	TextureInternalPtr m_texture; // Hold a reference.
	Bool m_imported;

	RT(StackMemoryPool* pool)
		: m_lastBatchThatTransitionedIt(pool)
		, m_dependentPasses(pool)
		, m_dependencyUsages(pool)
		, m_dependencySubresources(pool)
	{
	}
};

// Same as RT but for buffers.
class RenderGraph::BufferRange
{
public:
	BufferUsageBit m_usage;
	BufferInternalPtr m_buffer; // Hold a reference.
	PtrSize m_offset;
	PtrSize m_range;

	// Depedencies in SoA
	DynamicArray<U16, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependentPasses;
	DynamicArray<BufferUsageBit, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependencyUsages;

	BufferRange(StackMemoryPool* pool)
		: m_dependentPasses(pool)
		, m_dependencyUsages(pool)
	{
	}
};

class RenderGraph::AS
{
public:
	AccelerationStructureUsageBit m_usage;
	AccelerationStructurePtr m_as; // Hold a reference.

	// Depedencies in SoA
	DynamicArray<U16, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependentPasses;
	DynamicArray<AccelerationStructureUsageBit, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependencyUsages;

	AS(StackMemoryPool* pool)
		: m_dependentPasses(pool)
		, m_dependencyUsages(pool)
	{
	}
};

// Pipeline barrier.
class RenderGraph::TextureBarrier
{
public:
	U32 m_idx;
	TextureUsageBit m_usageBefore;
	TextureUsageBit m_usageAfter;
	TextureSubresourceDesc m_subresource;

	TextureBarrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSubresourceDesc& sub)
		: m_idx(rtIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
		, m_subresource(sub)
	{
	}
};

// Pipeline barrier.
class RenderGraph::BufferBarrier
{
public:
	U32 m_idx;
	BufferUsageBit m_usageBefore;
	BufferUsageBit m_usageAfter;

	BufferBarrier(U32 buffIdx, BufferUsageBit usageBefore, BufferUsageBit usageAfter)
		: m_idx(buffIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
	{
	}
};

// Pipeline barrier.
class RenderGraph::ASBarrier
{
public:
	U32 m_idx;
	AccelerationStructureUsageBit m_usageBefore;
	AccelerationStructureUsageBit m_usageAfter;

	ASBarrier(U32 asIdx, AccelerationStructureUsageBit usageBefore, AccelerationStructureUsageBit usageAfter)
		: m_idx(asIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
	{
	}
};

// Contains some extra things the RenderPassBase cannot hold.
class RenderGraph::Pass
{
public:
	// WARNING!!!!!: Whatever you put here needs manual destruction in RenderGraph::reset()

	DynamicBitSet<MemoryPoolPtrWrapper<StackMemoryPool>> m_dependsOnPassMask;

	WeakArray<RenderPassBase::TextureDependency> m_consumedTextures;

	Function<void(RenderPassWorkContext&), MemoryPoolPtrWrapper<StackMemoryPool>> m_callback;

	class
	{
	public:
		Array<RenderTarget, kMaxColorRenderTargets> m_colorRts;
		RenderTarget m_dsRt;
		TextureView m_vrsRt;
		U8 m_colorRtCount = 0;
		U8 m_vrsTexelSizeX = 0;
		U8 m_vrsTexelSizeY = 0;
		Bool m_hasRenderpass = false;

		Array<TextureInternalPtr, kMaxColorRenderTargets + 2> m_refs;
	} m_beginRenderpassInfo;

	BaseString<MemoryPoolPtrWrapper<StackMemoryPool>> m_name;

	U32 m_batchIdx ANKI_DEBUG_CODE(= kMaxU32);
	Bool m_writesToSwapchain = false;

	Pass(StackMemoryPool* pool)
		: m_dependsOnPassMask(pool)
		, m_name(pool)
	{
	}
};

// A batch of render passes. These passes can run in parallel.
// @warning It's POD. Destructor won't be called.
class RenderGraph::Batch
{
public:
	DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> m_passIndices;
	DynamicArray<TextureBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> m_textureBarriersBefore;
	DynamicArray<BufferBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> m_bufferBarriersBefore;
	DynamicArray<ASBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> m_asBarriersBefore;

	Batch(StackMemoryPool* pool)
		: m_passIndices(pool)
		, m_textureBarriersBefore(pool)
		, m_bufferBarriersBefore(pool)
		, m_asBarriersBefore(pool)
	{
	}

	Batch(Batch&& b)
	{
		*this = std::move(b);
	}

	Batch& operator=(Batch&& b)
	{
		m_passIndices = std::move(b.m_passIndices);
		m_textureBarriersBefore = std::move(b.m_textureBarriersBefore);
		m_bufferBarriersBefore = std::move(b.m_bufferBarriersBefore);
		m_asBarriersBefore = std::move(b.m_asBarriersBefore);

		return *this;
	}
};

// The RenderGraph build context.
class RenderGraph::BakeContext
{
public:
	WeakArray<Pass> m_passes;
	DynamicBitSet<MemoryPoolPtrWrapper<StackMemoryPool>, U64> m_passIsInBatch;
	DynamicArray<Batch, MemoryPoolPtrWrapper<StackMemoryPool>> m_batches;
	WeakArray<RT> m_rts;
	WeakArray<BufferRange> m_buffers;
	WeakArray<AS> m_as;

	Bool m_gatherStatistics = false;

	BakeContext(StackMemoryPool* pool)
		: m_passIsInBatch(pool)
		, m_batches(pool)
	{
	}
};

RenderGraph::RenderGraph(CString name)
	: GrObject(kClassType, name)
{
	const Array<PtrSize, 8> memoryClasses = {256_KB, 1_MB, 4_MB, 8_MB, 16_MB, 32_MB, 128_MB, 256_MB};
	m_texMemPool.init(memoryClasses.getBack(), 32, "RenderGraph memory", memoryClasses, BufferUsageBit::kTexture);
}

RenderGraph::~RenderGraph()
{
	ANKI_ASSERT(m_ctx == nullptr);
}

RenderGraph* RenderGraph::newInstance()
{
	return anki::newInstance<RenderGraph>(GrMemoryPool::getSingleton(), "N/A");
}

void RenderGraph::reset()
{
	ANKI_TRACE_SCOPED_EVENT(GrRenderGraphReset);

	if(!m_ctx)
	{
		return;
	}

	if((m_version % kPeriodicCleanupEvery) == 0)
	{
		// Do cleanup
		periodicCleanup();
	}

	// Extract the final usage of the imported RTs and clean all RTs
	for(RT& rt : m_ctx->m_rts)
	{
		if(rt.m_imported)
		{
			const U32 surfOrVolumeCount = getTextureSurfOrVolCount(rt.m_texture);

			// Create a new hash because our hash map dislikes concurent keys.
			const U64 uuid = rt.m_texture->getUuid();
			const U64 hash = computeHash(&uuid, sizeof(uuid));

			auto it = m_importedRenderTargets.find(hash);
			if(it != m_importedRenderTargets.getEnd())
			{
				// Found
				ANKI_ASSERT(it->m_surfOrVolLastUsages.getSize() == surfOrVolumeCount);
				ANKI_ASSERT(rt.m_surfOrVolUsages.getSize() == surfOrVolumeCount);
			}
			else
			{
				// Not found, create
				it = m_importedRenderTargets.emplace(hash);
				it->m_surfOrVolLastUsages.resize(surfOrVolumeCount);
			}

			// Update the usage
			for(U32 surfOrVolIdx = 0; surfOrVolIdx < surfOrVolumeCount; ++surfOrVolIdx)
			{
				it->m_surfOrVolLastUsages[surfOrVolIdx] = rt.m_surfOrVolUsages[surfOrVolIdx];
			}
		}

		rt.m_texture.reset(nullptr);
	}

	for(BufferRange& buff : m_ctx->m_buffers)
	{
		buff.m_buffer.reset(nullptr);
	}

	for(AS& as : m_ctx->m_as)
	{
		as.m_as.reset(nullptr);
	}

	for(auto& it : m_renderTargetCache)
	{
		it.m_texturesInUse = 0;
	}

	for(Pass& p : m_ctx->m_passes)
	{
		p.m_beginRenderpassInfo.m_refs.fill(TextureInternalPtr(nullptr));
		p.m_callback.destroy();
		p.m_name.destroy();
	}

	m_ctx = nullptr;
	++m_version;
}

TextureInternalPtr RenderGraph::getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash)
{
	ANKI_ASSERT(hash);

	// Find a cache entry
	RenderTargetCacheEntry* entry = nullptr;
	auto it = m_renderTargetCache.find(hash);
	if(it == m_renderTargetCache.getEnd()) [[unlikely]]
	{
		// Didn't found the entry, create a new one

		auto it2 = m_renderTargetCache.emplace(hash);
		entry = &(*it2);
	}
	else
	{
		entry = &(*it);
	}
	ANKI_ASSERT(entry);

	// Create or pop one tex from the cache
	TextureInternalPtr tex;
	const Bool createNewTex = entry->m_textures.getSize() == entry->m_texturesInUse;
	if(!createNewTex)
	{
		// Pop

		tex = entry->m_textures[entry->m_texturesInUse++];
	}
	else
	{
		// Create it

		const PtrSize texSize = GrManager::getSingleton().getTextureMemoryRequirement(initInf);
		SegregatedListsGpuMemoryPoolAllocation alloc = m_texMemPool.allocate(texSize, 1);
		TextureInitInfo newTexInit = initInf;
		newTexInit.m_memoryBuffer = alloc;

		tex = GrManager::getSingleton().newTexture(newTexInit);

		ANKI_ASSERT(entry->m_texturesInUse == entry->m_textures.getSize());
		entry->m_textures.emplaceBack(tex);
		entry->m_texAllocations.emplaceBack(std::move(alloc));
		++entry->m_texturesInUse;
	}

	return tex;
}

Bool RenderGraph::passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx)
{
	Bool depends = false;

	if(ctx.m_batches.getSize() > 0)
	{
		// Check if the deps of passIdx are all in a batch

		ctx.m_passes[passIdx].m_dependsOnPassMask.iterateSetBitsFromLeastSignificant([&](U32 depPassIdx) {
			if(!ctx.m_passIsInBatch.getBit(depPassIdx))
			{
				// Dependency pass is not in a batch
				depends = true;
				return FunctorContinue::kStop;
			}

			return FunctorContinue::kContinue;
		});
	}
	else
	{
		// First batch, check if passIdx depends on any pass

		depends = ctx.m_passes[passIdx].m_dependsOnPassMask.getSetBitCount() != 0;
	}

	return depends;
}

RenderGraph::BakeContext* RenderGraph::newContext(const RenderGraphBuilder& descr, StackMemoryPool& pool)
{
	ANKI_TRACE_FUNCTION();

	// Allocate
	BakeContext* ctx = anki::newInstance<BakeContext>(pool, &pool);

	// Init the resources
	allocateButNotConstructArray(pool, descr.m_renderTargets.getSize(), ctx->m_rts);
	for(U32 rtIdx = 0; rtIdx < descr.m_renderTargets.getSize(); ++rtIdx)
	{
		RT& outRt = ctx->m_rts[rtIdx];
		callConstructor(outRt, &pool);
		const RenderGraphBuilder::RT& inRt = descr.m_renderTargets[rtIdx];

		const Bool imported = inRt.m_importedTex.isCreated();
		if(imported)
		{
			// It's imported
			outRt.m_texture = inRt.m_importedTex;
		}
		else
		{
			// Need to create new

			// Create a new TextureInitInfo with the derived usage
			TextureInitInfo initInf = inRt.m_initInfo;
			initInf.m_usage = inRt.m_usageDerivedByDeps;
			ANKI_ASSERT(initInf.m_usage != TextureUsageBit::kNone && "Probably not referenced by any pass");

			// Create the new hash
			const U64 hash = appendHash(&initInf.m_usage, sizeof(initInf.m_usage), inRt.m_hash);

			// Get or create the texture
			outRt.m_texture = getOrCreateRenderTarget(initInf, hash);
		}

		// Init the usage
		const U32 surfOrVolumeCount = getTextureSurfOrVolCount(outRt.m_texture);
		outRt.m_surfOrVolUsages = {newArray<TextureUsageBit>(pool, surfOrVolumeCount, TextureUsageBit::kNone), surfOrVolumeCount};
		if(imported && inRt.m_importedAndUndefinedUsage)
		{
			// Get the usage from previous frames

			// Create a new hash because our hash map dislikes concurent keys.
			const U64 uuid = outRt.m_texture->getUuid();
			const U64 hash = computeHash(&uuid, sizeof(uuid));

			auto it = m_importedRenderTargets.find(hash);
			ANKI_ASSERT(it != m_importedRenderTargets.getEnd() && "Can't find the imported RT");

			ANKI_ASSERT(it->m_surfOrVolLastUsages.getSize() == surfOrVolumeCount);
			for(U32 surfOrVolIdx = 0; surfOrVolIdx < surfOrVolumeCount; ++surfOrVolIdx)
			{
				outRt.m_surfOrVolUsages[surfOrVolIdx] = it->m_surfOrVolLastUsages[surfOrVolIdx];
			}
		}
		else if(imported)
		{
			// Set the usage that was given by the user
			for(U32 surfOrVolIdx = 0; surfOrVolIdx < surfOrVolumeCount; ++surfOrVolIdx)
			{
				outRt.m_surfOrVolUsages[surfOrVolIdx] = inRt.m_importedLastKnownUsage;
			}
		}

		outRt.m_lastBatchThatTransitionedIt.resize(surfOrVolumeCount, kMaxU16);
		outRt.m_imported = imported;
	}

	// Buffers
	allocateButNotConstructArray(pool, descr.m_buffers.getSize(), ctx->m_buffers);
	for(U32 buffIdx = 0; buffIdx < ctx->m_buffers.getSize(); ++buffIdx)
	{
		callConstructor(ctx->m_buffers[buffIdx], &pool);
		ctx->m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
		ANKI_ASSERT(descr.m_buffers[buffIdx].m_importedBuff.isCreated());
		ctx->m_buffers[buffIdx].m_buffer = descr.m_buffers[buffIdx].m_importedBuff;
		ctx->m_buffers[buffIdx].m_offset = descr.m_buffers[buffIdx].m_offset;
		ctx->m_buffers[buffIdx].m_range = descr.m_buffers[buffIdx].m_range;
	}

	// AS
	allocateButNotConstructArray(pool, descr.m_as.getSize(), ctx->m_as);
	for(U32 i = 0; i < descr.m_as.getSize(); ++i)
	{
		callConstructor(ctx->m_as[i], &pool);
		ctx->m_as[i].m_usage = descr.m_as[i].m_usage;
		ctx->m_as[i].m_as = descr.m_as[i].m_importedAs;
		ANKI_ASSERT(ctx->m_as[i].m_as.isCreated());
	}

	ctx->m_gatherStatistics = descr.m_gatherStatistics;

	return ctx;
}

void RenderGraph::initRenderPassesAndSetDeps(const RenderGraphBuilder& descr)
{
	ANKI_TRACE_FUNCTION();

	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	StackMemoryPool* pool = ctx.m_batches.getMemoryPool().m_pool;

	allocateButNotConstructArray(*pool, passCount, ctx.m_passes);
	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassBase& inPass = descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];
		callConstructor(outPass, pool);

		outPass.m_callback = inPass.m_callback;
		outPass.m_name = inPass.m_name;
		outPass.m_writesToSwapchain = inPass.m_writesToSwapchain;

		// Populate a new view of dependencies
		allocateButNotConstructArray(*pool, inPass.m_rtDeps.getSize(), outPass.m_consumedTextures);
		U32 count = 0;
		for(const RenderPassBase::TextureDependency& dep : inPass.m_rtDeps)
		{
			RT& rt = ctx.m_rts[dep.m_handle.m_idx];
			rt.m_dependentPasses.emplaceBack(U16(passIdx));
			rt.m_dependencyUsages.emplaceBack(dep.m_usage);
			rt.m_dependencySubresources.emplaceBack(dep.m_subresource);

			callConstructor(outPass.m_consumedTextures[count++], dep);
		}

		for(const RenderPassBase::BufferDependency& dep : inPass.m_buffDeps)
		{
			BufferRange& buff = ctx.m_buffers[dep.m_handle.m_idx];
			buff.m_dependentPasses.emplaceBack(U16(passIdx));
			buff.m_dependencyUsages.emplaceBack(dep.m_usage);
		}

		for(const RenderPassBase::ASDependency& dep : inPass.m_asDeps)
		{
			AS& as = ctx.m_as[dep.m_handle.m_idx];
			as.m_dependentPasses.emplaceBack(U16(passIdx));
			as.m_dependencyUsages.emplaceBack(dep.m_usage);
		}
	}

	// Find depedent passes using textures
	for(const RT& rt : ctx.m_rts)
	{
		const U32 dependencyCount = rt.m_dependencyUsages.getSize();
		for(U32 i = 1; i < dependencyCount; ++i)
		{
			const TextureUsageBit crntRtUsage = rt.m_dependencyUsages[i];
			const TextureSubresourceDesc& crntSubresource = rt.m_dependencySubresources[i];
			const Bool crntIsReadDep = !!(crntRtUsage & TextureUsageBit::kAllRead);
			const Bool crntIsWriteDep = !!(crntRtUsage & TextureUsageBit::kAllWrite);

			U32 j = i;
			while(j--)
			{
				const TextureUsageBit prevRtUsage = rt.m_dependencyUsages[j];
				const TextureSubresourceDesc& prevSubresource = rt.m_dependencySubresources[j];
				const Bool prevIsReadDep = !!(prevRtUsage & TextureUsageBit::kAllRead);
				const Bool prevIsWriteDep = !!(prevRtUsage & TextureUsageBit::kAllWrite);

				const Bool crntReadPrevWrite = crntIsReadDep && prevIsWriteDep;
				const Bool crntWritePrevRead = crntIsWriteDep && prevIsReadDep;
				const Bool crntWritePrevWrite = crntIsWriteDep && prevIsWriteDep;

				const Bool crntDependsOnPrev = crntReadPrevWrite || crntWritePrevRead || crntWritePrevWrite;
				const Bool subresourcesOverlap = crntSubresource.overlapsWith(prevSubresource);

				if(crntDependsOnPrev && subresourcesOverlap)
				{
					const U32 crntPassIdx = rt.m_dependentPasses[i];
					const U32 prevPassIdx = rt.m_dependentPasses[j];

					ctx.m_passes[crntPassIdx].m_dependsOnPassMask.setBit(prevPassIdx);
				}
			}
		}
	}

	// Find depedent passes using buffers
	for(const BufferRange& buff : ctx.m_buffers)
	{
		const U32 dependencyCount = buff.m_dependencyUsages.getSize();
		for(U32 i = 1; i < dependencyCount; ++i)
		{
			const BufferUsageBit crntRtUsage = buff.m_dependencyUsages[i];
			const Bool crntIsReadDep = !!(crntRtUsage & BufferUsageBit::kAllRead);
			const Bool crntIsWriteDep = !!(crntRtUsage & BufferUsageBit::kAllWrite);

			U32 j = i;
			while(j--)
			{
				const BufferUsageBit prevRtUsage = buff.m_dependencyUsages[j];
				const Bool prevIsReadDep = !!(prevRtUsage & BufferUsageBit::kAllRead);
				const Bool prevIsWriteDep = !!(prevRtUsage & BufferUsageBit::kAllWrite);

				const Bool crntReadPrevWrite = crntIsReadDep && prevIsWriteDep;
				const Bool crntWritePrevRead = crntIsWriteDep && prevIsReadDep;
				const Bool crntWritePrevWrite = crntIsWriteDep && prevIsWriteDep;

				const Bool crntDependsOnPrev = crntReadPrevWrite || crntWritePrevRead || crntWritePrevWrite;

				if(crntDependsOnPrev)
				{
					const U32 crntPassIdx = buff.m_dependentPasses[i];
					const U32 prevPassIdx = buff.m_dependentPasses[j];

					ctx.m_passes[crntPassIdx].m_dependsOnPassMask.setBit(prevPassIdx);
				}
			}
		}
	}

	// Find depedent passes using acceleration structures
	for(const AS& as : ctx.m_as)
	{
		const U32 dependencyCount = as.m_dependencyUsages.getSize();
		for(U32 i = 1; i < dependencyCount; ++i)
		{
			const AccelerationStructureUsageBit crntRtUsage = as.m_dependencyUsages[i];
			const Bool crntIsReadDep = !!(crntRtUsage & AccelerationStructureUsageBit::kAllRead);
			const Bool crntIsWriteDep = !!(crntRtUsage & AccelerationStructureUsageBit::kAllWrite);

			U32 j = i;
			while(j--)
			{
				const AccelerationStructureUsageBit prevRtUsage = as.m_dependencyUsages[j];
				const Bool prevIsReadDep = !!(prevRtUsage & AccelerationStructureUsageBit::kAllRead);
				const Bool prevIsWriteDep = !!(prevRtUsage & AccelerationStructureUsageBit::kAllWrite);

				const Bool crntReadPrevWrite = crntIsReadDep && prevIsWriteDep;
				const Bool crntWritePrevRead = crntIsWriteDep && prevIsReadDep;
				const Bool crntWritePrevWrite = crntIsWriteDep && prevIsWriteDep;

				const Bool crntDependsOnPrev = crntReadPrevWrite || crntWritePrevRead || crntWritePrevWrite;

				if(crntDependsOnPrev)
				{
					const U32 crntPassIdx = as.m_dependentPasses[i];
					const U32 prevPassIdx = as.m_dependentPasses[j];

					ctx.m_passes[crntPassIdx].m_dependsOnPassMask.setBit(prevPassIdx);
				}
			}
		}
	}
}

void RenderGraph::initBatches()
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(m_ctx);

	U32 passesAssignedToBatchCount = 0;
	const U32 passCount = m_ctx->m_passes.getSize();
	ANKI_ASSERT(passCount > 0);
	while(passesAssignedToBatchCount < passCount)
	{
		Batch& batch = *m_ctx->m_batches.emplaceBack(m_ctx->m_batches.getMemoryPool().m_pool);

		for(U32 i = 0; i < passCount; ++i)
		{
			if(!m_ctx->m_passIsInBatch.getBit(i) && !passHasUnmetDependencies(*m_ctx, i))
			{
				// Add to the batch
				++passesAssignedToBatchCount;
				batch.m_passIndices.emplaceBack(i);
			}
		}

		// Mark batch's passes done
		for(U32 passIdx : batch.m_passIndices)
		{
			m_ctx->m_passIsInBatch.setBit(passIdx);
			m_ctx->m_passes[passIdx].m_batchIdx = m_ctx->m_batches.getSize() - 1;
		}
	}
}

void RenderGraph::initGraphicsPasses(const RenderGraphBuilder& descr)
{
	ANKI_TRACE_FUNCTION();

	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassBase& baseInPass = descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];

		// Create command buffers and framebuffer
		const Bool isGraphicsPass = baseInPass.m_graphicsPassExtra != nullptr;
		if(isGraphicsPass)
		{
			const GraphicsRenderPass& inPass = static_cast<const GraphicsRenderPass&>(baseInPass);

			if(inPass.m_graphicsPassExtra->m_hasRenderpass)
			{
				outPass.m_beginRenderpassInfo.m_hasRenderpass = true;
				outPass.m_beginRenderpassInfo.m_colorRtCount = inPass.m_graphicsPassExtra->m_colorRtCount;

				// Init the usage bits
				for(U32 i = 0; i < inPass.m_graphicsPassExtra->m_colorRtCount; ++i)
				{
					const GraphicsRenderPassTargetDesc& inAttachment = inPass.m_graphicsPassExtra->m_rts[i];
					RenderTarget& outAttachment = outPass.m_beginRenderpassInfo.m_colorRts[i];

					getCrntUsage(inAttachment.m_handle, outPass.m_batchIdx, inAttachment.m_subresource, outAttachment.m_usage);

					outAttachment.m_textureView = TextureView(m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture.get(), inAttachment.m_subresource);
					outPass.m_beginRenderpassInfo.m_refs[i] = m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture;

					outAttachment.m_loadOperation = inAttachment.m_loadOperation;
					outAttachment.m_storeOperation = inAttachment.m_storeOperation;
					outAttachment.m_clearValue = inAttachment.m_clearValue;
				}

				if(!!inPass.m_graphicsPassExtra->m_rts[kMaxColorRenderTargets].m_subresource.m_depthStencilAspect)
				{
					const GraphicsRenderPassTargetDesc& inAttachment = inPass.m_graphicsPassExtra->m_rts[kMaxColorRenderTargets];
					RenderTarget& outAttachment = outPass.m_beginRenderpassInfo.m_dsRt;

					getCrntUsage(inAttachment.m_handle, outPass.m_batchIdx, inAttachment.m_subresource, outAttachment.m_usage);

					outAttachment.m_textureView = TextureView(m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture.get(), inAttachment.m_subresource);
					outPass.m_beginRenderpassInfo.m_refs[kMaxColorRenderTargets] = m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture;

					outAttachment.m_loadOperation = inAttachment.m_loadOperation;
					outAttachment.m_storeOperation = inAttachment.m_storeOperation;
					outAttachment.m_stencilLoadOperation = inAttachment.m_stencilLoadOperation;
					outAttachment.m_stencilStoreOperation = inAttachment.m_stencilStoreOperation;
					outAttachment.m_clearValue = inAttachment.m_clearValue;
				}

				if(inPass.m_graphicsPassExtra->m_vrsRtTexelSizeX > 0)
				{
					const GraphicsRenderPassTargetDesc& inAttachment = inPass.m_graphicsPassExtra->m_rts[kMaxColorRenderTargets + 1];

					outPass.m_beginRenderpassInfo.m_vrsRt =
						TextureView(m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture.get(), inAttachment.m_subresource);
					outPass.m_beginRenderpassInfo.m_refs[kMaxColorRenderTargets + 1] = m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture;

					outPass.m_beginRenderpassInfo.m_vrsTexelSizeX = inPass.m_graphicsPassExtra->m_vrsRtTexelSizeX;
					outPass.m_beginRenderpassInfo.m_vrsTexelSizeY = inPass.m_graphicsPassExtra->m_vrsRtTexelSizeY;
				}
			}
		}
	}
}

template<typename TFunc>
void RenderGraph::iterateSurfsOrVolumes(const Texture& tex, const TextureSubresourceDesc& subresource, TFunc func)
{
	subresource.validate(tex);
	const U32 faceCount = textureTypeIsCube(tex.getTextureType()) ? 6 : 1;

	if(subresource.m_allSurfacesOrVolumes)
	{
		for(U32 mip = 0; mip < tex.getMipmapCount(); ++mip)
		{
			for(U32 layer = 0; layer < tex.getLayerCount(); ++layer)
			{
				for(U32 face = 0; face < faceCount; ++face)
				{
					// Compute surf or vol idx
					const U32 idx = (faceCount * tex.getLayerCount()) * mip + faceCount * layer + face;

					if(!func(idx, TextureSubresourceDesc::surface(mip, face, layer, subresource.m_depthStencilAspect)))
					{
						return;
					}
				}
			}
		}
	}
	else
	{
		const U32 idx = (faceCount * tex.getLayerCount()) * subresource.m_mipmap + faceCount * subresource.m_layer + subresource.m_face;

		func(idx, subresource);
	}
}

void RenderGraph::setTextureBarrier(Batch& batch, const RenderPassBase::TextureDependency& dep)
{
	BakeContext& ctx = *m_ctx;
	const U32 batchIdx = U32(&batch - &ctx.m_batches[0]);
	const U32 rtIdx = dep.m_handle.m_idx;
	const TextureUsageBit depUsage = dep.m_usage;
	RT& rt = ctx.m_rts[rtIdx];

	iterateSurfsOrVolumes(*rt.m_texture, dep.m_subresource, [&](U32 surfOrVolIdx, const TextureSubresourceDesc& subresource) {
		TextureUsageBit& crntUsage = rt.m_surfOrVolUsages[surfOrVolIdx];

		const Bool skipBarrier = crntUsage == depUsage && !(crntUsage & TextureUsageBit::kAllWrite);

		if(!skipBarrier)
		{
			// Check if we can merge barriers
			if(rt.m_lastBatchThatTransitionedIt[surfOrVolIdx] == batchIdx)
			{
				// Will merge the barriers

				crntUsage |= depUsage;

				[[maybe_unused]] Bool found = false;
				for(TextureBarrier& b : batch.m_textureBarriersBefore)
				{
					if(b.m_idx == rtIdx && b.m_subresource == subresource)
					{
						b.m_usageAfter |= depUsage;
						found = true;
						break;
					}
				}

				ANKI_ASSERT(found);
			}
			else
			{
				// Create a new barrier for this surface

				batch.m_textureBarriersBefore.emplaceBack(rtIdx, crntUsage, depUsage, subresource);

				crntUsage = depUsage;
				rt.m_lastBatchThatTransitionedIt[surfOrVolIdx] = U16(batchIdx);
			}
		}

		return true;
	});
}

void RenderGraph::setBatchBarriers(const RenderGraphBuilder& descr)
{
	ANKI_TRACE_FUNCTION();

	BakeContext& ctx = *m_ctx;

	// For all batches
	for(Batch& batch : ctx.m_batches)
	{
		DynamicBitSet<MemoryPoolPtrWrapper<StackMemoryPool>, U64> buffHasBarrierMask(ctx.m_batches.getMemoryPool());
		DynamicBitSet<MemoryPoolPtrWrapper<StackMemoryPool>, U64> asHasBarrierMask(ctx.m_batches.getMemoryPool());

		// For all passes of that batch
		for(U32 passIdx : batch.m_passIndices)
		{
			const RenderPassBase& pass = descr.m_passes[passIdx];

			// Do textures
			for(const RenderPassBase::TextureDependency& dep : pass.m_rtDeps)
			{
				setTextureBarrier(batch, dep);
			}

			// Do buffers
			for(const RenderPassBase::BufferDependency& dep : pass.m_buffDeps)
			{
				const U32 buffIdx = dep.m_handle.m_idx;
				const BufferUsageBit depUsage = dep.m_usage;
				BufferUsageBit& crntUsage = ctx.m_buffers[buffIdx].m_usage;

				const Bool skipBarrier = crntUsage == depUsage && !(crntUsage & BufferUsageBit::kAllWrite);
				if(skipBarrier)
				{
					continue;
				}

				const Bool buffHasBarrier = buffHasBarrierMask.getBit(buffIdx);

				if(!buffHasBarrier)
				{
					// Buff hasn't had a barrier in this batch, add a new barrier

					batch.m_bufferBarriersBefore.emplaceBack(buffIdx, crntUsage, depUsage);

					crntUsage = depUsage;
					buffHasBarrierMask.setBit(buffIdx);
				}
				else
				{
					// Buff already in a barrier, merge the 2 barriers

					BufferBarrier* barrierToMergeTo = nullptr;
					for(BufferBarrier& b : batch.m_bufferBarriersBefore)
					{
						if(b.m_idx == buffIdx)
						{
							barrierToMergeTo = &b;
							break;
						}
					}

					ANKI_ASSERT(barrierToMergeTo);
					ANKI_ASSERT(!!barrierToMergeTo->m_usageAfter);
					barrierToMergeTo->m_usageAfter |= depUsage;
					crntUsage = barrierToMergeTo->m_usageAfter;
				}
			}

			// Do AS
			for(const RenderPassBase::ASDependency& dep : pass.m_asDeps)
			{
				const U32 asIdx = dep.m_handle.m_idx;
				const AccelerationStructureUsageBit depUsage = dep.m_usage;
				AccelerationStructureUsageBit& crntUsage = ctx.m_as[asIdx].m_usage;

				const Bool skipBarrier = crntUsage == depUsage && !(crntUsage & AccelerationStructureUsageBit::kAllWrite);
				if(skipBarrier)
				{
					continue;
				}

				const Bool asHasBarrierInThisBatch = asHasBarrierMask.getBit(asIdx);
				if(!asHasBarrierInThisBatch)
				{
					// AS doesn't have a barrier in this batch, create a new one

					batch.m_asBarriersBefore.emplaceBack(asIdx, crntUsage, depUsage);
					crntUsage = depUsage;
					asHasBarrierMask.setBit(asIdx);
				}
				else
				{
					// AS already has a barrier, merge the 2 barriers

					ASBarrier* barrierToMergeTo = nullptr;
					for(ASBarrier& other : batch.m_asBarriersBefore)
					{
						if(other.m_idx == asIdx)
						{
							barrierToMergeTo = &other;
							break;
						}
					}

					ANKI_ASSERT(barrierToMergeTo);
					ANKI_ASSERT(!!barrierToMergeTo->m_usageAfter);
					barrierToMergeTo->m_usageAfter |= depUsage;
					crntUsage = barrierToMergeTo->m_usageAfter;
				}
			}
		} // For all passes

		ANKI_ASSERT(batch.m_bufferBarriersBefore.getSize() || batch.m_textureBarriersBefore.getSize() || batch.m_asBarriersBefore.getSize());

#if ANKI_DBG_RENDER_GRAPH
		// Sort the barriers to ease the dumped graph
		std::sort(batch.m_textureBarriersBefore.getBegin(), batch.m_textureBarriersBefore.getEnd(),
				  [&](const TextureBarrier& a, const TextureBarrier& b) {
					  const U aidx = a.m_idx;
					  const U bidx = b.m_idx;

					  if(aidx == bidx)
					  {
						  if(a.m_surface.m_level != b.m_surface.m_level)
						  {
							  return a.m_surface.m_level < b.m_surface.m_level;
						  }
						  else if(a.m_surface.m_face != b.m_surface.m_face)
						  {
							  return a.m_surface.m_face < b.m_surface.m_face;
						  }
						  else if(a.m_surface.m_layer != b.m_surface.m_layer)
						  {
							  return a.m_surface.m_layer < b.m_surface.m_layer;
						  }
						  else
						  {
							  return false;
						  }
					  }
					  else
					  {
						  return aidx < bidx;
					  }
				  });

		std::sort(batch.m_bufferBarriersBefore.getBegin(), batch.m_bufferBarriersBefore.getEnd(),
				  [&](const BufferBarrier& a, const BufferBarrier& b) {
					  return a.m_idx < b.m_idx;
				  });

		std::sort(batch.m_asBarriersBefore.getBegin(), batch.m_asBarriersBefore.getEnd(), [&](const ASBarrier& a, const ASBarrier& b) {
			return a.m_idx < b.m_idx;
		});
#endif
	} // For all batches
}

void RenderGraph::minimizeSubchannelSwitches()
{
	BakeContext& ctx = *m_ctx;

	Bool computeFirst = true;
	for(Batch& batch : ctx.m_batches)
	{
		U32 graphicsPasses = 0;
		U32 computePasses = 0;

		std::sort(batch.m_passIndices.getBegin(), batch.m_passIndices.getEnd(), [&](U32 a, U32 b) {
			const Bool aIsCompute = !ctx.m_passes[a].m_beginRenderpassInfo.m_hasRenderpass;
			const Bool bIsCompute = !ctx.m_passes[b].m_beginRenderpassInfo.m_hasRenderpass;

			graphicsPasses += !aIsCompute + !bIsCompute;
			computePasses += aIsCompute + bIsCompute;

			if(computeFirst)
			{
				return !aIsCompute < !bIsCompute;
			}
			else
			{
				return aIsCompute < bIsCompute;
			}
		});

		if(graphicsPasses && !computePasses)
		{
			// Only graphics passes in this batch, start next batch from graphics
			computeFirst = false;
		}
		else if(computePasses && !graphicsPasses)
		{
			// Only compute passes in this batch, start next batch from compute
			computeFirst = true;
		}
		else
		{
			// This batch ends in compute start next batch in compute and if it ends with graphics start next in graphics
			computeFirst = !computeFirst;
		}
	}
}

void RenderGraph::sortBatchPasses()
{
	BakeContext& ctx = *m_ctx;

	for(Batch& batch : ctx.m_batches)
	{
		std::sort(batch.m_passIndices.getBegin(), batch.m_passIndices.getEnd(), [&](U32 a, U32 b) {
			const Bool aIsCompute = !ctx.m_passes[a].m_beginRenderpassInfo.m_hasRenderpass;
			const Bool bIsCompute = !ctx.m_passes[b].m_beginRenderpassInfo.m_hasRenderpass;

			return aIsCompute < bIsCompute;
		});
	}
}

void RenderGraph::compileNewGraph(const RenderGraphBuilder& descr, StackMemoryPool& pool)
{
	ANKI_TRACE_SCOPED_EVENT(GrRenderGraphCompile);

	// Init the context
	BakeContext& ctx = *newContext(descr, pool);
	m_ctx = &ctx;

	// Init the passes and find the dependencies between passes
	initRenderPassesAndSetDeps(descr);

	// Walk the graph and create pass batches
	initBatches();

	// Now that we know the batches every pass belongs init the graphics passes
	initGraphicsPasses(descr);

	// Create barriers between batches
	setBatchBarriers(descr);

	// Sort passes in batches
	if(GrManager::getSingleton().getDeviceCapabilities().m_gpuVendor == GpuVendor::kNvidia)
	{
		minimizeSubchannelSwitches();
	}
	else
	{
		sortBatchPasses();
	}

#if ANKI_DBG_RENDER_GRAPH
	if(dumpDependencyDotFile(descr, ctx, "./"))
	{
		ANKI_LOGF("Won't recover on debug code");
	}
#endif
}

Texture& RenderGraph::getTexture(RenderTargetHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_rts[handle.m_idx].m_texture.isCreated());
	return *m_ctx->m_rts[handle.m_idx].m_texture;
}

void RenderGraph::getCachedBuffer(BufferHandle handle, Buffer*& buff, PtrSize& offset, PtrSize& range) const
{
	const BufferRange& record = m_ctx->m_buffers[handle.m_idx];
	buff = record.m_buffer.get();
	offset = record.m_offset;
	range = record.m_range;
}

AccelerationStructure* RenderGraph::getAs(AccelerationStructureHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_as[handle.m_idx].m_as.isCreated());
	return m_ctx->m_as[handle.m_idx].m_as.get();
}

void RenderGraph::recordAndSubmitCommandBuffers(FencePtr* optionalFence)
{
	ANKI_TRACE_SCOPED_EVENT(GrRenderGraphRecordAndSubmit);
	ANKI_ASSERT(m_ctx);

	const U32 batchGroupCount = min(CoreThreadJobManager::getSingleton().getThreadCount(), m_ctx->m_batches.getSize());
	StackMemoryPool* pool = m_ctx->m_batches.getMemoryPool().m_pool;

	DynamicArray<CommandBufferPtr, MemoryPoolPtrWrapper<StackMemoryPool>> cmdbs(pool);
	cmdbs.resize(batchGroupCount);
	SpinLock cmdbsMtx;
	Atomic<U32> firstGroupThatWroteToSwapchain(kMaxU32);

	for(U32 group = 0; group < batchGroupCount; ++group)
	{
		U32 start, end;
		splitThreadedProblem(group, batchGroupCount, m_ctx->m_batches.getSize(), start, end);

		if(start == end)
		{
			continue;
		}

		CoreThreadJobManager::getSingleton().dispatchTask(
			[this, start, end, pool, &cmdbs, &cmdbsMtx, group, batchGroupCount, &firstGroupThatWroteToSwapchain]([[maybe_unused]] U32 tid) {
				ANKI_TRACE_SCOPED_EVENT(GrRenderGraphTask);

				Array<Char, 32> name;
				snprintf(name.getBegin(), name.getSize(), "RenderGraph cmdb %u-%u", start, end);
				CommandBufferInitInfo cmdbInit(name.getBegin());
				cmdbInit.m_flags = CommandBufferFlag::kGeneralWork;
				CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

				// Write timestamp
				const Bool setPreQuery = m_ctx->m_gatherStatistics && group == 0;
				const Bool setPostQuery = m_ctx->m_gatherStatistics && group == batchGroupCount - 1;
				TimestampQueryInternalPtr preQuery, postQuery;
				if(setPreQuery)
				{
					preQuery = GrManager::getSingleton().newTimestampQuery();
					cmdb->writeTimestamp(preQuery.get());
				}

				if(setPostQuery)
				{
					postQuery = GrManager::getSingleton().newTimestampQuery();
				}

				// Bookkeeping
				{
					LockGuard lock(cmdbsMtx);
					cmdbs[group] = cmdb;

					if(preQuery.isCreated())
					{
						m_statistics.m_timestamps[m_statistics.m_nextTimestamp][0] = preQuery;
					}

					if(postQuery.isCreated())
					{
						m_statistics.m_timestamps[m_statistics.m_nextTimestamp][1] = postQuery;
						m_statistics.m_cpuStartTimes[m_statistics.m_nextTimestamp] = HighRezTimer::getCurrentTime();
					}
				}

				RenderPassWorkContext ctx;
				ctx.m_rgraph = this;

				for(U32 i = start; i < end; ++i)
				{
					const Batch& batch = m_ctx->m_batches[i];

					// Set the barriers
					DynamicArray<TextureBarrierInfo, MemoryPoolPtrWrapper<StackMemoryPool>> texBarriers(pool);
					texBarriers.resizeStorage(batch.m_textureBarriersBefore.getSize());
					for(const TextureBarrier& barrier : batch.m_textureBarriersBefore)
					{
						const Texture& tex = *m_ctx->m_rts[barrier.m_idx].m_texture;
						TextureBarrierInfo& inf = *texBarriers.emplaceBack();
						inf.m_previousUsage = barrier.m_usageBefore;
						inf.m_nextUsage = barrier.m_usageAfter;
						inf.m_textureView = TextureView(&tex, barrier.m_subresource);
					}

					DynamicArray<BufferBarrierInfo, MemoryPoolPtrWrapper<StackMemoryPool>> buffBarriers(pool);
					buffBarriers.resizeStorage(batch.m_bufferBarriersBefore.getSize());
					for(const BufferBarrier& barrier : batch.m_bufferBarriersBefore)
					{
						BufferBarrierInfo& inf = *buffBarriers.emplaceBack();
						inf.m_previousUsage = barrier.m_usageBefore;
						inf.m_nextUsage = barrier.m_usageAfter;
						inf.m_bufferView = BufferView(m_ctx->m_buffers[barrier.m_idx].m_buffer.get(), m_ctx->m_buffers[barrier.m_idx].m_offset,
													  m_ctx->m_buffers[barrier.m_idx].m_range);
					}

					// Sort them for the command buffer to merge as many as possible
					std::sort(buffBarriers.getBegin(), buffBarriers.getEnd(), [](const BufferBarrierInfo& a, const BufferBarrierInfo& b) {
						return a.m_bufferView.getBuffer().getUuid() < b.m_bufferView.getBuffer().getUuid();
					});

					DynamicArray<AccelerationStructureBarrierInfo, MemoryPoolPtrWrapper<StackMemoryPool>> asBarriers(pool);
					for(const ASBarrier& barrier : batch.m_asBarriersBefore)
					{
						AccelerationStructureBarrierInfo& inf = *asBarriers.emplaceBack();
						inf.m_previousUsage = barrier.m_usageBefore;
						inf.m_nextUsage = barrier.m_usageAfter;
						inf.m_as = m_ctx->m_as[barrier.m_idx].m_as.get();
					}

					cmdb->pushDebugMarker("Barrier", Vec3(1.0f, 0.0f, 0.0f));
					cmdb->setPipelineBarrier(texBarriers, buffBarriers, asBarriers);
					cmdb->popDebugMarker();

					ctx.m_commandBuffer = cmdb.get();
					ctx.m_batchIdx = i;

					// Call the passes
					for(U32 passIdx : batch.m_passIndices)
					{
						Pass& pass = m_ctx->m_passes[passIdx];

						if(pass.m_writesToSwapchain)
						{
							firstGroupThatWroteToSwapchain.min(group);
						}

						const Vec3 passColor = (pass.m_beginRenderpassInfo.m_hasRenderpass) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(1.0f, 1.0f, 0.0f);
						cmdb->pushDebugMarker(pass.m_name, passColor);

						if(pass.m_beginRenderpassInfo.m_hasRenderpass)
						{
							cmdb->beginRenderPass({pass.m_beginRenderpassInfo.m_colorRts.getBegin(), U32(pass.m_beginRenderpassInfo.m_colorRtCount)},
												  pass.m_beginRenderpassInfo.m_dsRt.m_textureView.isValid() ? &pass.m_beginRenderpassInfo.m_dsRt
																											: nullptr,
												  pass.m_beginRenderpassInfo.m_vrsRt, pass.m_beginRenderpassInfo.m_vrsTexelSizeX,
												  pass.m_beginRenderpassInfo.m_vrsTexelSizeY);
						}

						{
							ANKI_TRACE_SCOPED_EVENT(GrRenderGraphCallback);
							ctx.m_passIdx = passIdx;
							pass.m_callback(ctx);
						}

						if(pass.m_beginRenderpassInfo.m_hasRenderpass)
						{
							cmdb->endRenderPass();
						}

						cmdb->popDebugMarker();
					}
				} // end for batches

				if(setPostQuery)
				{
					// Write a timestamp before the last flush
					cmdb->writeTimestamp(postQuery.get());
				}

				cmdb->endRecording();
			});
	}

	CoreThreadJobManager::getSingleton().waitForAllTasksToFinish();

	// Submit
	DynamicArray<CommandBuffer*, MemoryPoolPtrWrapper<StackMemoryPool>> pCmdbs(pool);
	pCmdbs.resize(cmdbs.getSize());
	for(U32 i = 0; i < cmdbs.getSize(); ++i)
	{
		pCmdbs[i] = cmdbs[i].get();
	}

	FencePtr fence;
	const U32 firstGroupThatWroteToSwapchain2 = firstGroupThatWroteToSwapchain.getNonAtomically();
	if(firstGroupThatWroteToSwapchain2 == 0 || firstGroupThatWroteToSwapchain2 == kMaxU32)
	{
		GrManager::getSingleton().submit(WeakArray(pCmdbs), {}, &fence, true);
	}
	else
	{
		// 2 submits. The 1st contains all the batches that don't write to swapchain

		GrManager::getSingleton().submit(WeakArray(pCmdbs).subrange(0, firstGroupThatWroteToSwapchain2), {}, nullptr);

		GrManager::getSingleton().submit(
			WeakArray(pCmdbs).subrange(firstGroupThatWroteToSwapchain2, batchGroupCount - firstGroupThatWroteToSwapchain2), {}, &fence, true);
	}

	if(optionalFence)
	{
		*optionalFence = fence;
	}

	m_texMemPool.endFrame(fence.get());
}

void RenderGraph::getCrntUsage(RenderTargetHandle handle, U32 batchIdx, const TextureSubresourceDesc& subresource, TextureUsageBit& usage) const
{
	usage = TextureUsageBit::kNone;
	const Batch& batch = m_ctx->m_batches[batchIdx];

	for(U32 passIdx : batch.m_passIndices)
	{
		for(const RenderPassBase::TextureDependency& consumer : m_ctx->m_passes[passIdx].m_consumedTextures)
		{
			if(consumer.m_handle == handle && subresource.overlapsWith(consumer.m_subresource))
			{
				usage |= consumer.m_usage;
				break;
			}
		}
	}
}

void RenderGraph::periodicCleanup()
{
	U32 rtsCleanedCount = 0;
	for(RenderTargetCacheEntry& entry : m_renderTargetCache)
	{
		if(entry.m_texturesInUse < entry.m_textures.getSize())
		{
			// Should cleanup

			rtsCleanedCount += entry.m_textures.getSize() - entry.m_texturesInUse;

			// New array
			GrDynamicArray<TextureInternalPtr> newArray;
			GrDynamicArray<SegregatedListsGpuMemoryPoolAllocation> newAllocations;
			if(entry.m_texturesInUse > 0)
			{
				newArray.resize(entry.m_texturesInUse);
				newAllocations.resize(entry.m_texturesInUse);
			}

			// Populate the new array
			for(U32 i = 0; i < newArray.getSize(); ++i)
			{
				newArray[i] = std::move(entry.m_textures[i]);
				newAllocations[i] = std::move(entry.m_texAllocations[i]);
			}

			// Destroy the old array and the rest of the textures
			entry.m_textures.destroy();
			entry.m_texAllocations.destroy();

			// Move new array
			entry.m_textures = std::move(newArray);
			entry.m_texAllocations = std::move(newAllocations);
		}
	}

	if(rtsCleanedCount > 0)
	{
		ANKI_GR_LOGI("Cleaned %u render targets", rtsCleanedCount);
	}
}

void RenderGraph::getStatistics(RenderGraphStatistics& statistics)
{
	m_statistics.m_nextTimestamp = (m_statistics.m_nextTimestamp + 1) % kMaxBufferedTimestamps;
	const U32 oldFrame = m_statistics.m_nextTimestamp;

	if(m_statistics.m_timestamps[oldFrame][0].isCreated() && m_statistics.m_timestamps[oldFrame][1].isCreated())
	{
		Second start, end;
		[[maybe_unused]] TimestampQueryResult res = m_statistics.m_timestamps[oldFrame][0]->getResult(start);
		ANKI_ASSERT(res == TimestampQueryResult::kAvailable);
		m_statistics.m_timestamps[oldFrame][0].reset(nullptr);

		res = m_statistics.m_timestamps[oldFrame][1]->getResult(end);
		ANKI_ASSERT(res == TimestampQueryResult::kAvailable);
		m_statistics.m_timestamps[oldFrame][1].reset(nullptr);

		const Second diff = end - start;
		statistics.m_gpuTime = diff;
		statistics.m_cpuStartTime = m_statistics.m_cpuStartTimes[oldFrame];
	}
	else
	{
		statistics.m_gpuTime = -1.0;
		statistics.m_cpuStartTime = -1.0;
	}

	m_texMemPool.getStats(statistics.m_gpuMemoryUsed, statistics.m_gpuMemoryPoolCapacity);
}

#if ANKI_DBG_RENDER_GRAPH
StringRaii RenderGraph::textureUsageToStr(StackMemoryPool& pool, TextureUsageBit usage)
{
	if(!usage)
	{
		return StringRaii(&pool, "None");
	}

	StringListRaii slist(&pool);

#	define ANKI_TEX_USAGE(u) \
		if(!!(usage & TextureUsageBit::u)) \
		{ \
			slist.pushBackSprintf("%s", #u); \
		}

	ANKI_TEX_USAGE(kSampledGeometry);
	ANKI_TEX_USAGE(kSampledFragment);
	ANKI_TEX_USAGE(kSampledCompute);
	ANKI_TEX_USAGE(kSampledTraceRays);
	ANKI_TEX_USAGE(kUavGeometryRead);
	ANKI_TEX_USAGE(kUavGeometryWrite);
	ANKI_TEX_USAGE(kUavFragmentRead);
	ANKI_TEX_USAGE(kUavFragmentWrite);
	ANKI_TEX_USAGE(kUavComputeRead);
	ANKI_TEX_USAGE(kUavComputeWrite);
	ANKI_TEX_USAGE(kUavTraceRaysRead);
	ANKI_TEX_USAGE(kUavTraceRaysWrite);
	ANKI_TEX_USAGE(kFramebufferRead);
	ANKI_TEX_USAGE(kFramebufferWrite);
	ANKI_TEX_USAGE(kTransferDestination);
	ANKI_TEX_USAGE(kGenerateMipmaps);
	ANKI_TEX_USAGE(kPresent);
	ANKI_TEX_USAGE(kFramebufferShadingRate);

	if(!usage)
	{
		slist.pushBackSprintf("?");
	}

#	undef ANKI_TEX_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringRaii str(&pool);
	slist.join(" | ", str);
	return str;
}

StringRaii RenderGraph::bufferUsageToStr(StackMemoryPool& pool, BufferUsageBit usage)
{
	StringListRaii slist(&pool);

#	define ANKI_BUFF_USAGE(u) \
		if(!!(usage & BufferUsageBit::u)) \
		{ \
			slist.pushBackSprintf("%s", #u); \
		}

	ANKI_BUFF_USAGE(kConstantGeometry);
	ANKI_BUFF_USAGE(kConstantPixel);
	ANKI_BUFF_USAGE(kConstantCompute);
	ANKI_BUFF_USAGE(kConstantDispatchRays);
	ANKI_BUFF_USAGE(kStorageGeometryRead);
	ANKI_BUFF_USAGE(kStorageGeometryWrite);
	ANKI_BUFF_USAGE(kStorageFragmentRead);
	ANKI_BUFF_USAGE(kStorageFragmentWrite);
	ANKI_BUFF_USAGE(kStorageComputeRead);
	ANKI_BUFF_USAGE(kStorageComputeWrite);
	ANKI_BUFF_USAGE(kStorageTraceRaysRead);
	ANKI_BUFF_USAGE(kStorageTraceRaysWrite);
	ANKI_BUFF_USAGE(kTextureGeometryRead);
	ANKI_BUFF_USAGE(kTextureGeometryWrite);
	ANKI_BUFF_USAGE(kTextureFragmentRead);
	ANKI_BUFF_USAGE(kTextureFragmentWrite);
	ANKI_BUFF_USAGE(kTextureComputeRead);
	ANKI_BUFF_USAGE(kTextureComputeWrite);
	ANKI_BUFF_USAGE(kTextureTraceRaysRead);
	ANKI_BUFF_USAGE(kTextureTraceRaysWrite);
	ANKI_BUFF_USAGE(kIndex);
	ANKI_BUFF_USAGE(kVertex);
	ANKI_BUFF_USAGE(kIndirectCompute);
	ANKI_BUFF_USAGE(kIndirectDraw);
	ANKI_BUFF_USAGE(kIndirectDispatchRays);
	ANKI_BUFF_USAGE(kTransferSource);
	ANKI_BUFF_USAGE(kTransferDestination);
	ANKI_BUFF_USAGE(kAccelerationStructureBuild);

	if(!usage)
	{
		slist.pushBackSprintf("NONE");
	}

#	undef ANKI_BUFF_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringRaii str(&pool);
	slist.join(" | ", str);
	return str;
}

StringRaii RenderGraph::asUsageToStr(StackMemoryPool& pool, AccelerationStructureUsageBit usage)
{
	StringListRaii slist(&pool);

#	define ANKI_AS_USAGE(u) \
		if(!!(usage & AccelerationStructureUsageBit::u)) \
		{ \
			slist.pushBackSprintf("%s", #u); \
		}

	ANKI_AS_USAGE(kBuild);
	ANKI_AS_USAGE(kAttach);
	ANKI_AS_USAGE(kGeometryRead);
	ANKI_AS_USAGE(kFragmentRead);
	ANKI_AS_USAGE(kComputeRead);
	ANKI_AS_USAGE(kTraceRaysRead);

	if(!usage)
	{
		slist.pushBackSprintf("NONE");
	}

#	undef ANKI_AS_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringRaii str(&pool);
	slist.join(" | ", str);
	return str;
}

Error RenderGraph::dumpDependencyDotFile(const RenderGraphBuilder& descr, const BakeContext& ctx, CString path) const
{
	ANKI_GR_LOGW("Running with debug code");

	static constexpr Array<const char*, 5> COLORS = {"red", "green", "blue", "magenta", "cyan"};
	StackMemoryPool& pool = *ctx.m_pool;
	StringListRaii slist(&pool);

	slist.pushBackSprintf("digraph {\n");
	slist.pushBackSprintf("\t//splines = ortho;\nconcentrate = true;\n");

	for(U32 batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		// Set same rank
		slist.pushBackSprintf("\t{rank=\"same\";");
		for(U32 passIdx : ctx.m_batches[batchIdx].m_passIndices)
		{
			slist.pushBackSprintf("\"%s\";", descr.m_passes[passIdx]->m_name.cstr());
		}
		slist.pushBackSprintf("}\n");

		// Print passes
		for(U32 passIdx : ctx.m_batches[batchIdx].m_passIndices)
		{
			CString passName = descr.m_passes[passIdx]->m_name.toCString();

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=%s,shape=box];\n", passName.cstr(), COLORS[batchIdx % COLORS.getSize()],
								  (descr.m_passes[passIdx]->m_type == RenderPassBase::Type::kGraphics) ? "bold" : "dashed");

			for(U32 depIdx : ctx.m_passes[passIdx].m_dependsOn)
			{
				slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", descr.m_passes[depIdx]->m_name.cstr(), passName.cstr());
			}

			if(ctx.m_passes[passIdx].m_dependsOn.getSize() == 0)
			{
				slist.pushBackSprintf("\tNONE->\"%s\";\n", descr.m_passes[passIdx]->m_name.cstr());
			}
		}
	}

#	if 0
	// Color the resources
	slist.pushBackSprintf("subgraph cluster_0 {\n");
	for(U rtIdx = 0; rtIdx < descr.m_renderTargets.getSize(); ++rtIdx)
	{
		slist.pushBackSprintf(
			"\t\"%s\"[color=%s];\n", &descr.m_renderTargets[rtIdx].m_name[0], COLORS[rtIdx % COLORS.getSize()]);
	}
	slist.pushBackSprintf("}\n");
#	endif

	// Barriers
	// slist.pushBackSprintf("subgraph cluster_1 {\n");
	StringRaii prevBubble(&pool);
	prevBubble.create("START");
	for(U32 batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		const Batch& batch = ctx.m_batches[batchIdx];

		StringRaii batchName(&pool);
		batchName.sprintf("batch%u", batchIdx);

		for(U32 barrierIdx = 0; barrierIdx < batch.m_textureBarriersBefore.getSize(); ++barrierIdx)
		{
			const TextureBarrier& barrier = batch.m_textureBarriersBefore[barrierIdx];

			StringRaii barrierLabel(&pool);
			barrierLabel.sprintf("<b>%s</b> (mip,dp,f,l)=(%u,%u,%u,%u)<br/>%s <b>to</b> %s", &descr.m_renderTargets[barrier.m_idx].m_name[0],
								 barrier.m_surface.m_level, barrier.m_surface.m_depth, barrier.m_surface.m_face, barrier.m_surface.m_layer,
								 textureUsageToStr(pool, barrier.m_usageBefore).cstr(), textureUsageToStr(pool, barrier.m_usageAfter).cstr());

			StringRaii barrierName(&pool);
			barrierName.sprintf("%s tex barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(), COLORS[batchIdx % COLORS.getSize()],
								  barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 barrierIdx = 0; barrierIdx < batch.m_bufferBarriersBefore.getSize(); ++barrierIdx)
		{
			const BufferBarrier& barrier = batch.m_bufferBarriersBefore[barrierIdx];

			StringRaii barrierLabel(&pool);
			barrierLabel.sprintf("<b>%s</b><br/>%s <b>to</b> %s", &descr.m_buffers[barrier.m_idx].m_name[0],
								 bufferUsageToStr(pool, barrier.m_usageBefore).cstr(), bufferUsageToStr(pool, barrier.m_usageAfter).cstr());

			StringRaii barrierName(&pool);
			barrierName.sprintf("%s buff barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(), COLORS[batchIdx % COLORS.getSize()],
								  barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 barrierIdx = 0; barrierIdx < batch.m_asBarriersBefore.getSize(); ++barrierIdx)
		{
			const ASBarrier& barrier = batch.m_asBarriersBefore[barrierIdx];

			StringRaii barrierLabel(&pool);
			barrierLabel.sprintf("<b>%s</b><br/>%s <b>to</b> %s", descr.m_as[barrier.m_idx].m_name.getBegin(),
								 asUsageToStr(pool, barrier.m_usageBefore).cstr(), asUsageToStr(pool, barrier.m_usageAfter).cstr());

			StringRaii barrierName(&pool);
			barrierName.sprintf("%s AS barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(), COLORS[batchIdx % COLORS.getSize()],
								  barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 passIdx : batch.m_passIndices)
		{
			const RenderPassBase& pass = *descr.m_passes[passIdx];
			StringRaii passName(&pool);
			passName.sprintf("%s pass", pass.m_name.cstr());
			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold];\n", passName.cstr(), COLORS[batchIdx % COLORS.getSize()]);
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), passName.cstr());

			prevBubble = passName;
		}
	}
	// slist.pushBackSprintf("}\n");

	slist.pushBackSprintf("}");

	File file;
	ANKI_CHECK(file.open(StringRaii(&pool).sprintf("%s/rgraph_%05u.dot", &path[0], m_version).toCString(), FileOpenFlag::kWrite));
	for(const String& s : slist)
	{
		ANKI_CHECK(file.writeTextf("%s", &s[0]));
	}

	return Error::kNone;
}
#endif

} // end namespace anki
