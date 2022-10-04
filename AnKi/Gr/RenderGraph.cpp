// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/HighRezTimer.h>

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
	DynamicArray<TextureUsageBit> m_surfOrVolUsages;
	DynamicArray<U16> m_lastBatchThatTransitionedIt;
	TexturePtr m_texture; ///< Hold a reference.
	Bool m_imported;
};

/// Same as RT but for buffers.
class RenderGraph::Buffer
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
	TextureSurfaceInfo m_surface;
	DepthStencilAspectBit m_dsAspect;

	TextureBarrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSurfaceInfo& surf,
				   DepthStencilAspectBit dsAspect)
		: m_idx(rtIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
		, m_surface(surf)
		, m_dsAspect(dsAspect)
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
	DynamicArray<U32> m_dependsOn;

	DynamicArray<RenderPassDependency::TextureInfo> m_consumedTextures;

	Function<void(RenderPassWorkContext&)> m_callback;

	DynamicArray<CommandBufferPtr> m_secondLevelCmdbs;
	/// Will reuse the m_secondLevelCmdbInitInfo.m_framebuffer to get the framebuffer.
	CommandBufferInitInfo m_secondLevelCmdbInitInfo;
	Array<U32, 4> m_fbRenderArea;
	Array<TextureUsageBit, kMaxColorRenderTargets> m_colorUsages = {}; ///< For beginRender pass
	TextureUsageBit m_dsUsage = TextureUsageBit::kNone; ///< For beginRender pass

	U32 m_batchIdx ANKI_DEBUG_CODE(= kMaxU32);
	Bool m_drawsToPresentable = false;

	FramebufferPtr& fb()
	{
		return m_secondLevelCmdbInitInfo.m_framebuffer;
	}

	const FramebufferPtr& fb() const
	{
		return m_secondLevelCmdbInitInfo.m_framebuffer;
	}
};

/// A batch of render passes. These passes can run in parallel.
/// @warning It's POD. Destructor won't be called.
class RenderGraph::Batch
{
public:
	DynamicArray<U32> m_passIndices;
	DynamicArray<TextureBarrier> m_textureBarriersBefore;
	DynamicArray<BufferBarrier> m_bufferBarriersBefore;
	DynamicArray<ASBarrier> m_asBarriersBefore;
	CommandBuffer* m_cmdb; ///< Someone else holds the ref already so have a ptr here.
};

/// The RenderGraph build context.
class RenderGraph::BakeContext
{
public:
	StackMemoryPool* m_pool = nullptr;
	DynamicArray<Pass> m_passes;
	BitSet<kMaxRenderGraphPasses, U64> m_passIsInBatch{false};
	DynamicArray<Batch> m_batches;
	DynamicArray<RT> m_rts;
	DynamicArray<Buffer> m_buffers;
	DynamicArray<AS> m_as;

	DynamicArray<CommandBufferPtr> m_graphicsCmdbs;

	Bool m_gatherStatistics = false;

	BakeContext(StackMemoryPool* pool)
		: m_pool(pool)
	{
	}
};

