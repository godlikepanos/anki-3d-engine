// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Core/Common.h>

namespace anki {

#define ANKI_DBG_RENDER_GRAPH 0

static inline U32 getTextureSurfOrVolCount(const TexturePtr& tex)
{
	return tex->getMipmapCount() * tex->getLayerCount() * (textureTypeIsCube(tex->getTextureType()) ? 6 : 1);
}

/// Contains some extra things for render targets.
class RenderGraph::RT
{
public:
	DynamicArray<TextureUsageBit, MemoryPoolPtrWrapper<StackMemoryPool>> m_surfOrVolUsages;
	DynamicArray<U16, MemoryPoolPtrWrapper<StackMemoryPool>> m_lastBatchThatTransitionedIt;
	TexturePtr m_texture; ///< Hold a reference.
	Bool m_imported;

	RT(StackMemoryPool* pool)
		: m_surfOrVolUsages(pool)
		, m_lastBatchThatTransitionedIt(pool)
	{
	}
};

/// Same as RT but for buffers.
class RenderGraph::BufferRange
{
public:
	BufferUsageBit m_usage;
	BufferPtr m_buffer; ///< Hold a reference.
	PtrSize m_offset;
	PtrSize m_range;
};

class RenderGraph::AS
{
public:
	AccelerationStructureUsageBit m_usage;
	AccelerationStructurePtr m_as; ///< Hold a reference.
};

/// Pipeline barrier.
class RenderGraph::TextureBarrier
{
public:
	U32 m_idx;
	TextureUsageBit m_usageBefore;
	TextureUsageBit m_usageAfter;
	TextureSubresourceDescriptor m_subresource;

	TextureBarrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSubresourceDescriptor& sub)
		: m_idx(rtIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
		, m_subresource(sub)
	{
	}
};

/// Pipeline barrier.
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

/// Pipeline barrier.
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

/// Contains some extra things the RenderPassBase cannot hold.
class RenderGraph::Pass
{
public:
	// WARNING!!!!!: Whatever you put here needs manual destruction in RenderGraph::reset()
	DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> m_dependsOn;

	DynamicArray<RenderPassDependency::TextureInfo, MemoryPoolPtrWrapper<StackMemoryPool>> m_consumedTextures;

	Function<void(RenderPassWorkContext&), MemoryPoolPtrWrapper<StackMemoryPool>> m_callback;

	class
	{
	public:
		Array<RenderTarget, kMaxColorRenderTargets> m_colorRts;
		RenderTarget m_dsRt;
		TextureView m_vrsRt;
		Array<U32, 4> m_renderArea = {};
		U8 m_colorRtCount = 0;
		U8 m_vrsTexelSizeX = 0;
		U8 m_vrsTexelSizeY = 0;

		Array<TexturePtr, kMaxColorRenderTargets + 2> m_refs;

		Bool hasRenderpass() const
		{
			return m_renderArea[3] != 0;
		}
	} m_beginRenderpassInfo;

	BaseString<MemoryPoolPtrWrapper<StackMemoryPool>> m_name;

	U32 m_batchIdx ANKI_DEBUG_CODE(= kMaxU32);
	Bool m_drawsToPresentable = false;

	Pass(StackMemoryPool* pool)
		: m_dependsOn(pool)
		, m_consumedTextures(pool)
		, m_name(pool)
	{
	}
};

/// A batch of render passes. These passes can run in parallel.
/// @warning It's POD. Destructor won't be called.
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

/// The RenderGraph build context.
class RenderGraph::BakeContext
{
public:
	DynamicArray<Pass, MemoryPoolPtrWrapper<StackMemoryPool>> m_passes;
	BitSet<kMaxRenderGraphPasses, U64> m_passIsInBatch{false};
	DynamicArray<Batch, MemoryPoolPtrWrapper<StackMemoryPool>> m_batches;
	DynamicArray<RT, MemoryPoolPtrWrapper<StackMemoryPool>> m_rts;
	DynamicArray<BufferRange, MemoryPoolPtrWrapper<StackMemoryPool>> m_buffers;
	DynamicArray<AS, MemoryPoolPtrWrapper<StackMemoryPool>> m_as;

