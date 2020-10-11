// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/RenderGraph.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/util/Tracer.h>
#include <anki/util/BitSet.h>
#include <anki/util/File.h>
#include <anki/util/StringList.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

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

	TextureBarrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSurfaceInfo& surf)
		: m_idx(rtIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
		, m_surface(surf)
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

	RenderPassWorkCallback m_callback;
	void* m_userData;

	DynamicArray<CommandBufferPtr> m_secondLevelCmdbs;
	/// Will reuse the m_secondLevelCmdbInitInfo.m_framebuffer to get the framebuffer.
	CommandBufferInitInfo m_secondLevelCmdbInitInfo;
	Array<U32, 4> m_fbRenderArea;
	Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS> m_colorUsages = {}; ///< For beginRender pass
	TextureUsageBit m_dsUsage = TextureUsageBit::NONE; ///< For beginRender pass

	U32 m_batchIdx ANKI_DEBUG_CODE(= MAX_U32);
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
	StackAllocator<U8> m_alloc;
	DynamicArray<Pass> m_passes;
	BitSet<MAX_RENDER_GRAPH_PASSES, U64> m_passIsInBatch{false};
	DynamicArray<Batch> m_batches;
	DynamicArray<RT> m_rts;
	DynamicArray<Buffer> m_buffers;
	DynamicArray<AS> m_as;

	DynamicArray<CommandBufferPtr> m_graphicsCmdbs;

	Bool m_gatherStatistics = false;

	BakeContext(const StackAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}
};

void FramebufferDescription::bake()
{
	ANKI_ASSERT(m_hash == 0 && "Already baked");

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

		Array<ColorAttachment, MAX_COLOR_ATTACHMENTS> colorAttachments;
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
		struct DSAttachment
		{
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
		const Bool hasDepth = !!(inAtt.m_aspect & DepthStencilAspectBit::DEPTH);
		const Bool hasStencil = !!(inAtt.m_aspect & DepthStencilAspectBit::STENCIL);

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

	ANKI_ASSERT(m_hash != 0 && m_hash != 1);
}

RenderGraph::RenderGraph(GrManager* manager, CString name)
	: GrObject(manager, CLASS_TYPE, name)
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
		entry.m_textures.destroy(getAllocator());
		m_renderTargetCache.erase(getAllocator(), it);
	}

	m_fbCache.destroy(getAllocator());

	for(auto& it : m_importedRenderTargets)
	{
		it.m_surfOrVolLastUsages.destroy(getAllocator());
	}

	m_importedRenderTargets.destroy(getAllocator());
}

RenderGraph* RenderGraph::newInstance(GrManager* manager)
{
	return manager->getAllocator().newInstance<RenderGraph>(manager, "N/A");
}

void RenderGraph::reset()
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_RESET);

	if(!m_ctx)
	{
		return;
	}

	if((m_version % PERIODIC_CLEANUP_EVERY) == 0)
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
				it = m_importedRenderTargets.emplace(getAllocator(), hash);
				it->m_surfOrVolLastUsages.create(getAllocator(), surfOrVolumeCount);
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
		p.m_secondLevelCmdbs.destroy(m_ctx->m_alloc);
	}

	m_ctx->m_graphicsCmdbs.destroy(m_ctx->m_alloc);

	m_ctx->m_alloc = StackAllocator<U8>();
	m_ctx = nullptr;
	++m_version;
}

