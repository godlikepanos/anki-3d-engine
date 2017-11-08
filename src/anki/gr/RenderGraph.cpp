// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/RenderGraph.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/core/Trace.h>
#include <anki/util/BitSet.h>
#include <anki/util/File.h>
#include <anki/util/StringList.h>

namespace anki
{

#define ANKI_DBG_RENDER_GRAPH 0

/// Contains some extra things for render targets.
class RenderGraph::RT
{
public:
	struct Usage
	{
		TextureUsageBit m_usage;
		TextureSurfaceInfo m_surface;
	};

	DynamicArray<Usage> m_surfUsages;

	TexturePtr m_texture; ///< Hold a reference.
};

/// Same as RT but for buffers.
class RenderGraph::Buffer
{
public:
	BufferUsageBit m_usage;
	BufferPtr m_buffer; ///< Hold a reference.
};

/// Pipeline barrier of texture or buffer.
class RenderGraph::Barrier
{
public:
	struct TextureInfo
	{
		U32 m_idx;
		TextureUsageBit m_usageBefore;
		TextureUsageBit m_usageAfter;
		TextureSurfaceInfo m_surface;
	};

	struct BufferInfo
	{
		U32 m_idx;
		BufferUsageBit m_usageBefore;
		BufferUsageBit m_usageAfter;
	};

	union
	{
		TextureInfo m_texture;
		BufferInfo m_buffer;
	};

	Bool8 m_isTexture;

	Barrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSurfaceInfo& surf)
		: m_texture({rtIdx, usageBefore, usageAfter, surf})
		, m_isTexture(true)
	{
	}

	Barrier(U32 buffIdx, BufferUsageBit usageBefore, BufferUsageBit usageAfter)
		: m_buffer({buffIdx, usageBefore, usageAfter})
		, m_isTexture(false)
	{
	}
};

/// Contains some extra things the RenderPassBase cannot hold.
class RenderGraph::Pass
{
public:
	DynamicArray<U32> m_dependsOn;

	RenderPassWorkCallback m_callback;
	void* m_userData;

	DynamicArray<CommandBufferPtr> m_secondLevelCmdbs;
	FramebufferPtr m_fb;
	Array<U32, 4> m_fbRenderArea;
};

/// A batch of render passes. These passes can run in parallel.
class RenderGraph::Batch
{
public:
	DynamicArray<U32> m_passIndices;
	DynamicArray<Barrier> m_barriersBefore;
};

/// The RenderGraph build context.
class RenderGraph::BakeContext
{
public:
	StackAllocator<U8> m_alloc;
	DynamicArray<Pass> m_passes;
	BitSet<MAX_RENDER_GRAPH_PASSES> m_passIsInBatch = {false};
	DynamicArray<Batch> m_batches;
	DynamicArray<RT> m_rts;
	DynamicArray<Buffer> m_buffers;

	CommandBufferPtr m_cmdb;

	BakeContext(const StackAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}
};