void FramebufferDescription::bake()
{
	m_hash = 0;
	ANKI_ASSERT(m_colorAttachmentCount > 0 || !!m_depthStencilAttachment.m_aspect);

	// First the depth attachments
	if(m_colorAttachmentCount)
	{
		ANKI_BEGIN_PACKED_STRUCT
		struct ColorAttachment
		{
			TextureSurfaceInfo m_surf;
			U32 m_loadOp;
			U32 m_storeOp;
			Array<U32, 4> m_clearColor;
		};
		ANKI_END_PACKED_STRUCT
		static_assert(sizeof(ColorAttachment) == 4 * (4 + 1 + 1 + 4), "Wrong size");

		Array<ColorAttachment, kMaxColorRenderTargets> colorAttachments;
		for(U i = 0; i < m_colorAttachmentCount; ++i)
		{
			const FramebufferDescriptionAttachment& inAtt = m_colorAttachments[i];
			colorAttachments[i].m_surf = inAtt.m_surface;
			colorAttachments[i].m_loadOp = static_cast<U32>(inAtt.m_loadOperation);
			colorAttachments[i].m_storeOp = static_cast<U32>(inAtt.m_storeOperation);
			memcpy(&colorAttachments[i].m_clearColor[0], &inAtt.m_clearValue.m_coloru[0], sizeof(U32) * 4);
		}

		m_hash = computeHash(&colorAttachments[0], sizeof(ColorAttachment) * m_colorAttachmentCount);
	}

	// DS attachment
	if(!!m_depthStencilAttachment.m_aspect)
	{
		ANKI_BEGIN_PACKED_STRUCT
		class DSAttachment
		{
		public:
			TextureSurfaceInfo m_surf;
			U32 m_loadOp;
			U32 m_storeOp;
			U32 m_stencilLoadOp;
			U32 m_stencilStoreOp;
			U32 m_aspect;
			F32 m_depthClear;
			I32 m_stencilClear;
		} outAtt;
		ANKI_END_PACKED_STRUCT

		const FramebufferDescriptionAttachment& inAtt = m_depthStencilAttachment;
		const Bool hasDepth = !!(inAtt.m_aspect & DepthStencilAspectBit::kDepth);
		const Bool hasStencil = !!(inAtt.m_aspect & DepthStencilAspectBit::kStencil);

		outAtt.m_surf = inAtt.m_surface;
		outAtt.m_loadOp = (hasDepth) ? static_cast<U32>(inAtt.m_loadOperation) : 0;
		outAtt.m_storeOp = (hasDepth) ? static_cast<U32>(inAtt.m_storeOperation) : 0;
		outAtt.m_stencilLoadOp = (hasStencil) ? static_cast<U32>(inAtt.m_stencilLoadOperation) : 0;
		outAtt.m_stencilStoreOp = (hasStencil) ? static_cast<U32>(inAtt.m_stencilStoreOperation) : 0;
		outAtt.m_aspect = static_cast<U32>(inAtt.m_aspect);
		outAtt.m_depthClear = (hasDepth) ? inAtt.m_clearValue.m_depthStencil.m_depth : 0.0f;
		outAtt.m_stencilClear = (hasStencil) ? inAtt.m_clearValue.m_depthStencil.m_stencil : 0;

		m_hash = (m_hash != 0) ? appendHash(&outAtt, sizeof(outAtt), m_hash) : computeHash(&outAtt, sizeof(outAtt));
	}

	// SRI
	if(m_shadingRateAttachmentTexelWidth > 0 && m_shadingRateAttachmentTexelHeight > 0)
	{
		ANKI_BEGIN_PACKED_STRUCT
		class SriToHash
		{
		public:
			U32 m_sriTexelWidth;
			U32 m_sriTexelHeight;
			TextureSurfaceInfo m_surface;
		} sriToHash;
		ANKI_END_PACKED_STRUCT

		sriToHash.m_sriTexelWidth = m_shadingRateAttachmentTexelWidth;
		sriToHash.m_sriTexelHeight = m_shadingRateAttachmentTexelHeight;
		sriToHash.m_surface = m_shadingRateAttachmentSurface;

		m_hash = (m_hash != 0) ? appendHash(&sriToHash, sizeof(sriToHash), m_hash)
							   : computeHash(&sriToHash, sizeof(sriToHash));
	}

	ANKI_ASSERT(m_hash != 0 && m_hash != 1);
}

RenderGraph::RenderGraph(GrManager* manager, CString name)
	: GrObject(manager, kClassType, name)
{
	ANKI_ASSERT(manager);
}

RenderGraph::~RenderGraph()
{
	ANKI_ASSERT(m_ctx == nullptr);

	while(!m_renderTargetCache.isEmpty())
	{
		auto it = m_renderTargetCache.getBegin();
		RenderTargetCacheEntry& entry = *it;
		entry.m_textures.destroy(getMemoryPool());
		m_renderTargetCache.erase(getMemoryPool(), it);
	}

	m_fbCache.destroy(getMemoryPool());

	for(auto& it : m_importedRenderTargets)
	{
		it.m_surfOrVolLastUsages.destroy(getMemoryPool());
	}

	m_importedRenderTargets.destroy(getMemoryPool());
}

RenderGraph* RenderGraph::newInstance(GrManager* manager)
{
	return anki::newInstance<RenderGraph>(manager->getMemoryPool(), manager, "N/A");
}

void RenderGraph::reset()
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_RESET);

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
				it = m_importedRenderTargets.emplace(getMemoryPool(), hash);
				it->m_surfOrVolLastUsages.create(getMemoryPool(), surfOrVolumeCount);
			}

			// Update the usage
			for(U32 surfOrVolIdx = 0; surfOrVolIdx < surfOrVolumeCount; ++surfOrVolIdx)
			{
				it->m_surfOrVolLastUsages[surfOrVolIdx] = rt.m_surfOrVolUsages[surfOrVolIdx];
			}
		}

		rt.m_texture.reset(nullptr);
	}

	for(Buffer& buff : m_ctx->m_buffers)
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
		p.fb().reset(nullptr);
		p.m_secondLevelCmdbs.destroy(*m_ctx->m_pool);
		p.m_callback.destroy(*m_ctx->m_pool);
	}

	m_ctx->m_graphicsCmdbs.destroy(*m_ctx->m_pool);

	m_ctx = nullptr;
	++m_version;
}

TexturePtr RenderGraph::getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash)
{
	ANKI_ASSERT(hash);

	// Find a cache entry
	RenderTargetCacheEntry* entry = nullptr;
	auto it = m_renderTargetCache.find(hash);
	if(ANKI_UNLIKELY(it == m_renderTargetCache.getEnd()))
	{
		// Didn't found the entry, create a new one

		auto it2 = m_renderTargetCache.emplace(getMemoryPool(), hash);
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

		tex = getManager().newTexture(initInf);

		ANKI_ASSERT(entry->m_texturesInUse == entry->m_textures.getSize());
		entry->m_textures.resize(getMemoryPool(), entry->m_textures.getSize() + 1);
		entry->m_textures[entry->m_textures.getSize() - 1] = tex;
		++entry->m_texturesInUse;
	}

	return tex;
}