TexturePtr RenderGraph::getOrCreateRenderTarget(const TextureInitInfo& initInf, U64 hash)
{
	ANKI_ASSERT(hash);
	auto alloc = getManager().getAllocator();

	// Find a cache entry
	RenderTargetCacheEntry* entry = nullptr;
	auto it = m_renderTargetCache.find(hash);
	if(ANKI_UNLIKELY(it == m_renderTargetCache.getEnd()))
	{
		// Didn't found the entry, create a new one

		auto it2 = m_renderTargetCache.emplace(getAllocator(), hash);
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
		entry->m_textures.resize(alloc, entry->m_textures.getSize() + 1);
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
	Array<U64, MAX_COLOR_ATTACHMENTS + 1> uuids;
	U count = 0;
	for(U i = 0; i < fbDescr.m_colorAttachmentCount; ++i)
	{
		uuids[count++] = m_ctx->m_rts[rtHandles[i].m_idx].m_texture->getUuid();

		if(!!(m_ctx->m_rts[rtHandles[i].m_idx].m_texture->getTextureUsage() & TextureUsageBit::PRESENT))
		{
			drawsToPresentable = true;
		}
	}

	if(!!fbDescr.m_depthStencilAttachment.m_aspect)
	{
		uuids[count++] = m_ctx->m_rts[rtHandles[MAX_COLOR_ATTACHMENTS].m_idx].m_texture->getUuid();
	}

	hash = appendHash(&uuids[0], sizeof(U64) * count, hash);

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

			outAtt.m_textureView = view;
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
			TextureViewInitInfo viewInit(m_ctx->m_rts[rtHandles[MAX_COLOR_ATTACHMENTS].m_idx].m_texture,
										 TextureSubresourceInfo(inAtt.m_surface, inAtt.m_aspect), "RenderGraph");
			TextureViewPtr view = getManager().newTextureView(viewInit);

			outAtt.m_textureView = view;
		}

		// Set FB name
		Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> cutName;
		const U cutNameLen = min<U>(name.getLength(), MAX_GR_OBJECT_NAME_LENGTH);
		memcpy(&cutName[0], &name[0], cutNameLen);
		cutName[cutNameLen] = '\0';
		fbInit.setName(&cutName[0]);

		// Create
		fb = getManager().newFramebuffer(fbInit);
		m_fbCache.emplace(getAllocator(), hash, fb);
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
		const BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> aReadBWrite = a.m_readRtMask & b.m_writeRtMask;
		const BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> aWriteBRead = a.m_writeRtMask & b.m_readRtMask;
		const BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> aWriteBWrite = a.m_writeRtMask & b.m_writeRtMask;

		const BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS, U64> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

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

					if(!((aDep.m_texture.m_usage | bDep.m_texture.m_usage) & TextureUsageBit::ALL_WRITE))
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
		const BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> aReadBWrite = a.m_readBuffMask & b.m_writeBuffMask;
		const BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> aWriteBRead = a.m_writeBuffMask & b.m_readBuffMask;
		const BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> aWriteBWrite = a.m_writeBuffMask & b.m_writeBuffMask;

		const BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

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

					if(!((aDep.m_buffer.m_usage | bDep.m_buffer.m_usage) & BufferUsageBit::ALL_WRITE))
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
		const BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> aReadBWrite = a.m_readAsMask & b.m_writeAsMask;
		const BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> aWriteBRead = a.m_writeAsMask & b.m_readAsMask;
		const BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> aWriteBWrite = a.m_writeAsMask & b.m_writeAsMask;

		const BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

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

					if(!((aDep.m_as.m_usage | bDep.m_as.m_usage) & AccelerationStructureUsageBit::ALL_WRITE))
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

RenderGraph::BakeContext* RenderGraph::newContext(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	// Allocate
	BakeContext* ctx = alloc.newInstance<BakeContext>(alloc);

	// Init the resources
	ctx->m_rts.create(alloc, descr.m_renderTargets.getSize());
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
			ANKI_ASSERT(initInf.m_usage != TextureUsageBit::NONE);

			// Create the new hash
			const U64 hash = appendHash(&initInf.m_usage, sizeof(initInf.m_usage), inRt.m_hash);

			// Get or create the texture
			outRt.m_texture = getOrCreateRenderTarget(initInf, hash);
		}

		// Init the usage
		const U32 surfOrVolumeCount = getTextureSurfOrVolCount(outRt.m_texture);
		outRt.m_surfOrVolUsages.create(alloc, surfOrVolumeCount, TextureUsageBit::NONE);
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

		outRt.m_lastBatchThatTransitionedIt.create(alloc, surfOrVolumeCount, MAX_U16);
		outRt.m_imported = imported;
	}

	// Buffers
	ctx->m_buffers.create(alloc, descr.m_buffers.getSize());
	for(U32 buffIdx = 0; buffIdx < ctx->m_buffers.getSize(); ++buffIdx)
	{
		ctx->m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
		ANKI_ASSERT(descr.m_buffers[buffIdx].m_importedBuff.isCreated());
		ctx->m_buffers[buffIdx].m_buffer = descr.m_buffers[buffIdx].m_importedBuff;
	}

	// AS
	ctx->m_as.create(alloc, descr.m_as.getSize());
	for(U32 i = 0; i < descr.m_as.getSize(); ++i)
	{
		ctx->m_as[i].m_usage = descr.m_as[i].m_usage;
		ctx->m_as[i].m_as = descr.m_as[i].m_importedAs;
		ANKI_ASSERT(ctx->m_as[i].m_as.isCreated());
	}

	ctx->m_gatherStatistics = descr.m_gatherStatistics;

	return ctx;
}