	Bool m_gatherStatistics = false;

	BakeContext(StackMemoryPool* pool)
		: m_passes(pool)
		, m_batches(pool)
		, m_rts(pool)
		, m_buffers(pool)
		, m_as(pool)
	{
	}
};

RenderGraph::RenderGraph(CString name)
	: GrObject(kClassType, name)
{
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
		p.m_beginRenderpassInfo.m_refs.fill(TexturePtr(nullptr));
		p.m_callback.destroy();
		p.m_name.destroy();
	}

	m_ctx = nullptr;
	++m_version;
}

TexturePtr RenderGraph::getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash)
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
	TexturePtr tex;
	const Bool createNewTex = entry->m_textures.getSize() == entry->m_texturesInUse;
	if(!createNewTex)
	{
		// Pop

		tex = entry->m_textures[entry->m_texturesInUse++];
	}
	else
	{
		// Create it

		tex = GrManager::getSingleton().newTexture(initInf);

		ANKI_ASSERT(entry->m_texturesInUse == entry->m_textures.getSize());
		entry->m_textures.resize(entry->m_textures.getSize() + 1);
		entry->m_textures[entry->m_textures.getSize() - 1] = tex;
		++entry->m_texturesInUse;
	}

	return tex;
}

Bool RenderGraph::passADependsOnB(const RenderPassDescriptionBase& a, const RenderPassDescriptionBase& b)
{
	// Render targets
	{
		// Compute the 3 types of dependencies
		const BitSet<kMaxRenderGraphRenderTargets, U64> aReadBWrite = a.m_readRtMask & b.m_writeRtMask;
		const BitSet<kMaxRenderGraphRenderTargets, U64> aWriteBRead = a.m_writeRtMask & b.m_readRtMask;
		const BitSet<kMaxRenderGraphRenderTargets, U64> aWriteBWrite = a.m_writeRtMask & b.m_writeRtMask;

		const BitSet<kMaxRenderGraphRenderTargets, U64> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

		if(fullDep.getAnySet())
		{
			// There might be an overlap

			for(const RenderPassDependency& aDep : a.m_rtDeps)
			{
				if(!fullDep.get(aDep.m_texture.m_handle.m_idx))
				{
					continue;
				}

				for(const RenderPassDependency& bDep : b.m_rtDeps)
				{
					if(aDep.m_texture.m_handle != bDep.m_texture.m_handle)
					{
						continue;
					}

					if(!((aDep.m_texture.m_usage | bDep.m_texture.m_usage) & TextureUsageBit::kAllWrite))
					{
						// Don't care about read to read deps
						continue;
					}

					if(aDep.m_texture.m_subresource.overlapsWith(bDep.m_texture.m_subresource))
					{
						return true;
					}
				}
			}
		}
	}

	// Buffers
	if(a.m_readBuffMask || a.m_writeBuffMask)
	{
		const BitSet<kMaxRenderGraphBuffers, U64> aReadBWrite = a.m_readBuffMask & b.m_writeBuffMask;
		const BitSet<kMaxRenderGraphBuffers, U64> aWriteBRead = a.m_writeBuffMask & b.m_readBuffMask;
		const BitSet<kMaxRenderGraphBuffers, U64> aWriteBWrite = a.m_writeBuffMask & b.m_writeBuffMask;

		const BitSet<kMaxRenderGraphBuffers, U64> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

		if(fullDep.getAnySet())
		{
			// There might be an overlap

			for(const RenderPassDependency& aDep : a.m_buffDeps)
			{
				if(!fullDep.get(aDep.m_buffer.m_handle.m_idx))
				{
					continue;
				}

				for(const RenderPassDependency& bDep : b.m_buffDeps)
				{
					if(aDep.m_buffer.m_handle != bDep.m_buffer.m_handle)
					{
						continue;
					}

					if(!((aDep.m_buffer.m_usage | bDep.m_buffer.m_usage) & BufferUsageBit::kAllWrite))
					{
						// Don't care about read to read deps
						continue;
					}

					// TODO: Take into account the ranges
					return true;
				}
			}
		}
	}

	// AS
	if(a.m_readAsMask || a.m_writeAsMask)
	{
		const BitSet<kMaxRenderGraphAccelerationStructures, U32> aReadBWrite = a.m_readAsMask & b.m_writeAsMask;
		const BitSet<kMaxRenderGraphAccelerationStructures, U32> aWriteBRead = a.m_writeAsMask & b.m_readAsMask;
		const BitSet<kMaxRenderGraphAccelerationStructures, U32> aWriteBWrite = a.m_writeAsMask & b.m_writeAsMask;

		const BitSet<kMaxRenderGraphAccelerationStructures, U32> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

		if(fullDep)
		{
			for(const RenderPassDependency& aDep : a.m_asDeps)
			{
				if(!fullDep.get(aDep.m_as.m_handle.m_idx))
				{
					continue;
				}

				for(const RenderPassDependency& bDep : b.m_asDeps)
				{
					if(aDep.m_as.m_handle != bDep.m_as.m_handle)
					{
						continue;
					}

					if(!((aDep.m_as.m_usage | bDep.m_as.m_usage) & AccelerationStructureUsageBit::kAllWrite))
					{
						// Don't care about read to read deps
						continue;
					}

					return true;
				}
			}
		}
	}

	return false;
}