FramebufferPtr RenderGraph::getOrCreateFramebuffer(const FramebufferDescription& fbDescr,
												   const RenderTargetHandle* rtHandles, CString name,
												   Bool& drawsToPresentable)
{
	ANKI_ASSERT(rtHandles);
	U64 hash = fbDescr.m_hash;
	ANKI_ASSERT(hash > 0);

	drawsToPresentable = false;

	// Create a hash that includes the render targets
	Array<U64, kMaxColorRenderTargets + 2> uuids;
	U count = 0;
	for(U i = 0; i < fbDescr.m_colorAttachmentCount; ++i)
	{
		uuids[count++] = m_ctx->m_rts[rtHandles[i].m_idx].m_texture->getUuid();

		if(!!(m_ctx->m_rts[rtHandles[i].m_idx].m_texture->getTextureUsage() & TextureUsageBit::kPresent))
		{
			drawsToPresentable = true;
		}
	}

	if(!!fbDescr.m_depthStencilAttachment.m_aspect)
	{
		uuids[count++] = m_ctx->m_rts[rtHandles[kMaxColorRenderTargets].m_idx].m_texture->getUuid();
	}

	if(fbDescr.m_shadingRateAttachmentTexelWidth > 0)
	{
		uuids[count++] = m_ctx->m_rts[rtHandles[kMaxColorRenderTargets + 1].m_idx].m_texture->getUuid();
	}

	hash = appendHash(&uuids[0], sizeof(U64) * count, hash);

	// Hash the name of the pass. If you don't the code bellow may fetch an FB with some another name and that will
	// cause problems with tools. The FB name is used as a debug marker
	hash = appendHash(name.cstr(), name.getLength(), hash);

	FramebufferPtr fb;
	auto it = m_fbCache.find(hash);
	if(it != m_fbCache.getEnd())
	{
		fb = *it;
	}
	else
	{
		// Create a complete fb init info
		FramebufferInitInfo fbInit;
		fbInit.m_colorAttachmentCount = fbDescr.m_colorAttachmentCount;
		for(U i = 0; i < fbInit.m_colorAttachmentCount; ++i)
		{
			FramebufferAttachmentInfo& outAtt = fbInit.m_colorAttachments[i];
			const FramebufferDescriptionAttachment& inAtt = fbDescr.m_colorAttachments[i];

			outAtt.m_clearValue = inAtt.m_clearValue;
			outAtt.m_loadOperation = inAtt.m_loadOperation;
			outAtt.m_storeOperation = inAtt.m_storeOperation;

			// Create texture view
			TextureViewInitInfo viewInit(m_ctx->m_rts[rtHandles[i].m_idx].m_texture,
										 TextureSubresourceInfo(inAtt.m_surface), "RenderGraph");
			TextureViewPtr view = getManager().newTextureView(viewInit);

			outAtt.m_textureView = std::move(view);
		}

		if(!!fbDescr.m_depthStencilAttachment.m_aspect)
		{
			FramebufferAttachmentInfo& outAtt = fbInit.m_depthStencilAttachment;
			const FramebufferDescriptionAttachment& inAtt = fbDescr.m_depthStencilAttachment;

			outAtt.m_clearValue = inAtt.m_clearValue;
			outAtt.m_loadOperation = inAtt.m_loadOperation;
			outAtt.m_storeOperation = inAtt.m_storeOperation;
			outAtt.m_stencilLoadOperation = inAtt.m_stencilLoadOperation;
			outAtt.m_stencilStoreOperation = inAtt.m_stencilStoreOperation;

			// Create texture view
			TextureViewInitInfo viewInit(m_ctx->m_rts[rtHandles[kMaxColorRenderTargets].m_idx].m_texture,
										 TextureSubresourceInfo(inAtt.m_surface, inAtt.m_aspect), "RenderGraph");
			TextureViewPtr view = getManager().newTextureView(viewInit);

			outAtt.m_textureView = std::move(view);
		}

		if(fbDescr.m_shadingRateAttachmentTexelWidth > 0)
		{
			TextureViewInitInfo viewInit(m_ctx->m_rts[rtHandles[kMaxColorRenderTargets + 1].m_idx].m_texture,
										 fbDescr.m_shadingRateAttachmentSurface, "RenderGraph SRI");
			TextureViewPtr view = getManager().newTextureView(viewInit);

			fbInit.m_shadingRateImage.m_texelWidth = fbDescr.m_shadingRateAttachmentTexelWidth;
			fbInit.m_shadingRateImage.m_texelHeight = fbDescr.m_shadingRateAttachmentTexelHeight;
			fbInit.m_shadingRateImage.m_textureView = std::move(view);
		}

		// Set FB name
		fbInit.setName(name);

		// Create
		fb = getManager().newFramebuffer(fbInit);
		m_fbCache.emplace(getMemoryPool(), hash, fb);
	}

	return fb;
}