void FramebufferDescription::bake()
{
	ANKI_ASSERT(m_hash == 0 && "Already baked");
	if(m_defaultFb)
	{
		m_fbInitInfo.m_colorAttachmentCount = 1;
		m_hash = 1;
		return;
	}

	// Populate the FB init info
	m_fbInitInfo.m_colorAttachmentCount = m_colorAttachmentCount;
	for(U i = 0; i < m_colorAttachmentCount; ++i)
	{
		FramebufferAttachmentInfo& out = m_fbInitInfo.m_colorAttachments[i];
		const FramebufferDescriptionAttachment& in = m_colorAttachments[i];

		out.m_surface = in.m_surface;
		out.m_clearValue = in.m_clearValue;
		out.m_loadOperation = in.m_loadOperation;
		out.m_storeOperation = in.m_storeOperation;
	}

	if(!!m_depthStencilAttachment.m_aspect)
	{
		FramebufferAttachmentInfo& out = m_fbInitInfo.m_depthStencilAttachment;
		const FramebufferDescriptionAttachment& in = m_depthStencilAttachment;

		out.m_surface = in.m_surface;
		out.m_loadOperation = in.m_loadOperation;
		out.m_storeOperation = in.m_storeOperation;
		out.m_clearValue = in.m_clearValue;

		out.m_stencilLoadOperation = in.m_stencilLoadOperation;
		out.m_stencilStoreOperation = in.m_stencilStoreOperation;

		out.m_aspect = in.m_aspect;
	}

	m_hash = 0;
	ANKI_ASSERT(m_fbInitInfo.m_colorAttachmentCount > 0 || !!m_fbInitInfo.m_depthStencilAttachment.m_aspect);

	// First the depth attachments
	if(m_fbInitInfo.m_colorAttachmentCount)
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
		for(U i = 0; i < m_fbInitInfo.m_colorAttachmentCount; ++i)
		{
			const FramebufferAttachmentInfo& inAtt = m_fbInitInfo.m_colorAttachments[i];
			colorAttachments[i].m_surf = inAtt.m_surface;
			colorAttachments[i].m_loadOp = static_cast<U32>(inAtt.m_loadOperation);
			colorAttachments[i].m_storeOp = static_cast<U32>(inAtt.m_storeOperation);
			memcpy(&colorAttachments[i].m_clearColor[0], &inAtt.m_clearValue.m_coloru[0], sizeof(U32) * 4);
		}

		m_hash = computeHash(&colorAttachments[0], sizeof(ColorAttachment) * m_fbInitInfo.m_colorAttachmentCount);
	}

	// DS attachment
	if(!!m_fbInitInfo.m_depthStencilAttachment.m_aspect)
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

		const FramebufferAttachmentInfo& inAtt = m_fbInitInfo.m_depthStencilAttachment;
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

RenderGraph::RenderGraph(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
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
}

void RenderGraph::reset()
{
	if(!m_ctx)
	{
		return;
	}

	for(RT& rt : m_ctx->m_rts)
	{
		rt.m_texture.reset(nullptr);
	}

	for(Buffer& buff : m_ctx->m_buffers)
	{
		buff.m_buffer.reset(nullptr);
	}

	for(auto& it : m_renderTargetCache)
	{
		it.m_texturesInUse = 0;
	}

	for(Pass& p : m_ctx->m_passes)
	{
		p.m_fb.reset(nullptr);
		p.m_secondLevelCmdbs.destroy(m_ctx->m_alloc);
	}

	m_ctx->m_cmdb.reset(nullptr);

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

		tex = getManager().newInstance<Texture>(initInf);

		ANKI_ASSERT(entry->m_texturesInUse == entry->m_textures.getSize());
		entry->m_textures.resize(alloc, entry->m_textures.getSize() + 1);
		entry->m_textures[entry->m_textures.getSize() - 1] = tex;
		++entry->m_texturesInUse;
	}

	return tex;
}

FramebufferPtr RenderGraph::getOrCreateFramebuffer(
	const FramebufferInitInfo& fbInit_, const RenderTargetHandle* rtHandles, U64 hash)
{
	ANKI_ASSERT(rtHandles);
	ANKI_ASSERT(hash > 0);

	const Bool defaultFb = hash == 1;

	if(!defaultFb)
	{
		// Create a hash that includes the render targets
		hash = appendHash(rtHandles, sizeof(RenderTargetHandle) * (MAX_COLOR_ATTACHMENTS + 1), hash);
	}

	FramebufferPtr fb;
	auto it = m_fbCache.find(hash);
	if(it != m_fbCache.getEnd())
	{
		fb = *it;
	}
	else
	{
		// Create a complete fb init info
		FramebufferInitInfo fbInit = fbInit_;
		if(!defaultFb)
		{
			for(U i = 0; i < fbInit.m_colorAttachmentCount; ++i)
			{
				fbInit.m_colorAttachments[i].m_texture = m_ctx->m_rts[rtHandles[i].m_idx].m_texture;
				ANKI_ASSERT(fbInit.m_colorAttachments[i].m_texture.isCreated());
			}

			if(!!fbInit.m_depthStencilAttachment.m_aspect)
			{
				fbInit.m_depthStencilAttachment.m_texture =
					m_ctx->m_rts[rtHandles[MAX_COLOR_ATTACHMENTS].m_idx].m_texture;
				ANKI_ASSERT(fbInit.m_depthStencilAttachment.m_texture.isCreated());
			}
		}

		fb = getManager().newInstance<Framebuffer>(fbInit);

		// TODO: Check why the hell it compiles if you remove the parameter "hash"
		m_fbCache.emplace(getAllocator(), hash, fb);
	}

	return fb;
}

