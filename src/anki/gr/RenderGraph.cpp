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
	HashMap<TextureSurfaceInfo, TextureUsageBit> m_surfUsageMap;

	RT()
		: m_surfUsageMap(8, 4)
	{
	}
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
	DynamicArrayAuto<U32> m_dependsOn;

	Pass(const StackAllocator<U8>& alloc)
		: m_dependsOn(alloc)
	{
	}
};

/// A batch of render passes. These passes can run in parallel.
class RenderGraph::Batch
{
public:
	DynamicArrayAuto<U32> m_passes;
	DynamicArrayAuto<Barrier> m_barriersBefore;
	DynamicArrayAuto<Barrier> m_barriersAfter;

	Batch(const StackAllocator<U8>& alloc)
		: m_passes(alloc)
		, m_barriersBefore(alloc)
		, m_barriersAfter(alloc)
	{
	}
};

class RenderGraph::BakeContext
{
public:
	StackAllocator<U8> m_alloc;
	DynamicArrayAuto<Pass> m_passes;
	BitSet<MAX_RENDER_GRAPH_PASSES> m_passIsInBatch = {false};
	const RenderGraphDescription* m_descr = nullptr;
	DynamicArrayAuto<Batch> m_batches;
	DynamicArrayAuto<RT> m_rts;

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> m_satisfiedRts = {false}; ///< XXX
	BitSet<MAX_RENDER_GRAPH_BUFFERS> m_satisfiedConsumerBuffers = {false}; ///< XXX

	BakeContext(const StackAllocator<U8>& alloc)
		: m_alloc(alloc)
		, m_passes(alloc)
		, m_batches(alloc)
		, m_rts(alloc)
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
	Bool depends = false;

	/// Compute the 3 types of dependencies
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aReadBWrite = a.m_consumerRtMask & b.m_producerRtMask;
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aWriteBRead = a.m_producerRtMask & b.m_consumerRtMask;
	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> aWriteBWrite = a.m_producerRtMask & b.m_producerRtMask;

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> fullDep = aReadBWrite | aWriteBRead | aWriteBWrite;
	// fullDep &= ~ctx.m_satisfiedRts;

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
						depends = true;
						// ANKI_ASSERT(!ctx.m_satisfiedRts.get(dep.m_texture.m_handle));
						// ctx.m_satisfiedRts.set(dep.m_texture.m_handle);
					}
				}
			}
		}
	}

#if 0
	BitSet<MAX_RENDER_GRAPH_BUFFERS> bufferIntersection =
		(a.m_consumerBufferMask & ~ctx.m_satisfiedConsumerBuffers) & b.m_producerBufferMask;
	if(bufferIntersection.getAny())
	{
		// There is overlap
		for(const RenderPassDependency& dep : a.m_consumers)
		{
			if(!dep.m_isTexture && bufferIntersection.get(dep.m_bufferHandle))
			{
				depends = true;
				ANKI_ASSERT(!ctx.m_satisfiedConsumerBuffers.get(dep.m_bufferHandle));
				ctx.m_satisfiedConsumerBuffers.set(dep.m_bufferHandle);
			}
		}
	}
