// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/RenderGraph.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Framebuffer.h>
#include <anki/util/BitSet.h>
#include <anki/util/File.h>
#include <anki/util/StringList.h>

namespace anki
{

/// Contains some extra things for render targets.
class RenderGraph::RT
{
public:
	struct Usage
	{
		TextureUsageBit m_usage;
		TextureSurfaceInfo m_surf;
	};

	DynamicArray<Usage> m_surfUsages;

	TexturePtr m_tex; ///< Hold a reference.
};

/// Same as RT but for buffers.
class RenderGraph::Buffer
{
public:
	BufferUsageBit m_usage;
	BufferPtr m_buff; ///< Hold a reference.
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
		TextureSurfaceInfo m_surf;
	};

	struct BufferInfo
	{
		U32 m_idx;
		BufferUsageBit m_usageBefore;
		BufferUsageBit m_usageAfter;
	};

	union
	{
		TextureInfo m_tex;
		BufferInfo m_buff;
	};

	Bool8 m_isTexture;

	Barrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSurfaceInfo& surf)
		: m_tex({rtIdx, usageBefore, usageAfter, surf})
		, m_isTexture(true)
	{
	}

	Barrier(U32 buffIdx, BufferUsageBit usageBefore, BufferUsageBit usageAfter)
		: m_buff({buffIdx, usageBefore, usageAfter})
		, m_isTexture(false)
	{
	}
};

/// Contains some extra things the RenderPassBase cannot hold.
class RenderGraph::Pass
{
public:
	DynamicArray<U32> m_dependsOn;

	RenderPassWorkCallback m_callback = nullptr;
	void* m_userData = nullptr;
	U32 m_secondLevelCmdbsCount = 0;
};

/// A batch of render passes. These passes can run in parallel.
class RenderGraph::Batch
{
public:
	DynamicArray<U32> m_passes;
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

	BakeContext(const StackAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}
};

RenderGraph::RenderGraph(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
	ANKI_ASSERT(manager);
}

RenderGraph::~RenderGraph()
{
	reset();

	while(!m_renderTargetCache.isEmpty())
	{
		auto it = m_renderTargetCache.getBegin();
		RenderTargetCacheEntry& entry = *it;
		entry.m_textures.destroy(getAllocator());
		m_renderTargetCache.erase(getAllocator(), it);
	}
}

void RenderGraph::reset()
{
	// TODO

	for(RT& rt : m_ctx->m_rts)
	{
		rt.m_tex.reset(nullptr);
	}

	for(auto& it : m_renderTargetCache)
	{
		it.m_texturesInUse = 0;
	}

	m_ctx = nullptr;
}

TexturePtr RenderGraph::getOrCreateRenderTarget(const TextureInitInfo& initInf)
{
	auto alloc = getManager().getAllocator();

	// Find a cache entry
	RenderTargetCacheEntry* entry = nullptr;
	auto it = m_renderTargetCache.find(initInf);
	if(ANKI_UNLIKELY(it == m_renderTargetCache.getEnd()))
	{
		// Didn't found the entry, create a new one

		auto it2 = m_renderTargetCache.emplace(getAllocator(), initInf);
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

		ANKI_ASSERT(entry->m_texturesInUse == 0);
		entry->m_textures.resize(alloc, entry->m_textures.getSize() + 1);
		entry->m_textures[entry->m_textures.getSize() - 1] = tex;
		++entry->m_texturesInUse;
	}

	return tex;
}