Bool RenderGraph::passHasUnmetDependencies(const BakeContext& ctx, U32 passIdx)
{
	Bool depends = false;

	if(ctx.m_batches.getSize() > 0)
	{
		// Check if the deps of passIdx are all in a batch

		for(const U32 depPassIdx : ctx.m_passes[passIdx].m_dependsOn)
		{
			if(!ctx.m_passIsInBatch.get(depPassIdx))
			{
				// Dependency pass is not in a batch
				depends = true;
				break;
			}
		}
	}
	else
	{
		// First batch, check if passIdx depends on any pass

		depends = ctx.m_passes[passIdx].m_dependsOn.getSize() != 0;
	}

	return depends;
}

RenderGraph::BakeContext* RenderGraph::newContext(const RenderGraphDescription& descr, StackMemoryPool& pool)
{
	// Allocate
	BakeContext* ctx = anki::newInstance<BakeContext>(pool, &pool);

	// Init the resources
	ctx->m_rts.resizeStorage(descr.m_renderTargets.getSize());
	for(U32 rtIdx = 0; rtIdx < descr.m_renderTargets.getSize(); ++rtIdx)
	{
		RT& outRt = *ctx->m_rts.emplaceBack(&pool);
		const RenderGraphDescription::RT& inRt = descr.m_renderTargets[rtIdx];

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
		outRt.m_surfOrVolUsages.resize(surfOrVolumeCount, TextureUsageBit::kNone);
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
	ctx->m_buffers.resize(descr.m_buffers.getSize());
	for(U32 buffIdx = 0; buffIdx < ctx->m_buffers.getSize(); ++buffIdx)
	{
		ctx->m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
		ANKI_ASSERT(descr.m_buffers[buffIdx].m_importedBuff.isCreated());
		ctx->m_buffers[buffIdx].m_buffer = descr.m_buffers[buffIdx].m_importedBuff;
		ctx->m_buffers[buffIdx].m_offset = descr.m_buffers[buffIdx].m_offset;
		ctx->m_buffers[buffIdx].m_range = descr.m_buffers[buffIdx].m_range;
	}

	// AS
	ctx->m_as.resize(descr.m_as.getSize());
	for(U32 i = 0; i < descr.m_as.getSize(); ++i)
	{
		ctx->m_as[i].m_usage = descr.m_as[i].m_usage;
		ctx->m_as[i].m_as = descr.m_as[i].m_importedAs;
		ANKI_ASSERT(ctx->m_as[i].m_as.isCreated());
	}

	ctx->m_gatherStatistics = descr.m_gatherStatistics;

	return ctx;
}

void RenderGraph::initRenderPassesAndSetDeps(const RenderGraphDescription& descr)
{
	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	ctx.m_passes.resizeStorage(passCount);
	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassDescriptionBase& inPass = *descr.m_passes[passIdx];
		Pass& outPass = *ctx.m_passes.emplaceBack(ctx.m_as.getMemoryPool().m_pool);

		outPass.m_callback = inPass.m_callback;
		outPass.m_name = inPass.m_name;

		// Create consumer info
		outPass.m_consumedTextures.resize(inPass.m_rtDeps.getSize());
		for(U32 depIdx = 0; depIdx < inPass.m_rtDeps.getSize(); ++depIdx)
		{
			const RenderPassDependency& inDep = inPass.m_rtDeps[depIdx];
			ANKI_ASSERT(inDep.m_type == RenderPassDependency::Type::kTexture);

			RenderPassDependency::TextureInfo& inf = outPass.m_consumedTextures[depIdx];

			ANKI_ASSERT(sizeof(inf) == sizeof(inDep.m_texture));
			memcpy(&inf, &inDep.m_texture, sizeof(inf));
		}

		// Set dependencies by checking all previous subpasses.
		U32 prevPassIdx = passIdx;
		while(prevPassIdx--)
		{
			const RenderPassDescriptionBase& prevPass = *descr.m_passes[prevPassIdx];

			if(passADependsOnB(inPass, prevPass))
			{
				outPass.m_dependsOn.emplaceBack(prevPassIdx);
			}
		}
	}
}