#endif

	return depends;
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
	BakeContext ctx(descr.m_alloc);
	ctx.m_descr = &descr;

	// Init the render targets
	ctx.m_rts.create(descr.m_renderTargets.getSize());

	// Find the dependencies between passes
	for(U i = 0; i < passCount; ++i)
	{
		ctx.m_satisfiedRts.unsetAll();
		ctx.m_satisfiedConsumerBuffers.unsetAll();

		ctx.m_passes.emplaceBack(ctx.m_alloc);
		const RenderPassBase& crntPass = *descr.m_passes[i];

		U j = i;
		while(j--)
		{
			const RenderPassBase& prevPass = *descr.m_passes[j];
			if(passADependsOnB(ctx, crntPass, prevPass))
			{
				ctx.m_passes[i].m_dependsOn.emplaceBack(j);
			}
		}
	}

	// Walk the graph and create pass batches
	U passesInBatchCount = 0;
	while(passesInBatchCount < passCount)
	{
		Batch batch(ctx.m_alloc);

		for(U i = 0; i < passCount; ++i)
		{
			if(!ctx.m_passIsInBatch.get(i) && !passHasUnmetDependencies(ctx, i))
			{
				// Add to the batch
				++passesInBatchCount;
				batch.m_passes.emplaceBack(i);
			}
		}

		// Push back batch
		ctx.m_batches.emplaceBack(std::move(batch));

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

					// Get current suage
					TextureUsageBit crntUsage;
					auto it = ctx.m_rts[rtIdx].m_surfUsageMap.find(consumer.m_texture.m_surface);
					if(it != ctx.m_rts[rtIdx].m_surfUsageMap.getEnd())
					{
						crntUsage = *it;
						*it = consumerUsage;
					}
					else
					{
						crntUsage = ctx.m_descr->m_renderTargets[rtIdx].m_usage;
						ctx.m_rts[rtIdx].m_surfUsageMap.emplace(alloc, consumer.m_texture.m_surface, consumerUsage);
					}

					if(crntUsage != consumerUsage)
					{
						batch.m_barriersBefore.emplaceBack(
							consumer.m_texture.m_handle, crntUsage, consumerUsage, consumer.m_texture.m_surface);
					}
				}
				else
				{
					// TODO
				}
			}

// For all producers, just check some stuff
#if ANKI_EXTRA_CHECKS
			for(const RenderPassDependency& producer : pass.m_producers)
			{
				if(producer.m_isTexture)
				{
					const U32 rtIdx = producer.m_texture.m_handle;

					auto it = ctx.m_rts[rtIdx].m_surfUsageMap.find(producer.m_texture.m_surface);
					ANKI_ASSERT(it != ctx.m_rts[rtIdx].m_surfUsageMap.getEnd()
						&& "There should have been a consumer that added a map entry");

					ANKI_ASSERT(*it == producer.m_texture.m_usage && "The consumer should have set that");
				}
				else
				{
					// TODO
				}
			}
#endif
		}
	}
}

StringAuto RenderGraph::textureUsageToStr(const BakeContext& ctx, TextureUsageBit usage)
{
	StringListAuto slist(ctx.m_alloc);

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

	StringAuto str(ctx.m_alloc);
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

	// Barriers
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
				barrierName.sprintf("%s barrier%u\n%s -> %s",
					&batchName[0],
					barrierIdx,
					&textureUsageToStr(ctx, barrier.m_tex.m_usageBefore).toCString()[0],
					&textureUsageToStr(ctx, barrier.m_tex.m_usageAfter).toCString()[0]);
			}
			else
			{
				// TODO
			}

			slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold,shape=box];\n", &barrierName[0], COLORS[batchIdx % 6]);
			slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", &prevBubble[0], &barrierName[0]);

			// Print render target dep
			if(barrier.m_isTexture)
			{
				StringAuto rtName(ctx.m_alloc);
				rtName.sprintf("%s\n(mip:%u dp:%u f:%u l:%u)",
					&ctx.m_descr->m_renderTargets[barrier.m_tex.m_idx].m_name[0],
					barrier.m_tex.m_surf.m_level,
					barrier.m_tex.m_surf.m_depth,
					barrier.m_tex.m_surf.m_face,
					barrier.m_tex.m_surf.m_layer);

				slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", &rtName[0], &barrierName[0]);
			}
			else
			{
				// TODO
			}

			prevBubble = barrierName;
		}

		slist.pushBackSprintf("\t\"%s\"[color=%s,style=bold];\n", &batchName[0], COLORS[batchIdx % 6]);
		slist.pushBackSprintf("\t\"%s\"->\"%s\";\n", &prevBubble[0], &batchName[0]);
		prevBubble = batchName;
	}

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