Bool RenderGraph::overlappingTextureSubresource(const TextureSubresourceInfo& suba, const TextureSubresourceInfo& subb)
{
#define ANKI_OVERLAPPING(first, count) \
	((suba.first < subb.first + subb.count) && (subb.first < suba.first + suba.count))

	const Bool overlappingFaces = ANKI_OVERLAPPING(m_firstFace, m_faceCount);
	const Bool overlappingMips = ANKI_OVERLAPPING(m_firstMipmap, m_mipmapCount);
	const Bool overlappingLayers = ANKI_OVERLAPPING(m_firstLayer, m_layerCount);
#undef ANKI_OVERLAPPING

	return overlappingFaces && overlappingLayers && overlappingMips;
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

		if(fullDep.getAny())
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

					if(overlappingTextureSubresource(aDep.m_texture.m_subresource, bDep.m_texture.m_subresource))
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

		if(fullDep.getAny())
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
			if(ctx.m_passIsInBatch.get(depPassIdx) == false)
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
	ctx->m_rts.create(pool, descr.m_renderTargets.getSize());
	for(U32 rtIdx = 0; rtIdx < ctx->m_rts.getSize(); ++rtIdx)
	{
		RT& outRt = ctx->m_rts[rtIdx];
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
		outRt.m_surfOrVolUsages.create(pool, surfOrVolumeCount, TextureUsageBit::kNone);
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

		outRt.m_lastBatchThatTransitionedIt.create(pool, surfOrVolumeCount, kMaxU16);
		outRt.m_imported = imported;
	}

	// Buffers
	ctx->m_buffers.create(pool, descr.m_buffers.getSize());
	for(U32 buffIdx = 0; buffIdx < ctx->m_buffers.getSize(); ++buffIdx)
	{
		ctx->m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
		ANKI_ASSERT(descr.m_buffers[buffIdx].m_importedBuff.isCreated());
		ctx->m_buffers[buffIdx].m_buffer = descr.m_buffers[buffIdx].m_importedBuff;
		ctx->m_buffers[buffIdx].m_offset = descr.m_buffers[buffIdx].m_offset;
		ctx->m_buffers[buffIdx].m_range = descr.m_buffers[buffIdx].m_range;
	}

	// AS
	ctx->m_as.create(pool, descr.m_as.getSize());
	for(U32 i = 0; i < descr.m_as.getSize(); ++i)
	{
		ctx->m_as[i].m_usage = descr.m_as[i].m_usage;
		ctx->m_as[i].m_as = descr.m_as[i].m_importedAs;
		ANKI_ASSERT(ctx->m_as[i].m_as.isCreated());
	}

	ctx->m_gatherStatistics = descr.m_gatherStatistics;

	return ctx;
}

