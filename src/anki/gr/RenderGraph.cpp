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
};

/// Same as RT but for buffers.
class RenderGraph::Buffer
{
public:
	BufferUsageBit m_usage;
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
};

/// A batch of render passes. These passes can run in parallel.
class RenderGraph::Batch
{
public:
	DynamicArray<U32> m_passes;
	DynamicArray<Barrier> m_barriersBefore;
};

class RenderGraph::BakeContext
{
public:
	StackAllocator<U8> m_alloc;
	DynamicArray<Pass> m_passes;
	BitSet<MAX_RENDER_GRAPH_PASSES> m_passIsInBatch = {false};
	const RenderGraphDescription* m_descr = nullptr;
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

void RenderGraph::compileNewGraph(const RenderGraphDescription& descr)
{
	const U passCount = descr.m_passes.getSize();
	ANKI_ASSERT(passCount > 0);

	// Init the context
	auto alloc = descr.m_alloc;
	BakeContext& ctx = *alloc.newInstance<BakeContext>(descr.m_alloc);
	m_ctx = &ctx;
	ctx.m_descr = &descr;

	// Init the resources
	ctx.m_rts.create(alloc, descr.m_renderTargets.getSize());
	ctx.m_buffers.create(alloc, descr.m_buffers.getSize());
	for(U buffIdx = 0; buffIdx < ctx.m_buffers.getSize(); ++buffIdx)
	{
		ctx.m_buffers[buffIdx].m_usage = descr.m_buffers[buffIdx].m_usage;
	}

	// Find the dependencies between passes
	for(U i = 0; i < passCount; ++i)
	{
		ctx.m_passes.emplaceBack(alloc);
		const RenderPassBase& crntPass = *descr.m_passes[i];

		U j = i;
		while(j--)
		{
			const RenderPassBase& prevPass = *descr.m_passes[j];
			if(passADependsOnB(ctx, crntPass, prevPass))
			{
				ctx.m_passes[i].m_dependsOn.emplaceBack(alloc, j);
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
	setBatchBarriers(ctx);

#if 1
	if(dumpDependencyDotFile(ctx, "./"))
	{
		ANKI_LOGF("Won't recover on debug code");
	}
#endif
}

void RenderGraph::setBatchBarriers(BakeContext& ctx) const
{
	const StackAllocator<U8>& alloc = ctx.m_alloc;

	// For all batches
	for(Batch& batch : ctx.m_batches)
	{
		// For all passes of that batch
		for(U passIdx : batch.m_passes)
		{
			const RenderPassBase& pass = *ctx.m_descr->m_passes[passIdx];

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

					if(!anySurfaceFound && ctx.m_descr->m_renderTargets[rtIdx].m_usage != consumerUsage)
					{
						batch.m_barriersBefore.emplaceBack(alloc,
							rtIdx,
							ctx.m_descr->m_renderTargets[rtIdx].m_usage,
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
			}
		}
	}
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

Error RenderGraph::dumpDependencyDotFile(const BakeContext& ctx, CString path) const
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
			slist.pushBackSprintf("\"%s\";", &ctx.m_descr->m_passes[passIdx]->m_name[0]);
		}
		slist.pushBackSprintf("}\n");

		// Print passes
		for(U32 passIdx : ctx.m_batches[batchIdx].m_passes)
		{
			CString passName = ctx.m_descr->m_passes[passIdx]->m_name.toCString();

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box];\n", &passName[0], COLORS[batchIdx % 6]);

			for(U32 depIdx : ctx.m_passes[passIdx].m_dependsOn)
			{
				slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", &ctx.m_descr->m_passes[depIdx]->m_name[0], &passName[0]);
			}

			if(ctx.m_passes[passIdx].m_dependsOn.getSize() == 0)
			{
				slist.pushBackSprintf("\tNONE->\"%s\";\n", &ctx.m_descr->m_passes[passIdx]->m_name[0]);
			}
		}
	}

	// Color the resources
	slist.pushBackSprintf("subgraph cluster_0 {\n");
	for(U rtIdx = 0; rtIdx < ctx.m_descr->m_renderTargets.getSize(); ++rtIdx)
	{
		slist.pushBackSprintf(
			"\t\"%s\"[color=%s];\n", &ctx.m_descr->m_renderTargets[rtIdx].m_name[0], COLORS[rtIdx % 6]);
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
					&batchName[0],
					barrierIdx,
					&ctx.m_descr->m_renderTargets[barrier.m_tex.m_idx].m_name[0],
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
					&batchName[0],
					barrierIdx,
					&ctx.m_descr->m_buffers[barrier.m_buff.m_idx].m_name[0],
					&bufferUsageToStr(alloc, barrier.m_buff.m_usageBefore).toCString()[0],
					&bufferUsageToStr(alloc, barrier.m_buff.m_usageAfter).toCString()[0]);
			}

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box];\n", &barrierName[0], COLORS[batchIdx % 6]);
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", &prevBubble[0], &barrierName[0]);

			prevBubble = barrierName;
		}

		for(U passIdx : batch.m_passes)
		{
			const RenderPassBase& pass = *ctx.m_descr->m_passes[passIdx];
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

} // end namespace anki