Bool RenderGraph::overlappingDependency(const RenderPassDependency& a, const RenderPassDependency& b)
{
	if(a.m_isTexture && b.m_isTexture)
	{
		return a.m_texture.m_handle == b.m_texture.m_handle
			&& (a.m_texture.m_surface == b.m_texture.m_surface || a.m_texture.m_wholeTex || b.m_texture.m_wholeTex);
	}
	else
	{
		return a.m_isTexture == b.m_isTexture && a.m_buffer.m_handle == b.m_buffer.m_handle;
	}
}

Bool RenderGraph::passADependsOnB(const RenderPassDescriptionBase& a, const RenderPassDescriptionBase& b)
{
	// Render targets
	{
		// Compute the 3 types of dependencies
		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aReadBWrite = a.m_consumerRtMask & b.m_producerRtMask;
		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aWriteBRead = a.m_producerRtMask & b.m_consumerRtMask;
		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aWriteBWrite = a.m_producerRtMask & b.m_producerRtMask;

		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

		if(fullDep.getAny())
		{
			// There might be an overlap

			for(const RenderPassDependency& consumer : a.m_consumers)
			{
				if(consumer.m_isTexture && fullDep.get(consumer.m_texture.m_handle.m_idx))
				{
					for(const RenderPassDependency& producer : b.m_producers)
					{
						if(overlappingDependency(producer, consumer))
						{
							return true;
						}
					}
				}
			}
		}
	}

	// Buffers
	{
		BitSet<MAX_RENDER_GRAPH_BUFFERS> aReadBWrite = a.m_consumerBufferMask & b.m_producerBufferMask;
		BitSet<MAX_RENDER_GRAPH_BUFFERS> aWriteBRead = a.m_producerBufferMask & b.m_consumerBufferMask;
		BitSet<MAX_RENDER_GRAPH_BUFFERS> aWriteBWrite = a.m_producerBufferMask & b.m_producerBufferMask;

		BitSet<MAX_RENDER_GRAPH_BUFFERS> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

		if(fullDep.getAny())
		{
			// There might be an overlap

			for(const RenderPassDependency& consumer : a.m_consumers)
			{
				if(!consumer.m_isTexture && fullDep.get(consumer.m_buffer.m_handle.m_idx))
				{
					for(const RenderPassDependency& producer : b.m_producers)
					{
						if(overlappingDependency(producer, consumer))
						{
							return true;
						}
					}
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
	for(U rtIdx = 0; rtIdx < ctx->m_rts.getSize(); ++rtIdx)
	{
		if(descr.m_renderTargets[rtIdx].m_importedTex.isCreated())
		{
			// It's imported
			ctx->m_rts[rtIdx].m_texture = descr.m_renderTargets[rtIdx].m_importedTex;
		}
		else
		{
			TexturePtr rt =
				getOrCreateRenderTarget(descr.m_renderTargets[rtIdx].m_initInfo, descr.m_renderTargets[rtIdx].m_hash);
			ctx->m_rts[rtIdx].m_texture = rt;
		}
	}

	// Buffers
	ctx->m_buffers.create(alloc, descr.m_buffers.getSize());
	for(U buffIdx = 0; buffIdx < ctx->m_buffers.getSize(); ++buffIdx)
	{
		ctx->m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
		ANKI_ASSERT(descr.m_buffers[buffIdx].m_importedBuff.isCreated());
		ctx->m_buffers[buffIdx].m_buffer = descr.m_buffers[buffIdx].m_importedBuff;
	}

	return ctx;
}

void RenderGraph::initRenderPassesAndSetDeps(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	BakeContext& ctx = *m_ctx;
	const U passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	ctx.m_passes.create(alloc, passCount);
	for(U i = 0; i < passCount; ++i)
	{
		const RenderPassDescriptionBase& inPass = *descr.m_passes[i];
		Pass& outPass = ctx.m_passes[i];

		outPass.m_callback = inPass.m_callback;
		outPass.m_userData = inPass.m_userData;

		// Create command buffers and framebuffer
		if(inPass.m_type == RenderPassDescriptionBase::Type::GRAPHICS)
		{
			const GraphicsRenderPassDescription& graphicsPass =
				static_cast<const GraphicsRenderPassDescription&>(inPass);
			if(graphicsPass.hasFramebuffer())
			{
				outPass.m_fb = getOrCreateFramebuffer(
					graphicsPass.m_fbInitInfo, &graphicsPass.m_rtHandles[0], graphicsPass.m_fbHash);

				outPass.m_fbRenderArea = graphicsPass.m_fbRenderArea;

				// Create the second level command buffers
				if(inPass.m_secondLevelCmdbsCount)
				{
					outPass.m_secondLevelCmdbs.create(alloc, inPass.m_secondLevelCmdbsCount);
					CommandBufferInitInfo cmdbInit;
					cmdbInit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SECOND_LEVEL;
					cmdbInit.m_framebuffer = outPass.m_fb;
					for(U cmdbIdx = 0; cmdbIdx < inPass.m_secondLevelCmdbsCount; ++cmdbIdx)
					{
						outPass.m_secondLevelCmdbs[cmdbIdx] = getManager().newInstance<CommandBuffer>(cmdbInit);
					}
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

		U j = i;
		while(j--)
		{
			const RenderPassDescriptionBase& prevPass = *descr.m_passes[j];
			if(passADependsOnB(inPass, prevPass))
			{
				outPass.m_dependsOn.emplaceBack(alloc, j);
			}
		}
	}
}

void RenderGraph::initBatches()
{
	ANKI_ASSERT(m_ctx);

	U passesInBatchCount = 0;
	const U passCount = m_ctx->m_passes.getSize();
	ANKI_ASSERT(passCount > 0);
	while(passesInBatchCount < passCount)
	{
		Batch batch;

		for(U i = 0; i < passCount; ++i)
		{
			if(!m_ctx->m_passIsInBatch.get(i) && !passHasUnmetDependencies(*m_ctx, i))
			{
				// Add to the batch
				++passesInBatchCount;
				batch.m_passIndices.emplaceBack(m_ctx->m_alloc, i);
			}
		}

		// Push back batch
		m_ctx->m_batches.emplaceBack(m_ctx->m_alloc, std::move(batch));

		// Mark batch's passes done
		for(U32 passIdx : m_ctx->m_batches.getBack().m_passIndices)
		{
			m_ctx->m_passIsInBatch.set(passIdx);
		}
	}
}

void RenderGraph::setBatchBarriers(const RenderGraphDescription& descr, BakeContext& ctx) const
{
	const StackAllocator<U8>& alloc = ctx.m_alloc;

	// For all batches
	for(Batch& batch : ctx.m_batches)
	{
		// For all passes of that batch
		for(U passIdx : batch.m_passIndices)
		{
			const RenderPassDescriptionBase& pass = *descr.m_passes[passIdx];

			// For all consumers
			for(const RenderPassDependency& consumer : pass.m_consumers)
			{
				if(consumer.m_isTexture)
				{
					const U32 rtIdx = consumer.m_texture.m_handle.m_idx;
					const TextureUsageBit consumerUsage = consumer.m_texture.m_usage;

					Bool anySurfaceFound = false;
					const Bool wholeTex = consumer.m_texture.m_wholeTex;
					for(RT::Usage& u : ctx.m_rts[rtIdx].m_surfUsages)
					{
						if(wholeTex || u.m_surface == consumer.m_texture.m_surface)
						{
							anySurfaceFound = true;
							if(u.m_usage != consumerUsage)
							{
								batch.m_barriersBefore.emplaceBack(
									alloc, consumer.m_texture.m_handle.m_idx, u.m_usage, consumerUsage, u.m_surface);

								u.m_usage = consumer.m_texture.m_usage;
							}
						}
					}

					// Create the transition from the initial state
					if(!anySurfaceFound && descr.m_renderTargets[rtIdx].m_usage != consumerUsage)
					{
						batch.m_barriersBefore.emplaceBack(alloc,
							rtIdx,
							descr.m_renderTargets[rtIdx].m_usage,
							consumerUsage,
							consumer.m_texture.m_surface);

						RT::Usage usage{consumerUsage, consumer.m_texture.m_surface};
						ctx.m_rts[rtIdx].m_surfUsages.emplaceBack(alloc, usage);
					}
				}
				else
				{
					const U32 buffIdx = consumer.m_buffer.m_handle.m_idx;
					const BufferUsageBit consumerUsage = consumer.m_buffer.m_usage;

					if(consumerUsage != ctx.m_buffers[buffIdx].m_usage)
					{
						batch.m_barriersBefore.emplaceBack(
							alloc, buffIdx, ctx.m_buffers[buffIdx].m_usage, consumerUsage);

						ctx.m_buffers[buffIdx].m_usage = consumerUsage;
					}
				}
			} // For all consumers
		} // For all passes

#if ANKI_DBG_RENDER_GRAPH
		// Sort the barriers to ease the dumped graph
		std::sort(batch.m_barriersBefore.getBegin(),
			batch.m_barriersBefore.getEnd(),
			[&](const Barrier& a, const Barrier& b) {
				const U aidx = (a.m_isTexture) ? a.m_texture.m_idx : a.m_buffer.m_idx;
				const U bidx = (b.m_isTexture) ? b.m_texture.m_idx : b.m_buffer.m_idx;

				return aidx < bidx;
			});
#endif
	} // For all batches
}

void RenderGraph::compileNewGraph(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH);

	// Init the context
	BakeContext& ctx = *newContext(descr, alloc);
	m_ctx = &ctx;

	// Init the passes and find the dependencies between passes
	initRenderPassesAndSetDeps(descr, alloc);

	// Walk the graph and create pass batches
	initBatches();

	// Create barriers between batches
	setBatchBarriers(descr, ctx);

	// Create main command buffer
	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::COMPUTE_WORK;
	m_ctx->m_cmdb = getManager().newInstance<CommandBuffer>(cmdbInit);

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

BufferPtr RenderGraph::getBuffer(RenderPassBufferHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_buffers[handle.m_idx].m_buffer.isCreated());
	return m_ctx->m_buffers[handle.m_idx].m_buffer;
}

void RenderGraph::runSecondLevel(U32 threadIdx) const
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH);
	ANKI_ASSERT(m_ctx);
	for(const Pass& p : m_ctx->m_passes)
	{
		const U size = p.m_secondLevelCmdbs.getSize();
		if(threadIdx < size)
		{
			// TODO Inform about texture usage
			ANKI_ASSERT(p.m_secondLevelCmdbs[threadIdx].isCreated());
			p.m_callback(p.m_userData, p.m_secondLevelCmdbs[threadIdx], threadIdx, size, *this);
		}
	}
}

void RenderGraph::run() const
{
	ANKI_TRACE_SCOPED_EVENT(GR_RENDER_GRAPH);
	ANKI_ASSERT(m_ctx);
	CommandBufferPtr& cmdb = m_ctx->m_cmdb;

	for(const Batch& batch : m_ctx->m_batches)
	{
		// Set the barriers
		for(const Barrier& barrier : batch.m_barriersBefore)
		{
			if(barrier.m_isTexture)
			{
				cmdb->setTextureSurfaceBarrier(m_ctx->m_rts[barrier.m_texture.m_idx].m_texture,
					barrier.m_texture.m_usageBefore,
					barrier.m_texture.m_usageAfter,
					barrier.m_texture.m_surface);
			}
			else
			{
				cmdb->setBufferBarrier(m_ctx->m_buffers[barrier.m_buffer.m_idx].m_buffer,
					barrier.m_buffer.m_usageBefore,
					barrier.m_buffer.m_usageAfter,
					0,
					MAX_PTR_SIZE);
			}
		}

		// Call the passes
		for(U passIdx : batch.m_passIndices)
		{
			const Pass& pass = m_ctx->m_passes[passIdx];

			if(pass.m_fb.isCreated())
			{
				cmdb->beginRenderPass(pass.m_fb,
					pass.m_fbRenderArea[0],
					pass.m_fbRenderArea[1],
					pass.m_fbRenderArea[2],
					pass.m_fbRenderArea[3]);
			}

			const U size = pass.m_secondLevelCmdbs.getSize();
			if(size == 0)
			{
				pass.m_callback(pass.m_userData, cmdb, 0, 0, *this);
			}
			else
			{
				for(const CommandBufferPtr& cmdb2nd : pass.m_secondLevelCmdbs)
				{
					cmdb->pushSecondLevelCommandBuffer(cmdb2nd);
				}
			}

			if(pass.m_fb.isCreated())
			{
				cmdb->endRenderPass();
			}
		}
	}
}

void RenderGraph::flush()
{
	m_ctx->m_cmdb->flush();
}

#if ANKI_DBG_RENDER_GRAPH
StringAuto RenderGraph::textureUsageToStr(StackAllocator<U8>& alloc, TextureUsageBit usage)
{
	StringListAuto slist(alloc);

#define ANKI_TEX_USAGE(u)                \
	if(!!(usage & TextureUsageBit::u))   \
	{                                    \
		slist.pushBackSprintf("%s", #u); \
	}

	ANKI_TEX_USAGE(SAMPLED_VERTEX);
	ANKI_TEX_USAGE(SAMPLED_TESSELLATION_CONTROL);
	ANKI_TEX_USAGE(SAMPLED_TESSELLATION_EVALUATION);
	ANKI_TEX_USAGE(SAMPLED_GEOMETRY);
	ANKI_TEX_USAGE(SAMPLED_FRAGMENT);
	ANKI_TEX_USAGE(SAMPLED_COMPUTE);
	ANKI_TEX_USAGE(IMAGE_COMPUTE_READ);
	ANKI_TEX_USAGE(IMAGE_COMPUTE_WRITE);
	ANKI_TEX_USAGE(FRAMEBUFFER_ATTACHMENT_READ);
	ANKI_TEX_USAGE(FRAMEBUFFER_ATTACHMENT_WRITE);
	ANKI_TEX_USAGE(TRANSFER_DESTINATION);
	ANKI_TEX_USAGE(GENERATE_MIPMAPS);
	ANKI_TEX_USAGE(CLEAR);

	if(!usage)
	{
		slist.pushBackSprintf("NONE");
	}

#undef ANKI_TEX_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringAuto str(alloc);
	slist.join(" | ", str);
	return str;
}

StringAuto RenderGraph::bufferUsageToStr(StackAllocator<U8>& alloc, BufferUsageBit usage)
{
	StringListAuto slist(alloc);

#define ANKI_BUFF_USAGE(u)               \
	if(!!(usage & BufferUsageBit::u))    \
	{                                    \
		slist.pushBackSprintf("%s", #u); \
	}

	ANKI_BUFF_USAGE(UNIFORM_VERTEX);
	ANKI_BUFF_USAGE(UNIFORM_TESSELLATION_EVALUATION);
	ANKI_BUFF_USAGE(UNIFORM_TESSELLATION_CONTROL);
	ANKI_BUFF_USAGE(UNIFORM_GEOMETRY);
	ANKI_BUFF_USAGE(UNIFORM_FRAGMENT);
	ANKI_BUFF_USAGE(UNIFORM_COMPUTE);
	ANKI_BUFF_USAGE(STORAGE_VERTEX_READ);
	ANKI_BUFF_USAGE(STORAGE_VERTEX_WRITE);
	ANKI_BUFF_USAGE(STORAGE_TESSELLATION_EVALUATION_READ);
	ANKI_BUFF_USAGE(STORAGE_TESSELLATION_EVALUATION_WRITE);
	ANKI_BUFF_USAGE(STORAGE_TESSELLATION_CONTROL_READ);
	ANKI_BUFF_USAGE(STORAGE_TESSELLATION_CONTROL_WRITE);
	ANKI_BUFF_USAGE(STORAGE_GEOMETRY_READ);
	ANKI_BUFF_USAGE(STORAGE_GEOMETRY_WRITE);
	ANKI_BUFF_USAGE(STORAGE_FRAGMENT_READ);
	ANKI_BUFF_USAGE(STORAGE_FRAGMENT_WRITE);
	ANKI_BUFF_USAGE(STORAGE_COMPUTE_READ);
	ANKI_BUFF_USAGE(STORAGE_COMPUTE_WRITE);
	ANKI_BUFF_USAGE(TEXTURE_VERTEX_READ);
	ANKI_BUFF_USAGE(TEXTURE_TESSELLATION_EVALUATION_READ);
	ANKI_BUFF_USAGE(TEXTURE_TESSELLATION_CONTROL_READ);
	ANKI_BUFF_USAGE(TEXTURE_GEOMETRY_READ);
	ANKI_BUFF_USAGE(TEXTURE_FRAGMENT_READ);
	ANKI_BUFF_USAGE(TEXTURE_COMPUTE_READ);
	ANKI_BUFF_USAGE(INDEX);
	ANKI_BUFF_USAGE(VERTEX);
	ANKI_BUFF_USAGE(INDIRECT);
	ANKI_BUFF_USAGE(FILL);
	ANKI_BUFF_USAGE(BUFFER_UPLOAD_SOURCE);
	ANKI_BUFF_USAGE(BUFFER_UPLOAD_DESTINATION);
	ANKI_BUFF_USAGE(TEXTURE_UPLOAD_SOURCE);
	ANKI_BUFF_USAGE(QUERY_RESULT);

	if(!usage)
	{
		slist.pushBackSprintf("NONE");
	}

#undef ANKI_BUFF_USAGE

	ANKI_ASSERT(!slist.isEmpty());
	StringAuto str(alloc);
	slist.join(" | ", str);
	return str;
}

Error RenderGraph::dumpDependencyDotFile(
	const RenderGraphDescription& descr, const BakeContext& ctx, CString path) const
{
	static const Array<const char*, 6> COLORS = {{"red", "green", "blue", "magenta", "yellow", "cyan"}};
	auto alloc = ctx.m_alloc;
	StringListAuto slist(alloc);

	slist.pushBackSprintf("digraph {\n");
	slist.pushBackSprintf("\t//splines = ortho;\nconcentrate = true;\n");

	for(U batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
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

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=%s,shape=box];\n",
				passName.cstr(),
				COLORS[batchIdx % 6],
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

#if 0
	// Color the resources
	slist.pushBackSprintf("subgraph cluster_0 {\n");
	for(U rtIdx = 0; rtIdx < descr.m_renderTargets.getSize(); ++rtIdx)
	{
		slist.pushBackSprintf("\t\"%s\"[color=%s];\n", &descr.m_renderTargets[rtIdx].m_name[0], COLORS[rtIdx % 6]);
	}
	slist.pushBackSprintf("}\n");
#endif

	// Barriers
	// slist.pushBackSprintf("subgraph cluster_1 {\n");
	StringAuto prevBubble(ctx.m_alloc);
	prevBubble.create("START");
	for(U batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		const Batch& batch = ctx.m_batches[batchIdx];

		StringAuto batchName(ctx.m_alloc);
		batchName.sprintf("batch%u", batchIdx);

		for(U barrierIdx = 0; barrierIdx < batch.m_barriersBefore.getSize(); ++barrierIdx)
		{
			const Barrier& barrier = batch.m_barriersBefore[barrierIdx];
			StringAuto barrierName(ctx.m_alloc);
			StringAuto barrierLabel(ctx.m_alloc);
			if(barrier.m_isTexture)
			{
				barrierLabel.sprintf("<b>%s</b> (mip,dp,f,l)=(%u,%u,%u,%u)<br/>%s <b>-&gt;</b> %s",
					&descr.m_renderTargets[barrier.m_texture.m_idx].m_name[0],
					barrier.m_texture.m_surface.m_level,
					barrier.m_texture.m_surface.m_depth,
					barrier.m_texture.m_surface.m_face,
					barrier.m_texture.m_surface.m_layer,
					textureUsageToStr(alloc, barrier.m_texture.m_usageBefore).cstr(),
					textureUsageToStr(alloc, barrier.m_texture.m_usageAfter).cstr());

				barrierName.sprintf("%s barrier%u", batchName.cstr(), barrierIdx);
			}
			else
			{
				barrierLabel.sprintf("<b>%s</b><br/>%s <b>-&gt;</b> %s",
					&descr.m_buffers[barrier.m_buffer.m_idx].m_name[0],
					bufferUsageToStr(alloc, barrier.m_buffer.m_usageBefore).cstr(),
					bufferUsageToStr(alloc, barrier.m_buffer.m_usageAfter).cstr());

				barrierName.sprintf("%s barrier%u", batchName.cstr(), barrierIdx);
			}

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box,label=< %s >];\n",
				barrierName.cstr(),
				COLORS[batchIdx % 6],
				barrierLabel.cstr());
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U passIdx : batch.m_passIndices)
		{
			const RenderPassDescriptionBase& pass = *descr.m_passes[passIdx];
			StringAuto passName(alloc);
			passName.sprintf("%s pass", pass.m_name.cstr());
			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold];\n", passName.cstr(), COLORS[batchIdx % 6]);
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