void RenderGraph::initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackMemoryPool& pool)
{
	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	ctx.m_passes.create(pool, passCount);
	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassDescriptionBase& inPass = *descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];

		outPass.m_callback.copy(inPass.m_callback, pool);

		// Create consumer info
		outPass.m_consumedTextures.resize(pool, inPass.m_rtDeps.getSize());
		for(U32 depIdx = 0; depIdx < inPass.m_rtDeps.getSize(); ++depIdx)
		{
			const RenderPassDependency& inDep = inPass.m_rtDeps[depIdx];
			ANKI_ASSERT(inDep.m_type == RenderPassDependency::Type::kTexture);

			RenderPassDependency::TextureInfo& inf = outPass.m_consumedTextures[depIdx];

			ANKI_ASSERT(sizeof(inf) == sizeof(inDep.m_texture));
			memcpy(&inf, &inDep.m_texture, sizeof(inf));
		}

		// Create command buffers and framebuffer
		if(inPass.m_type == RenderPassDescriptionBase::Type::kGraphics)
		{
			const GraphicsRenderPassDescription& graphicsPass =
				static_cast<const GraphicsRenderPassDescription&>(inPass);

			if(graphicsPass.hasFramebuffer())
			{
				Bool drawsToPresentable;
				outPass.fb() = getOrCreateFramebuffer(graphicsPass.m_fbDescr, &graphicsPass.m_rtHandles[0],
													  inPass.m_name.cstr(), drawsToPresentable);

				outPass.m_fbRenderArea = graphicsPass.m_fbRenderArea;
				outPass.m_drawsToPresentable = drawsToPresentable;
			}
			else
			{
				ANKI_ASSERT(inPass.m_secondLevelCmdbsCount == 0 && "Can't have second level cmdbs");
			}
		}
		else
		{
			ANKI_ASSERT(inPass.m_secondLevelCmdbsCount == 0 && "Can't have second level cmdbs");
		}

		// Set dependencies by checking all previous subpasses.
		U32 prevPassIdx = passIdx;
		while(prevPassIdx--)
		{
			const RenderPassDescriptionBase& prevPass = *descr.m_passes[prevPassIdx];
			if(passADependsOnB(inPass, prevPass))
			{
				outPass.m_dependsOn.emplaceBack(pool, prevPassIdx);
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
	Bool setTimestamp = m_ctx->m_gatherStatistics;
	while(passesAssignedToBatchCount < passCount)
	{
		m_ctx->m_batches.emplaceBack(*m_ctx->m_pool);
		Batch& batch = m_ctx->m_batches.getBack();

		Bool drawsToPresentable = false;

		for(U32 i = 0; i < passCount; ++i)
		{
			if(!m_ctx->m_passIsInBatch.get(i) && !passHasUnmetDependencies(*m_ctx, i))
			{
				// Add to the batch
				++passesAssignedToBatchCount;
				batch.m_passIndices.emplaceBack(*m_ctx->m_pool, i);

				// Will batch draw to the swapchain?
				drawsToPresentable = drawsToPresentable || m_ctx->m_passes[i].m_drawsToPresentable;
			}
		}

		// Get or create cmdb for the batch.
		// Create a new cmdb if the batch is writing to swapchain. This will help Vulkan to have a dependency of the
		// swap chain image acquire to the 2nd command buffer instead of adding it to a single big cmdb.
		if(m_ctx->m_graphicsCmdbs.isEmpty() || drawsToPresentable)
		{
			CommandBufferInitInfo cmdbInit;
			cmdbInit.m_flags = CommandBufferFlag::kGeneralWork;
			CommandBufferPtr cmdb = getManager().newCommandBuffer(cmdbInit);

			m_ctx->m_graphicsCmdbs.emplaceBack(*m_ctx->m_pool, cmdb);

			batch.m_cmdb = cmdb.get();

			// Maybe write a timestamp
			if(ANKI_UNLIKELY(setTimestamp))
			{
				setTimestamp = false;
				TimestampQueryPtr query = getManager().newTimestampQuery();
				TimestampQuery* pQuery = query.get();
				cmdb->resetTimestampQueries({&pQuery, 1});
				cmdb->writeTimestamp(query);

				m_statistics.m_nextTimestamp = (m_statistics.m_nextTimestamp + 1) % kMaxBufferedTimestamps;
				m_statistics.m_timestamps[m_statistics.m_nextTimestamp * 2] = query;
			}
		}
		else
		{
			batch.m_cmdb = m_ctx->m_graphicsCmdbs.getBack().get();
		}

		// Mark batch's passes done
		for(U32 passIdx : m_ctx->m_batches.getBack().m_passIndices)
		{
			m_ctx->m_passIsInBatch.set(passIdx);
			m_ctx->m_passes[passIdx].m_batchIdx = m_ctx->m_batches.getSize() - 1;
		}
	}
}

void RenderGraph::initGraphicsPasses(const RenderGraphDescription& descr, StackMemoryPool& pool)
{
	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassDescriptionBase& inPass = *descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];

		// Create command buffers and framebuffer
		if(inPass.m_type == RenderPassDescriptionBase::Type::kGraphics)
		{
			const GraphicsRenderPassDescription& graphicsPass =
				static_cast<const GraphicsRenderPassDescription&>(inPass);

			if(graphicsPass.hasFramebuffer())
			{
				// Init the usage bits
				TextureUsageBit usage;
				for(U i = 0; i < graphicsPass.m_fbDescr.m_colorAttachmentCount; ++i)
				{
					getCrntUsage(graphicsPass.m_rtHandles[i], outPass.m_batchIdx,
								 TextureSubresourceInfo(graphicsPass.m_fbDescr.m_colorAttachments[i].m_surface), usage);

					outPass.m_colorUsages[i] = usage;
				}

				if(!!graphicsPass.m_fbDescr.m_depthStencilAttachment.m_aspect)
				{
					TextureSubresourceInfo subresource =
						TextureSubresourceInfo(graphicsPass.m_fbDescr.m_depthStencilAttachment.m_surface,
											   graphicsPass.m_fbDescr.m_depthStencilAttachment.m_aspect);

					getCrntUsage(graphicsPass.m_rtHandles[kMaxColorRenderTargets], outPass.m_batchIdx, subresource,
								 usage);

					outPass.m_dsUsage = usage;
				}

				// Do some pre-work for the second level command buffers
				if(inPass.m_secondLevelCmdbsCount)
				{
					outPass.m_secondLevelCmdbs.create(pool, inPass.m_secondLevelCmdbsCount);
					CommandBufferInitInfo& cmdbInit = outPass.m_secondLevelCmdbInitInfo;
					cmdbInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSecondLevel;
					ANKI_ASSERT(cmdbInit.m_framebuffer.isCreated());
					cmdbInit.m_colorAttachmentUsages = outPass.m_colorUsages;
					cmdbInit.m_depthStencilAttachmentUsage = outPass.m_dsUsage;
				}
			}
			else
			{
				ANKI_ASSERT(inPass.m_secondLevelCmdbsCount == 0 && "Can't have second level cmdbs");
			}
		}
		else
		{
			ANKI_ASSERT(inPass.m_secondLevelCmdbsCount == 0 && "Can't have second level cmdbs");
		}
	}
}