void RenderGraph::initBatches()
{
	ANKI_ASSERT(m_ctx);

	U passesAssignedToBatchCount = 0;
	const U passCount = m_ctx->m_passes.getSize();
	ANKI_ASSERT(passCount > 0);
	while(passesAssignedToBatchCount < passCount)
	{
		Batch batch(m_ctx->m_as.getMemoryPool().m_pool);

		for(U32 i = 0; i < passCount; ++i)
		{
			if(!m_ctx->m_passIsInBatch.get(i) && !passHasUnmetDependencies(*m_ctx, i))
			{
				// Add to the batch
				++passesAssignedToBatchCount;
				batch.m_passIndices.emplaceBack(i);
			}
		}

		// Mark batch's passes done
		for(U32 passIdx : batch.m_passIndices)
		{
			m_ctx->m_passIsInBatch.set(passIdx);
			m_ctx->m_passes[passIdx].m_batchIdx = m_ctx->m_batches.getSize();
		}

		m_ctx->m_batches.emplaceBack(std::move(batch));
	}
}

void RenderGraph::initGraphicsPasses(const RenderGraphDescription& descr)
{
	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassDescriptionBase& baseInPass = *descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];

		// Create command buffers and framebuffer
		if(baseInPass.m_type == RenderPassDescriptionBase::Type::kGraphics)
		{
			const GraphicsRenderPassDescription& inPass = static_cast<const GraphicsRenderPassDescription&>(baseInPass);

			if(inPass.hasRenderpass())
			{
				outPass.m_beginRenderpassInfo.m_renderArea = inPass.m_rpassRenderArea;
				outPass.m_beginRenderpassInfo.m_colorRtCount = inPass.m_colorRtCount;

				// Init the usage bits
				for(U32 i = 0; i < inPass.m_colorRtCount; ++i)
				{
					const RenderTargetInfo& inAttachment = inPass.m_rts[i];
					RenderTarget& outAttachment = outPass.m_beginRenderpassInfo.m_colorRts[i];

					getCrntUsage(inAttachment.m_handle, outPass.m_batchIdx, inAttachment.m_subresource, outAttachment.m_usage);

					outAttachment.m_textureView = TextureView(m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture.get(), inAttachment.m_subresource);
					outPass.m_beginRenderpassInfo.m_refs[i] = m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture;

					outAttachment.m_loadOperation = inAttachment.m_loadOperation;
					outAttachment.m_storeOperation = inAttachment.m_storeOperation;
					outAttachment.m_clearValue = inAttachment.m_clearValue;
				}

				if(!!inPass.m_rts[kMaxColorRenderTargets].m_subresource.m_depthStencilAspect)
				{
					const RenderTargetInfo& inAttachment = inPass.m_rts[kMaxColorRenderTargets];
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

				if(inPass.m_vrsRtTexelSizeX > 0)
				{
					const RenderTargetInfo& inAttachment = inPass.m_rts[kMaxColorRenderTargets + 1];

					outPass.m_beginRenderpassInfo.m_vrsRt =
						TextureView(m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture.get(), inAttachment.m_subresource);
					outPass.m_beginRenderpassInfo.m_refs[kMaxColorRenderTargets + 1] = m_ctx->m_rts[inAttachment.m_handle.m_idx].m_texture;

					outPass.m_beginRenderpassInfo.m_vrsTexelSizeX = inPass.m_vrsRtTexelSizeX;
					outPass.m_beginRenderpassInfo.m_vrsTexelSizeY = inPass.m_vrsRtTexelSizeY;
				}
			}
		}
	}
}