void RenderGraph::initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	ctx.m_passes.create(alloc, passCount);
	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassDescriptionBase& inPass = *descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];

		outPass.m_callback = inPass.m_callback;
		outPass.m_userData = inPass.m_userData;

		// Create consumer info
		outPass.m_consumedTextures.resize(alloc, inPass.m_rtDeps.getSize());
		for(U32 depIdx = 0; depIdx < inPass.m_rtDeps.getSize(); ++depIdx)
		{
			const RenderPassDependency& inDep = inPass.m_rtDeps[depIdx];
			ANKI_ASSERT(inDep.m_type == RenderPassDependency::Type::TEXTURE);

			RenderPassDependency::TextureInfo& inf = outPass.m_consumedTextures[depIdx];

			ANKI_ASSERT(sizeof(inf) == sizeof(inDep.m_texture));
			memcpy(&inf, &inDep.m_texture, sizeof(inf));
		}

		// Create command buffers and framebuffer
		if(inPass.m_type == RenderPassDescriptionBase::Type::GRAPHICS)
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
				outPass.m_dependsOn.emplaceBack(alloc, prevPassIdx);
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
		m_ctx->m_batches.emplaceBack(m_ctx->m_alloc);
		Batch& batch = m_ctx->m_batches.getBack();

		Bool drawsToPresentable = false;

		for(U32 i = 0; i < passCount; ++i)
		{
			if(!m_ctx->m_passIsInBatch.get(i) && !passHasUnmetDependencies(*m_ctx, i))
			{
				// Add to the batch
				++passesAssignedToBatchCount;
				batch.m_passIndices.emplaceBack(m_ctx->m_alloc, i);

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
			cmdbInit.m_flags = CommandBufferFlag::COMPUTE_WORK | CommandBufferFlag::GRAPHICS_WORK;
			CommandBufferPtr cmdb = getManager().newCommandBuffer(cmdbInit);

			m_ctx->m_graphicsCmdbs.emplaceBack(m_ctx->m_alloc, cmdb);

			batch.m_cmdb = cmdb.get();

			// Maybe write a timestamp
			if(ANKI_UNLIKELY(setTimestamp))
			{
				setTimestamp = false;
				TimestampQueryPtr query = getManager().newTimestampQuery();
				cmdb->resetTimestampQuery(query);
				cmdb->writeTimestamp(query);

				m_statistics.m_nextTimestamp = (m_statistics.m_nextTimestamp + 1) % MAX_TIMESTAMPS_BUFFERED;
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

void RenderGraph::initGraphicsPasses(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	BakeContext& ctx = *m_ctx;
	const U32 passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	for(U32 passIdx = 0; passIdx < passCount; ++passIdx)
	{
		const RenderPassDescriptionBase& inPass = *descr.m_passes[passIdx];
		Pass& outPass = ctx.m_passes[passIdx];

		// Create command buffers and framebuffer
		if(inPass.m_type == RenderPassDescriptionBase::Type::GRAPHICS)
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

					getCrntUsage(graphicsPass.m_rtHandles[MAX_COLOR_ATTACHMENTS], outPass.m_batchIdx, subresource,
								 usage);

					outPass.m_dsUsage = usage;
				}

				// Do some pre-work for the second level command buffers
				if(inPass.m_secondLevelCmdbsCount)
				{
					outPass.m_secondLevelCmdbs.create(alloc, inPass.m_secondLevelCmdbsCount);
					CommandBufferInitInfo& cmdbInit = outPass.m_secondLevelCmdbInitInfo;
					cmdbInit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SECOND_LEVEL;
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
	ANKI_ASSERT(dep.m_type == RenderPassDependency::Type::TEXTURE);

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

					Bool found = false;
					for(TextureBarrier& b : batch.m_textureBarriersBefore)
					{
						if(b.m_idx == rtIdx && b.m_surface == surf)
						{
							b.m_usageAfter |= depUsage;
							found = true;
							break;
						}
					}

					(void)found;
					ANKI_ASSERT(found);
				}
				else
				{
					// Create a new barrier for this surface

					batch.m_textureBarriersBefore.emplaceBack(ctx.m_alloc, rtIdx, crntUsage, depUsage, surf);

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
	const StackAllocator<U8>& alloc = ctx.m_alloc;

	// For all batches
	for(Batch& batch : ctx.m_batches)
	{
		BitSet<MAX_RENDER_GRAPH_BUFFERS, U64> buffHasBarrierMask(false);
		BitSet<MAX_RENDER_GRAPH_ACCELERATION_STRUCTURES, U32> asHasBarrierMask(false);

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

					batch.m_bufferBarriersBefore.emplaceBack(alloc, buffIdx, crntUsage, depUsage);

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

					batch.m_asBarriersBefore.emplaceBack(alloc, asIdx, crntUsage, depUsage);
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
				  [&](const BufferBarrier& a, const BufferBarrier& b) { return a.m_idx < b.m_idx; });

		std::sort(batch.m_asBarriersBefore.getBegin(), batch.m_asBarriersBefore.getEnd(),
				  [&](const ASBarrier& a, const ASBarrier& b) { return a.m_idx < b.m_idx; });
#endif
	} // For all batches
}

void RenderGraph::compileNewGraph(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_COMPILE);

	// Init the context
	BakeContext& ctx = *newContext(descr, alloc);
	m_ctx = &ctx;

	// Init the passes and find the dependencies between passes
	initRenderPassesAndSetDeps(descr, alloc);

	// Walk the graph and create pass batches
	initBatches();

	// Now that we know the batches every pass belongs init the graphics passes
	initGraphicsPasses(descr, alloc);

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
			ctx.m_userData = p.m_userData;

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
		for(const TextureBarrier& barrier : batch.m_textureBarriersBefore)
		{
			cmdb->setTextureSurfaceBarrier(m_ctx->m_rts[barrier.m_idx].m_texture, barrier.m_usageBefore,
										   barrier.m_usageAfter, barrier.m_surface);
		}
		for(const BufferBarrier& barrier : batch.m_bufferBarriersBefore)
		{
			cmdb->setBufferBarrier(m_ctx->m_buffers[barrier.m_idx].m_buffer, barrier.m_usageBefore,
								   barrier.m_usageAfter, 0, MAX_PTR_SIZE);
		}
		for(const ASBarrier& barrier : batch.m_asBarriersBefore)
		{
			cmdb->setAccelerationStructureBarrier(m_ctx->m_as[barrier.m_idx].m_as, barrier.m_usageBefore,
												  barrier.m_usageAfter);
		}

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
				ctx.m_userData = pass.m_userData;
				ctx.m_passIdx = passIdx;
				ctx.m_batchIdx = pass.m_batchIdx;

				ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH_CALLBACK);
				pass.m_callback(ctx);
			}
			else
			{
				for(const CommandBufferPtr& cmdb2nd : pass.m_secondLevelCmdbs)
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2nd);
				}
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
		// Maybe write a timestamp before flush
		if(ANKI_UNLIKELY(m_ctx->m_gatherStatistics && i == m_ctx->m_graphicsCmdbs.getSize() - 1))
		{
			TimestampQueryPtr query = getManager().newTimestampQuery();
			m_ctx->m_graphicsCmdbs[i]->resetTimestampQuery(query);
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
	usage = TextureUsageBit::NONE;
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
				newArray.create(getAllocator(), entry.m_texturesInUse);
			}

			// Populate the new array
			for(U32 i = 0; i < newArray.getSize(); ++i)
			{
				newArray[i] = std::move(entry.m_textures[i]);
			}

			// Destroy the old array and the rest of the textures
			entry.m_textures.destroy(getAllocator());

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
	const U32 oldFrame = (m_statistics.m_nextTimestamp + 1) % MAX_TIMESTAMPS_BUFFERED;

	if(m_statistics.m_timestamps[oldFrame * 2] && m_statistics.m_timestamps[oldFrame * 2 + 1])
	{
		Second start, end;
		TimestampQueryResult res = m_statistics.m_timestamps[oldFrame * 2]->getResult(start);
		(void)res;
		ANKI_ASSERT(res == TimestampQueryResult::AVAILABLE);

		res = m_statistics.m_timestamps[oldFrame * 2 + 1]->getResult(end);
		ANKI_ASSERT(res == TimestampQueryResult::AVAILABLE);

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
StringAuto RenderGraph::textureUsageToStr(StackAllocator<U8>& alloc, TextureUsageBit usage)
{
	StringListAuto slist(alloc);

#	define ANKI_TEX_USAGE(u) \
		if(!!(usage & TextureUsageBit::u)) \
		{ \
			slist.pushBackSprintf("%s", #u); \
		}

	ANKI_TEX_USAGE(SAMPLED_GEOMETRY);
	ANKI_TEX_USAGE(SAMPLED_FRAGMENT);
	ANKI_TEX_USAGE(SAMPLED_COMPUTE);
	ANKI_TEX_USAGE(SAMPLED_TRACE_RAYS);
	ANKI_TEX_USAGE(IMAGE_GEOMETRY_READ);
	ANKI_TEX_USAGE(IMAGE_GEOMETRY_WRITE);
	ANKI_TEX_USAGE(IMAGE_FRAGMENT_READ);
	ANKI_TEX_USAGE(IMAGE_FRAGMENT_WRITE);
	ANKI_TEX_USAGE(IMAGE_COMPUTE_READ);
	ANKI_TEX_USAGE(IMAGE_COMPUTE_WRITE);
	ANKI_TEX_USAGE(IMAGE_TRACE_RAYS_READ);
	ANKI_TEX_USAGE(IMAGE_TRACE_RAYS_WRITE);
	ANKI_TEX_USAGE(FRAMEBUFFER_ATTACHMENT_READ);
	ANKI_TEX_USAGE(FRAMEBUFFER_ATTACHMENT_WRITE);
	ANKI_TEX_USAGE(TRANSFER_DESTINATION);
	ANKI_TEX_USAGE(GENERATE_MIPMAPS);
	ANKI_TEX_USAGE(PRESENT);

	if(!usage)
	{
		slist.pushBackSprintf("NONE");
	}

#	undef ANKI_TEX_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringAuto str(alloc);
	slist.join(" | ", str);
	return str;
}

StringAuto RenderGraph::bufferUsageToStr(StackAllocator<U8>& alloc, BufferUsageBit usage)
{
	StringListAuto slist(alloc);

#	define ANKI_BUFF_USAGE(u) \
		if(!!(usage & BufferUsageBit::u)) \
		{ \
			slist.pushBackSprintf("%s", #u); \
		}

	ANKI_BUFF_USAGE(UNIFORM_GEOMETRY);
	ANKI_BUFF_USAGE(UNIFORM_FRAGMENT);
	ANKI_BUFF_USAGE(UNIFORM_COMPUTE);
	ANKI_BUFF_USAGE(UNIFORM_TRACE_RAYS);
	ANKI_BUFF_USAGE(STORAGE_GEOMETRY_READ);
	ANKI_BUFF_USAGE(STORAGE_GEOMETRY_WRITE);
	ANKI_BUFF_USAGE(STORAGE_FRAGMENT_READ);
	ANKI_BUFF_USAGE(STORAGE_FRAGMENT_WRITE);
	ANKI_BUFF_USAGE(STORAGE_COMPUTE_READ);
	ANKI_BUFF_USAGE(STORAGE_COMPUTE_WRITE);
	ANKI_BUFF_USAGE(STORAGE_TRACE_RAYS_READ);
	ANKI_BUFF_USAGE(STORAGE_TRACE_RAYS_WRITE);
	ANKI_BUFF_USAGE(TEXTURE_GEOMETRY_READ);
	ANKI_BUFF_USAGE(TEXTURE_GEOMETRY_WRITE);
	ANKI_BUFF_USAGE(TEXTURE_FRAGMENT_READ);
	ANKI_BUFF_USAGE(TEXTURE_FRAGMENT_WRITE);
	ANKI_BUFF_USAGE(TEXTURE_COMPUTE_READ);
	ANKI_BUFF_USAGE(TEXTURE_COMPUTE_WRITE);
	ANKI_BUFF_USAGE(TEXTURE_TRACE_RAYS_READ);
	ANKI_BUFF_USAGE(TEXTURE_TRACE_RAYS_WRITE);
	ANKI_BUFF_USAGE(INDEX);
	ANKI_BUFF_USAGE(VERTEX);
	ANKI_BUFF_USAGE(INDIRECT_COMPUTE);
	ANKI_BUFF_USAGE(INDIRECT_DRAW);
	ANKI_BUFF_USAGE(INDIRECT_TRACE_RAYS);
	ANKI_BUFF_USAGE(TRANSFER_SOURCE);
	ANKI_BUFF_USAGE(TRANSFER_DESTINATION);
	ANKI_BUFF_USAGE(ACCELERATION_STRUCTURE_BUILD);

	if(!usage)
	{
		slist.pushBackSprintf("NONE");
	}

#	undef ANKI_BUFF_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringAuto str(alloc);
	slist.join(" | ", str);
	return str;
}

StringAuto RenderGraph::asUsageToStr(StackAllocator<U8>& alloc, AccelerationStructureUsageBit usage)
{
	StringListAuto slist(alloc);

#	define ANKI_AS_USAGE(u) \
		if(!!(usage & AccelerationStructureUsageBit::u)) \
		{ \
			slist.pushBackSprintf("%s", #u); \
		}

	ANKI_AS_USAGE(BUILD);
	ANKI_AS_USAGE(ATTACH);
	ANKI_AS_USAGE(GEOMETRY_READ);
	ANKI_AS_USAGE(FRAGMENT_READ);
	ANKI_AS_USAGE(COMPUTE_READ);
	ANKI_AS_USAGE(TRACE_RAYS_READ);

#	undef ANKI_AS_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringAuto str(alloc);
	slist.join(" | ", str);
	return str;
}

Error RenderGraph::dumpDependencyDotFile(const RenderGraphDescription& descr, const BakeContext& ctx,
										 CString path) const
{
	ANKI_GR_LOGW("Running with debug code");

	static const Array<const char*, 5> COLORS = {"red", "green", "blue", "magenta", "cyan"};
	auto alloc = ctx.m_alloc;
	StringListAuto slist(alloc);

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
				(descr.m_passes[passIdx]->m_type == RenderPassDescriptionBase::Type::GRAPHICS) ? "bold" : "dashed");

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
	StringAuto prevBubble(ctx.m_alloc);
	prevBubble.create("START");
	for(U32 batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		const Batch& batch = ctx.m_batches[batchIdx];

		StringAuto batchName(ctx.m_alloc);
		batchName.sprintf("batch%u", batchIdx);

		for(U32 barrierIdx = 0; barrierIdx < batch.m_textureBarriersBefore.getSize(); ++barrierIdx)
		{
			const TextureBarrier& barrier = batch.m_textureBarriersBefore[barrierIdx];

			StringAuto barrierLabel(ctx.m_alloc);
			barrierLabel.sprintf("<b>%s</b> (mip,dp,f,l)=(%u,%u,%u,%u)<br/>%s <b>to</b> %s",
								 &descr.m_renderTargets[barrier.m_idx].m_name[0], barrier.m_surface.m_level,
								 barrier.m_surface.m_depth, barrier.m_surface.m_face, barrier.m_surface.m_layer,
								 textureUsageToStr(alloc, barrier.m_usageBefore).cstr(),
								 textureUsageToStr(alloc, barrier.m_usageAfter).cstr());

			StringAuto barrierName(ctx.m_alloc);
			barrierName.sprintf("%s tex barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()], barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 barrierIdx = 0; barrierIdx < batch.m_bufferBarriersBefore.getSize(); ++barrierIdx)
		{
			const BufferBarrier& barrier = batch.m_bufferBarriersBefore[barrierIdx];

			StringAuto barrierLabel(ctx.m_alloc);
			barrierLabel.sprintf("<b>%s</b><br/>%s <b>to</b> %s", &descr.m_buffers[barrier.m_idx].m_name[0],
								 bufferUsageToStr(alloc, barrier.m_usageBefore).cstr(),
								 bufferUsageToStr(alloc, barrier.m_usageAfter).cstr());

			StringAuto barrierName(ctx.m_alloc);
			barrierName.sprintf("%s buff barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()], barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 barrierIdx = 0; barrierIdx < batch.m_asBarriersBefore.getSize(); ++barrierIdx)
		{
			const ASBarrier& barrier = batch.m_asBarriersBefore[barrierIdx];

			StringAuto barrierLabel(ctx.m_alloc);
			barrierLabel.sprintf("<b>%s</b><br/>%s <b>to</b> %s", descr.m_as[barrier.m_idx].m_name.getBegin(),
								 asUsageToStr(alloc, barrier.m_usageBefore).cstr(),
								 asUsageToStr(alloc, barrier.m_usageAfter).cstr());

			StringAuto barrierName(ctx.m_alloc);
			barrierName.sprintf("%s AS barrier%u", batchName.cstr(), barrierIdx);

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n", barrierName.cstr(),
								  COLORS[batchIdx % COLORS.getSize()], barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U32 passIdx : batch.m_passIndices)
		{
			const RenderPassDescriptionBase& pass = *descr.m_passes[passIdx];
			StringAuto passName(alloc);
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
	ANKI_CHECK(
		file.open(StringAuto(alloc).sprintf("%s/rgraph_%u.dot", &path[0], m_version).toCString(), FileOpenFlag::WRITE));
	for(const String& s : slist)
	{
		ANKI_CHECK(file.writeText("%s", &s[0]));
	}

	return Error::NONE;
}
#endif

} // end namespace anki