template<typename TFunc>
void RenderGraph::iterateSurfsOrVolumes(const TexturePtr& tex, const TextureSubresourceInfo& subresource, TFunc func)
{
	for(U32 mip = subresource.m_firstMipmap; mip < subresource.m_firstMipmap + subresource.m_mipmapCount; ++mip)
	{
		for(U32 layer = subresource.m_firstLayer; layer < subresource.m_firstLayer + subresource.m_layerCount; ++layer)
		{
			for(U32 face = subresource.m_firstFace; face < U32(subresource.m_firstFace + subresource.m_faceCount);
				++face)
			{
				// Compute surf or vol idx
				const U32 faceCount = textureTypeIsCube(tex->getTextureType()) ? 6 : 1;
				const U32 idx = (faceCount * tex->getLayerCount()) * mip + faceCount * layer + face;
				const TextureSurfaceInfo surf(mip, 0, face, layer);

				if(!func(idx, surf))
				{
					return;
				}
			}
		}
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

	iterateSurfsOrVolumes(
		rt.m_texture, dep.m_texture.m_subresource, [&](U32 surfOrVolIdx, const TextureSurfaceInfo& surf) {
			TextureUsageBit& crntUsage = rt.m_surfOrVolUsages[surfOrVolIdx];
			if(crntUsage != depUsage)
			{
				// Check if we can merge barriers
				if(rt.m_lastBatchThatTransitionedIt[surfOrVolIdx] == batchIdx)
				{
					// Will merge the barriers

					crntUsage |= depUsage;

					[[maybe_unused]] Bool found = false;
					for(TextureBarrier& b : batch.m_textureBarriersBefore)
					{
						if(b.m_idx == rtIdx && b.m_surface == surf)
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

					batch.m_textureBarriersBefore.emplaceBack(*ctx.m_pool, rtIdx, crntUsage, depUsage, surf,
															  dep.m_texture.m_subresource.m_depthStencilAspect);

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
	StackMemoryPool& pool = *ctx.m_pool;

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

				if(depUsage == crntUsage)
				{
					continue;
				}

				const Bool buffHasBarrier = buffHasBarrierMask.get(buffIdx);

				if(!buffHasBarrier)
				{
					// Buff hasn't had a barrier in this batch, add a new barrier

					batch.m_bufferBarriersBefore.emplaceBack(pool, buffIdx, crntUsage, depUsage);

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

				if(depUsage == crntUsage)
				{
					continue;
				}

				const Bool asHasBarrierInThisBatch = asHasBarrierMask.get(asIdx);
				if(!asHasBarrierInThisBatch)
				{
					// AS doesn't have a barrier in this batch, create a new one

					batch.m_asBarriersBefore.emplaceBack(pool, asIdx, crntUsage, depUsage);
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

		std::sort(batch.m_asBarriersBefore.getBegin(), batch.m_asBarriersBefore.getEnd(),
				  [&](const ASBarrier& a, const ASBarrier& b) {
					  return a.m_idx < b.m_idx;
				  });
#endif
	} // For all batches
}

void RenderGraph::compileNewGraph(const RenderGraphDescription& descr, StackMemoryPool& pool)
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_COMPILE);

	// Init the context
	BakeContext& ctx = *newContext(descr, pool);
	m_ctx = &ctx;

	// Init the passes and find the dependencies between passes
	initRenderPassesAndSetDeps(descr, pool);

	// Walk the graph and create pass batches
	initBatches();

	// Now that we know the batches every pass belongs init the graphics passes
	initGraphicsPasses(descr, pool);

	// Create barriers between batches
	setBatchBarriers(descr);

#if ANKI_DBG_RENDER_GRAPH
	if(dumpDependencyDotFile(descr, ctx, "./"))
	{
		ANKI_LOGF("Won't recover on debug code");
	}
#endif
}

TexturePtr RenderGraph::getTexture(RenderTargetHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_rts[handle.m_idx].m_texture.isCreated());
	return m_ctx->m_rts[handle.m_idx].m_texture;
}

BufferPtr RenderGraph::getBuffer(BufferHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_buffers[handle.m_idx].m_buffer.isCreated());
	return m_ctx->m_buffers[handle.m_idx].m_buffer;
}

AccelerationStructurePtr RenderGraph::getAs(AccelerationStructureHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_as[handle.m_idx].m_as.isCreated());
	return m_ctx->m_as[handle.m_idx].m_as;
}

void RenderGraph::runSecondLevel(U32 threadIdx)
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_2ND_LEVEL);
	ANKI_ASSERT(m_ctx);

	RenderPassWorkContext ctx;
	ctx.m_rgraph = this;
	ctx.m_currentSecondLevelCommandBufferIndex = threadIdx;

	for(Pass& p : m_ctx->m_passes)
	{
		const U32 size = p.m_secondLevelCmdbs.getSize();
		if(threadIdx < size)
		{
			ANKI_ASSERT(!p.m_secondLevelCmdbs[threadIdx].isCreated());
			p.m_secondLevelCmdbs[threadIdx] = getManager().newCommandBuffer(p.m_secondLevelCmdbInitInfo);

			ctx.m_commandBuffer = p.m_secondLevelCmdbs[threadIdx];
			ctx.m_secondLevelCommandBufferCount = size;
			ctx.m_passIdx = U32(&p - &m_ctx->m_passes[0]);
			ctx.m_batchIdx = p.m_batchIdx;

			ANKI_ASSERT(ctx.m_commandBuffer.isCreated());

			{
				ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_CALLBACK);
				p.m_callback(ctx);
			}

			ctx.m_commandBuffer->flush();
		}
	}
}