template<typename TFunc>
void RenderGraph::iterateSurfsOrVolumes(const Texture& tex, const TextureSubresourceDescriptor& subresource, TFunc func)
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

					if(!func(idx, TextureSubresourceDescriptor::surface(mip, face, layer, subresource.m_depthStencilAspect)))
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

void RenderGraph::setTextureBarrier(Batch& batch, const RenderPassDependency& dep)
{
	ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::kTexture);

	BakeContext& ctx = *m_ctx;
	const U32 batchIdx = U32(&batch - &ctx.m_batches[0]);
	const U32 rtIdx = dep.m_texture.m_handle.m_idx;
	const TextureUsageBit depUsage = dep.m_texture.m_usage;
	RT& rt = ctx.m_rts[rtIdx];

	iterateSurfsOrVolumes(*rt.m_texture, dep.m_texture.m_subresource, [&](U32 surfOrVolIdx, const TextureSubresourceDescriptor& subresource) {
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

void RenderGraph::setBatchBarriers(const RenderGraphDescription& descr)
{
	BakeContext& ctx = *m_ctx;

	// For all batches
	for(Batch& batch : ctx.m_batches)
	{
		BitSet<kMaxRenderGraphBuffers, U64> buffHasBarrierMask(false);
		BitSet<kMaxRenderGraphAccelerationStructures, U32> asHasBarrierMask(false);

		// For all passes of that batch
		for(U32 passIdx : batch.m_passIndices)
		{
			const RenderPassDescriptionBase& pass = *descr.m_passes[passIdx];

			// Do textures
			for(const RenderPassDependency& dep : pass.m_rtDeps)
			{
				setTextureBarrier(batch, dep);
			}

			// Do buffers
			for(const RenderPassDependency& dep : pass.m_buffDeps)
			{
				const U32 buffIdx = dep.m_buffer.m_handle.m_idx;
				const BufferUsageBit depUsage = dep.m_buffer.m_usage;
				BufferUsageBit& crntUsage = ctx.m_buffers[buffIdx].m_usage;

				const Bool skipBarrier = crntUsage == depUsage && !(crntUsage & BufferUsageBit::kAllWrite);
				if(skipBarrier)
				{
					continue;
				}

				const Bool buffHasBarrier = buffHasBarrierMask.get(buffIdx);

				if(!buffHasBarrier)
				{
					// Buff hasn't had a barrier in this batch, add a new barrier

					batch.m_bufferBarriersBefore.emplaceBack(buffIdx, crntUsage, depUsage);

					crntUsage = depUsage;
					buffHasBarrierMask.set(buffIdx);
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
			for(const RenderPassDependency& dep : pass.m_asDeps)
			{
				const U32 asIdx = dep.m_as.m_handle.m_idx;
				const AccelerationStructureUsageBit depUsage = dep.m_as.m_usage;
				AccelerationStructureUsageBit& crntUsage = ctx.m_as[asIdx].m_usage;

				const Bool skipBarrier = crntUsage == depUsage && !(crntUsage & AccelerationStructureUsageBit::kAllWrite);
				if(skipBarrier)
				{
					continue;
				}

				const Bool asHasBarrierInThisBatch = asHasBarrierMask.get(asIdx);
				if(!asHasBarrierInThisBatch)
				{
					// AS doesn't have a barrier in this batch, create a new one

					batch.m_asBarriersBefore.emplaceBack(asIdx, crntUsage, depUsage);
					crntUsage = depUsage;
					asHasBarrierMask.set(asIdx);
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
			const Bool aIsCompute = !ctx.m_passes[a].m_beginRenderpassInfo.hasRenderpass();
			const Bool bIsCompute = !ctx.m_passes[b].m_beginRenderpassInfo.hasRenderpass();

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
			const Bool aIsCompute = !ctx.m_passes[a].m_beginRenderpassInfo.hasRenderpass();
			const Bool bIsCompute = !ctx.m_passes[b].m_beginRenderpassInfo.hasRenderpass();

			return aIsCompute < bIsCompute;
		});
	}
}

void RenderGraph::compileNewGraph(const RenderGraphDescription& descr, StackMemoryPool& pool)
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
	StackMemoryPool* pool = m_ctx->m_rts.getMemoryPool().m_pool;

	DynamicArray<CommandBufferPtr, MemoryPoolPtrWrapper<StackMemoryPool>> cmdbs(pool);
	cmdbs.resize(batchGroupCount);
	SpinLock cmdbsMtx;

	for(U32 group = 0; group < batchGroupCount; ++group)
	{
		U32 start, end;
		splitThreadedProblem(group, batchGroupCount, m_ctx->m_batches.getSize(), start, end);

		if(start == end)
		{
			continue;
		}

		CoreThreadJobManager::getSingleton().dispatchTask(
			[this, start, end, pool, &cmdbs, &cmdbsMtx, group, batchGroupCount]([[maybe_unused]] U32 tid) {
				ANKI_TRACE_SCOPED_EVENT(GrRenderGraphTask);

				CommandBufferInitInfo cmdbInit("RenderGraph cmdb");
				cmdbInit.m_flags = CommandBufferFlag::kGeneralWork;
				CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

				// Write timestamp
				const Bool setPreQuery = m_ctx->m_gatherStatistics && group == 0;
				const Bool setPostQuery = m_ctx->m_gatherStatistics && group == batchGroupCount - 1;
				TimestampQueryPtr preQuery, postQuery;
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
						m_statistics.m_nextTimestamp = (m_statistics.m_nextTimestamp + 1) % kMaxBufferedTimestamps;
						m_statistics.m_timestamps[m_statistics.m_nextTimestamp * 2] = preQuery;
					}

					if(postQuery.isCreated())
					{
						m_statistics.m_timestamps[m_statistics.m_nextTimestamp * 2 + 1] = postQuery;
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

						const Vec3 passColor = (pass.m_beginRenderpassInfo.hasRenderpass()) ? Vec3(0.0f, 1.0f, 0.0f) : Vec3(1.0f, 1.0f, 0.0f);
						cmdb->pushDebugMarker(pass.m_name, passColor);

						if(pass.m_beginRenderpassInfo.hasRenderpass())
						{
							cmdb->beginRenderPass({pass.m_beginRenderpassInfo.m_colorRts.getBegin(), U32(pass.m_beginRenderpassInfo.m_colorRtCount)},
												  pass.m_beginRenderpassInfo.m_dsRt.m_textureView.isValid() ? &pass.m_beginRenderpassInfo.m_dsRt
																											: nullptr,
												  pass.m_beginRenderpassInfo.m_renderArea[0], pass.m_beginRenderpassInfo.m_renderArea[1],
												  pass.m_beginRenderpassInfo.m_renderArea[2], pass.m_beginRenderpassInfo.m_renderArea[3],
												  pass.m_beginRenderpassInfo.m_vrsRt, pass.m_beginRenderpassInfo.m_vrsTexelSizeX,
												  pass.m_beginRenderpassInfo.m_vrsTexelSizeY);
						}

						{
							ANKI_TRACE_SCOPED_EVENT(GrRenderGraphCallback);
							ctx.m_passIdx = passIdx;
							pass.m_callback(ctx);
						}

						if(pass.m_beginRenderpassInfo.hasRenderpass())
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
	if(cmdbs.getSize() == 1) [[unlikely]]
	{
		GrManager::getSingleton().submit(cmdbs[0].get(), {}, optionalFence);
	}
	else
	{
		// 2 submits. The 1st contains all the batches minus the last. Then the last batch is alone given that it most likely it writes to the
		// swapchain

		DynamicArray<CommandBuffer*, MemoryPoolPtrWrapper<StackMemoryPool>> pCmdbs(pool);
		pCmdbs.resize(cmdbs.getSize() - 1);
		for(U32 i = 0; i < cmdbs.getSize() - 1; ++i)
		{
			pCmdbs[i] = cmdbs[i].get();
		}

		GrManager::getSingleton().submit(WeakArray(pCmdbs), {}, nullptr);
		GrManager::getSingleton().submit(cmdbs.getBack().get(), {}, optionalFence);
	}
}

void RenderGraph::getCrntUsage(RenderTargetHandle handle, U32 batchIdx, const TextureSubresourceDescriptor& subresource, TextureUsageBit& usage) const
{
	usage = TextureUsageBit::kNone;
	const Batch& batch = m_ctx->m_batches[batchIdx];

	for(U32 passIdx : batch.m_passIndices)
	{
		for(const RenderPassDependency::TextureInfo& consumer : m_ctx->m_passes[passIdx].m_consumedTextures)
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
			GrDynamicArray<TexturePtr> newArray;
			if(entry.m_texturesInUse > 0)
			{
				newArray.resize(entry.m_texturesInUse);
			}

			// Populate the new array
			for(U32 i = 0; i < newArray.getSize(); ++i)
			{
				newArray[i] = std::move(entry.m_textures[i]);
			}

			// Destroy the old array and the rest of the textures
			entry.m_textures.destroy();

			// Move new array
			entry.m_textures = std::move(newArray);
		}
	}

	if(rtsCleanedCount > 0)
	{
		ANKI_GR_LOGI("Cleaned %u render targets", rtsCleanedCount);
	}
}

void RenderGraph::getStatistics(RenderGraphStatistics& statistics) const
{
	const U32 oldFrame = (m_statistics.m_nextTimestamp + 1) % kMaxBufferedTimestamps;

	if(m_statistics.m_timestamps[oldFrame * 2] && m_statistics.m_timestamps[oldFrame * 2 + 1])
	{
		Second start, end;
		[[maybe_unused]] TimestampQueryResult res = m_statistics.m_timestamps[oldFrame * 2]->getResult(start);
		ANKI_ASSERT(res == TimestampQueryResult::kAvailable);

		res = m_statistics.m_timestamps[oldFrame * 2 + 1]->getResult(end);
		ANKI_ASSERT(res == TimestampQueryResult::kAvailable);

		const Second diff = end - start;
		statistics.m_gpuTime = diff;
		statistics.m_cpuStartTime = m_statistics.m_cpuStartTimes[oldFrame];
	}
	else
	{
		statistics.m_gpuTime = -1.0;
		statistics.m_cpuStartTime = -1.0;
	}
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
	ANKI_BUFF_USAGE(kConstantFragment);
	ANKI_BUFF_USAGE(kConstantCompute);
	ANKI_BUFF_USAGE(kConstantTraceRays);
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
	ANKI_BUFF_USAGE(kIndirectTraceRays);
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

Error RenderGraph::dumpDependencyDotFile(const RenderGraphDescription& descr, const BakeContext& ctx, CString path) const
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
								  (descr.m_passes[passIdx]->m_type == RenderPassDescriptionBase::Type::kGraphics) ? "bold" : "dashed");

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
			const RenderPassDescriptionBase& pass = *descr.m_passes[passIdx];
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