Bool RenderGraph::passADependsOnB(BakeContext& ctx, const RenderPassBase& a, const RenderPassBase& b)
{
	// Render targets
	{
		/// Compute the 3 types of dependencies
		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aReadBWrite = a.m_consumerRtMask & b.m_producerRtMask;
		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aWriteBRead = a.m_producerRtMask & b.m_consumerRtMask;
		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aWriteBWrite = a.m_producerRtMask & b.m_producerRtMask;

		BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;

		if(fullDep.getAny())
		{
			// There might be an overlap

			for(const RenderPassDependency& consumer : a.m_consumers)
			{
				if(consumer.m_isTexture && fullDep.get(consumer.m_texture.m_handle))
				{
					for(const RenderPassDependency& producer : b.m_producers)
					{
						if(producer.m_isTexture && producer.m_texture.m_handle == consumer.m_texture.m_handle
							&& producer.m_texture.m_surface == consumer.m_texture.m_surface)
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
				if(!consumer.m_isTexture && fullDep.get(consumer.m_buffer.m_handle))
				{
					for(const RenderPassDependency& producer : b.m_producers)
					{
						if(!producer.m_isTexture && producer.m_buffer.m_handle == consumer.m_buffer.m_handle)
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
			ctx->m_rts[rtIdx].m_tex = descr.m_renderTargets[rtIdx].m_importedTex;
		}
		else
		{
			TexturePtr rt = getOrCreateRenderTarget(descr.m_renderTargets[rtIdx].m_initInfo);
			ctx->m_rts[rtIdx].m_tex = rt;
		}
	}

	// Buffers
	ctx->m_buffers.create(alloc, descr.m_buffers.getSize());
	for(U buffIdx = 0; buffIdx < ctx->m_buffers.getSize(); ++buffIdx)
	{
		ctx->m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
		ANKI_ASSERT(descr.m_buffers[buffIdx].m_importedBuff.isCreated());
		ctx->m_buffers[buffIdx].m_buff = descr.m_buffers[buffIdx].m_importedBuff;
	}

	return ctx;
}

void RenderGraph::compileNewGraph(const RenderGraphDescription& descr, StackAllocator<U8>& alloc)
{
	// Init the context
	BakeContext& ctx = *newContext(descr, alloc);
	m_ctx = &ctx;

	// Init and find the dependencies between passes
	const U passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);
	ctx.m_passes.create(alloc, passCount);
	for(U i = 0; i < passCount; ++i)
	{
		const RenderPassBase& inPass = *descr.m_passes[i];
		Pass& outPass = ctx.m_passes[i];

		outPass.m_callback = inPass.m_callback;
		outPass.m_userData = inPass.m_userData;
		outPass.m_secondLevelCmdbsCount = inPass.m_secondLevelCmdbsCount;

		U j = i;
		while(j--)
		{
			const RenderPassBase& prevPass = *descr.m_passes[j];
			if(passADependsOnB(ctx, inPass, prevPass))
			{
				outPass.m_dependsOn.emplaceBack(alloc, j);
			}
		}
	}

	// Walk the graph and create pass batches
	U passesInBatchCount = 0;
	while(passesInBatchCount < passCount)
	{
		Batch batch;

		for(U i = 0; i < passCount; ++i)
		{
			if(!ctx.m_passIsInBatch.get(i) && !passHasUnmetDependencies(ctx, i))
			{
				// Add to the batch
				++passesInBatchCount;
				batch.m_passes.emplaceBack(alloc, i);
			}
		}

		// Push back batch
		ctx.m_batches.emplaceBack(alloc, std::move(batch));

		// Mark batch's passes done
		for(U32 passIdx : ctx.m_batches.getBack().m_passes)
		{
			ctx.m_passIsInBatch.set(passIdx);
		}
	}

	// Create barriers between batches
	setBatchBarriers(descr, ctx);

#if 1
	if(dumpDependencyDotFile(descr, ctx, "./"))
	{
		ANKI_LOGF("Won't recover on debug code");
	}
#endif
}

void RenderGraph::setBatchBarriers(const RenderGraphDescription& descr, BakeContext& ctx) const
{
	const StackAllocator<U8>& alloc = ctx.m_alloc;

	// For all batches
	for(Batch& batch : ctx.m_batches)
	{
		// For all passes of that batch
		for(U passIdx : batch.m_passes)
		{
			const RenderPassBase& pass = *descr.m_passes[passIdx];

			// For all consumers
			for(const RenderPassDependency& consumer : pass.m_consumers)
			{
				if(consumer.m_isTexture)
				{
					const U32 rtIdx = consumer.m_texture.m_handle;
					const TextureUsageBit consumerUsage = consumer.m_texture.m_usage;

					Bool anySurfaceFound = false;
					const Bool wholeTex = consumer.m_texture.m_wholeTex;
					for(RT::Usage& u : ctx.m_rts[rtIdx].m_surfUsages)
					{
						if(wholeTex || u.m_surf == consumer.m_texture.m_surface)
						{
							anySurfaceFound = true;
							if(u.m_usage != consumerUsage)
							{
								batch.m_barriersBefore.emplaceBack(
									alloc, consumer.m_texture.m_handle, u.m_usage, consumerUsage, u.m_surf);

								u.m_usage = consumer.m_texture.m_usage;
							}
						}
					}

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
					const U32 buffIdx = consumer.m_buffer.m_handle;
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
	} // For all batches
}

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

#undef ANKI_BUFF_USAGE

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
		for(U32 passIdx : ctx.m_batches[batchIdx].m_passes)
		{
			slist.pushBackSprintf("\"%s\";", descr.m_passes[passIdx]->m_name.cstr());
		}
		slist.pushBackSprintf("}\n");

		// Print passes
		for(U32 passIdx : ctx.m_batches[batchIdx].m_passes)
		{
			CString passName = descr.m_passes[passIdx]->m_name.toCString();

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box];\n", passName.cstr(), COLORS[batchIdx % 6]);

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

	// Color the resources
	slist.pushBackSprintf("subgraph cluster_0 {\n");
	for(U rtIdx = 0; rtIdx < descr.m_renderTargets.getSize(); ++rtIdx)
	{
		slist.pushBackSprintf("\t\"%s\"[color=%s];\n", &descr.m_renderTargets[rtIdx].m_name[0], COLORS[rtIdx % 6]);
	}
	slist.pushBackSprintf("}\n");

	// Barriers
	slist.pushBackSprintf("subgraph cluster_1 {\n");
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
			if(barrier.m_isTexture)
			{
				barrierName.sprintf("%s barrier%u\n%s (mip,dp,f,l)=(%u,%u,%u,%u)\n%s -> %s",
					batchName.cstr(),
					barrierIdx,
					&descr.m_renderTargets[barrier.m_tex.m_idx].m_name[0],
					barrier.m_tex.m_surf.m_level,
					barrier.m_tex.m_surf.m_depth,
					barrier.m_tex.m_surf.m_face,
					barrier.m_tex.m_surf.m_layer,
					&textureUsageToStr(alloc, barrier.m_tex.m_usageBefore).toCString()[0],
					&textureUsageToStr(alloc, barrier.m_tex.m_usageAfter).toCString()[0]);
			}
			else
			{
				barrierName.sprintf("%s barrier%u\n%s\n%s -> %s",
					batchName.cstr(),
					barrierIdx,
					&descr.m_buffers[barrier.m_buff.m_idx].m_name[0],
					bufferUsageToStr(alloc, barrier.m_buff.m_usageBefore).toCString().cstr(),
					bufferUsageToStr(alloc, barrier.m_buff.m_usageAfter).toCString().cstr());
			}

			slist.pushBackSprintf(
				"\t\"%s\"[color=%s,style=bold,shape=box];\n", barrierName.cstr(), COLORS[batchIdx % 6]);
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", prevBubble.cstr(), barrierName.cstr());

			prevBubble = barrierName;
		}

		for(U passIdx : batch.m_passes)
		{
			const RenderPassBase& pass = *descr.m_passes[passIdx];
			StringAuto passName(alloc);
			passName.sprintf("_%s_", pass.m_name.cstr());
			slist.pushBackSprintf("\t\"pass: %s\"[color=%s,style=bold];\n", passName.cstr(), COLORS[batchIdx % 6]);
			slist.pushBackSprintf("\t\"%s\"->\"pass: %s\";\n", prevBubble.cstr(), passName.cstr());

			prevBubble = passName;
		}
	}
	slist.pushBackSprintf("}\n");

	slist.pushBackSprintf("}");

	File file;
	ANKI_CHECK(file.open(StringAuto(alloc).sprintf("%s/rgraph.dot", &path[0]).toCString(), FileOpenFlag::WRITE));
	for(const String& s : slist)
	{
		ANKI_CHECK(file.writeText("%s", &s[0]));
	}

	return Error::NONE;
}

TexturePtr RenderGraph::getTexture(RenderTargetHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_rts[handle].m_tex.isCreated());
	return m_ctx->m_rts[handle].m_tex;
}

BufferPtr RenderGraph::getBuffer(RenderPassBufferHandle handle) const
{
	ANKI_ASSERT(m_ctx->m_buffers[handle].m_buff.isCreated());
	return m_ctx->m_buffers[handle].m_buff;
}

void RenderGraph::runSecondLevel()
{
	// TODO
}

} // end namespace anki