void RenderGraph::run() const
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_RUN);
	ANKI_ASSERT(m_ctx);

	RenderPassWorkContext ctx;
	ctx.m_rgraph = this;
	ctx.m_currentSecondLevelCommandBufferIndex = 0;
	ctx.m_secondLevelCommandBufferCount = 0;

	for(const Batch& batch : m_ctx->m_batches)
	{
		ctx.m_commandBuffer.reset(batch.m_cmdb);
		CommandBufferPtr& cmdb = ctx.m_commandBuffer;

		// Set the barriers
		DynamicArrayRaii<TextureBarrierInfo> texBarriers(m_ctx->m_pool);
		texBarriers.resizeStorage(batch.m_textureBarriersBefore.getSize());
		for(const TextureBarrier& barrier : batch.m_textureBarriersBefore)
		{
			TextureBarrierInfo& inf = *texBarriers.emplaceBack();
			inf.m_previousUsage = barrier.m_usageBefore;
			inf.m_nextUsage = barrier.m_usageAfter;
			inf.m_subresource = barrier.m_surface;
			inf.m_subresource.m_depthStencilAspect = barrier.m_dsAspect;
			inf.m_texture = m_ctx->m_rts[barrier.m_idx].m_texture.get();
		}
		DynamicArrayRaii<BufferBarrierInfo> buffBarriers(m_ctx->m_pool);
		buffBarriers.resizeStorage(batch.m_bufferBarriersBefore.getSize());
		for(const BufferBarrier& barrier : batch.m_bufferBarriersBefore)
		{
			BufferBarrierInfo& inf = *buffBarriers.emplaceBack();
			inf.m_previousUsage = barrier.m_usageBefore;
			inf.m_nextUsage = barrier.m_usageAfter;
			inf.m_offset = m_ctx->m_buffers[barrier.m_idx].m_offset;
			inf.m_size = m_ctx->m_buffers[barrier.m_idx].m_range;
			inf.m_buffer = m_ctx->m_buffers[barrier.m_idx].m_buffer.get();
		}
		DynamicArrayRaii<AccelerationStructureBarrierInfo> asBarriers(m_ctx->m_pool);
		for(const ASBarrier& barrier : batch.m_asBarriersBefore)
		{
			AccelerationStructureBarrierInfo& inf = *asBarriers.emplaceBack();
			inf.m_previousUsage = barrier.m_usageBefore;
			inf.m_nextUsage = barrier.m_usageAfter;
			inf.m_as = m_ctx->m_as[barrier.m_idx].m_as.get();
		}
		cmdb->setPipelineBarrier(texBarriers, buffBarriers, asBarriers);

		// Call the passes
		for(U32 passIdx : batch.m_passIndices)
		{
			const Pass& pass = m_ctx->m_passes[passIdx];

			if(pass.fb().isCreated())
			{
				cmdb->beginRenderPass(pass.fb(), pass.m_colorUsages, pass.m_dsUsage, pass.m_fbRenderArea[0],
									  pass.m_fbRenderArea[1], pass.m_fbRenderArea[2], pass.m_fbRenderArea[3]);
			}

			const U size = pass.m_secondLevelCmdbs.getSize();
			if(size == 0)
			{
				ctx.m_passIdx = passIdx;
				ctx.m_batchIdx = pass.m_batchIdx;

				ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_CALLBACK);
				pass.m_callback(ctx);
			}
			else
			{
				DynamicArrayRaii<CommandBuffer*> cmdbs(m_ctx->m_pool);
				cmdbs.resizeStorage(size);
				for(const CommandBufferPtr& cmdb2nd : pass.m_secondLevelCmdbs)
				{
					cmdbs.emplaceBack(cmdb2nd.get());
				}
				cmdb->pushSecondLevelCommandBuffers(cmdbs);
			}

			if(pass.fb().isCreated())
			{
				cmdb->endRenderPass();
			}
		}
	}
}

void RenderGraph::flush()
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_FLUSH);

	for(U32 i = 0; i < m_ctx->m_graphicsCmdbs.getSize(); ++i)
	{
		if(ANKI_UNLIKELY(m_ctx->m_gatherStatistics && i == m_ctx->m_graphicsCmdbs.getSize() - 1))
		{
			// Write a timestamp before the last flush

			TimestampQueryPtr query = getManager().newTimestampQuery();
			TimestampQuery* pQuery = query.get();
			m_ctx->m_graphicsCmdbs[i]->resetTimestampQueries({&pQuery, 1});
			m_ctx->m_graphicsCmdbs[i]->writeTimestamp(query);

			m_statistics.m_timestamps[m_statistics.m_nextTimestamp * 2 + 1] = query;
			m_statistics.m_cpuStartTimes[m_statistics.m_nextTimestamp] = HighRezTimer::getCurrentTime();
		}

		// Flush
		m_ctx->m_graphicsCmdbs[i]->flush();
	}
}

void RenderGraph::getCrntUsage(RenderTargetHandle handle, U32 batchIdx, const TextureSubresourceInfo& subresource,
							   TextureUsageBit& usage) const
{
	usage = TextureUsageBit::kNone;
	const Batch& batch = m_ctx->m_batches[batchIdx];

	for(U32 passIdx : batch.m_passIndices)
	{
		for(const RenderPassDependency::TextureInfo& consumer : m_ctx->m_passes[passIdx].m_consumedTextures)
		{
			if(consumer.m_handle == handle && overlappingTextureSubresource(subresource, consumer.m_subresource))
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
			DynamicArray<TexturePtr> newArray;
			if(entry.m_texturesInUse > 0)
			{
				newArray.create(getMemoryPool(), entry.m_texturesInUse);
			}

			// Populate the new array
			for(U32 i = 0; i < newArray.getSize(); ++i)
			{
				newArray[i] = std::move(entry.m_textures[i]);
			}

			// Destroy the old array and the rest of the textures
			entry.m_textures.destroy(getMemoryPool());

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
	ANKI_TEX_USAGE(kImageGeometryRead);
	ANKI_TEX_USAGE(kImageGeometryWrite);
	ANKI_TEX_USAGE(kImageFragmentRead);
	ANKI_TEX_USAGE(kImageFragmentWrite);
	ANKI_TEX_USAGE(kImageComputeRead);
	ANKI_TEX_USAGE(kImageComputeWrite);
	ANKI_TEX_USAGE(kImageTraceRaysRead);
	ANKI_TEX_USAGE(kImageTraceRaysWrite);
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

	ANKI_BUFF_USAGE(kUniformGeometry);
	ANKI_BUFF_USAGE(kUniformFragment);
	ANKI_BUFF_USAGE(kUniformCompute);
	ANKI_BUFF_USAGE(kUniformTraceRays);
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

Error RenderGraph::dumpDependencyDotFile(const RenderGraphDescription& descr, const BakeContext& ctx,
										 CString path) const
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

			slist.pushBackSprintf(
				"\t\"%s\"[color=%s,style=%s,shape=box];\n", passName.cstr(), COLORS[batchIdx % COLORS.getSize()],
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
			barrierLabel.sprintf("<b>%s</b> (mip,dp,f,l)=(%u,%u,%u,%u)<br/>%s <b>to</b> %s",
								 &descr.m_renderTargets[barrier.m_idx].m_name[0], barrier.m_surface.m_level,
								 barrier.m_surface.m_depth, barrier.m_surface.m_face, barrier.m_surface.m_layer,
								 textureUsageToStr(pool, barrier.m_usageBefore).cstr(),
								 textureUsageToStr(pool, barrier.m_usageAfter).cstr());

			StringRaii barrierName(&pool);
			barrierName.sprintf("%s tex barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()], barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 barrierIdx = 0; barrierIdx < batch.m_bufferBarriersBefore.getSize(); ++barrierIdx)
		{
			const BufferBarrier& barrier = batch.m_bufferBarriersBefore[barrierIdx];

			StringRaii barrierLabel(&pool);
			barrierLabel.sprintf("<b>%s</b><br/>%s <b>to</b> %s", &descr.m_buffers[barrier.m_idx].m_name[0],
								 bufferUsageToStr(pool, barrier.m_usageBefore).cstr(),
								 bufferUsageToStr(pool, barrier.m_usageAfter).cstr());

			StringRaii barrierName(&pool);
			barrierName.sprintf("%s buff barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()], barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 barrierIdx = 0; barrierIdx < batch.m_asBarriersBefore.getSize(); ++barrierIdx)
		{
			const ASBarrier& barrier = batch.m_asBarriersBefore[barrierIdx];

			StringRaii barrierLabel(&pool);
			barrierLabel.sprintf("<b>%s</b><br/>%s <b>to</b> %s", descr.m_as[barrier.m_idx].m_name.getBegin(),
								 asUsageToStr(pool, barrier.m_usageBefore).cstr(),
								 asUsageToStr(pool, barrier.m_usageAfter).cstr());

			StringRaii barrierName(&pool);
			barrierName.sprintf("%s AS barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()], barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 passIdx : batch.m_passIndices)
		{
			const RenderPassDescriptionBase& pass = *descr.m_passes[passIdx];
			StringRaii passName(&pool);
			passName.sprintf("%s pass", pass.m_name.cstr());
			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold];\n", passName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()]);
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), passName.cstr());

			prevBubble = passName;
		}
	}
	// slist.pushBackSprintf("}\n");

	slist.pushBackSprintf("}");

	File file;
	ANKI_CHECK(file.open(StringRaii(&pool).sprintf("%s/rgraph_%05u.dot", &path[0], m_version).toCString(),
						 FileOpenFlag::kWrite));
	for(const String& s : slist)
	{
		ANKI_CHECK(file.writeTextf("%s", &s[0]));
	}

	return Error::kNone;
}
#endif

} // end namespace anki
