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

namespace anki
{

/// Contains some extra things for render targets.
class RenderGraph::RT
{
public:
	TextureUsageBit m_usage;
};

class RenderGraph::RTBarrier
{
public:
	U32 m_rtIdx;
	TextureUsageBit m_usageBefore;
	TextureUsageBit m_usageAfter;
	TextureSurfaceInfo m_surf;

	RTBarrier(U32 rtIdx, TextureUsageBit usageBefore, TextureUsageBit usageAfter, const TextureSurfaceInfo& surf)
		: m_rtIdx(rtIdx)
		, m_usageBefore(usageBefore)
		, m_usageAfter(usageAfter)
		, m_surf(surf)
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
	DynamicArrayAuto<RTBarrier> m_rtBarriersBefore;
	DynamicArrayAuto<RTBarrier> m_rtBarriersAfter;

	Batch(const StackAllocator<U8>& alloc)
		: m_passes(alloc)
		, m_rtBarriersBefore(alloc)
		, m_rtBarriersAfter(alloc)
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

Error RenderGraph::dumpDependencyDotFile(const BakeContext& ctx, CString path) const
{
	File file;
	ANKI_CHECK(file.open(StringAuto(ctx.m_alloc).sprintf("%s/rgraph.dot", &path[0]).toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("digraph {\n"));
	ANKI_CHECK(file.writeText("splines = ortho;\nconcentrate = true;\n"));

	for(U batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		ANKI_CHECK(file.writeText("\t{rank=\"same\";"));
		for(U32 passIdx : ctx.m_batches[batchIdx].m_passes)
		{
			ANKI_CHECK(file.writeText("\"%s\";", &ctx.m_descr->m_passes[passIdx]->m_name[0]));
		}
		ANKI_CHECK(file.writeText("}\n"));

		for(U32 passIdx : ctx.m_batches[batchIdx].m_passes)
		{
			for(U32 depIdx : ctx.m_passes[passIdx].m_dependsOn)
			{
				ANKI_CHECK(file.writeText("\t\"%s\"->\"%s\";\n",
					&ctx.m_descr->m_passes[depIdx]->m_name[0],
					&ctx.m_descr->m_passes[passIdx]->m_name[0]));
			}

			if(ctx.m_passes[passIdx].m_dependsOn.getSize() == 0)
			{
				ANKI_CHECK(file.writeText("\tNONE->\"%s\";\n", &ctx.m_descr->m_passes[passIdx]->m_name[0]));
			}
		}
	}

	ANKI_CHECK(file.writeText("}"));

// Barriers
#if 0
	ANKI_CHECK(file.writeText("digraph {\n"));
	ANKI_CHECK(file.writeText("splines = ortho;\nconcentrate = true;\n"));

	for(U batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		const Batch& batch = ctx.m_batches[batchIdx];
		StringAuto prevBatchName(ctx.m_alloc);
		if(batchIdx == 0)
		{
			prevBatchName.sprintf("%s", "NONE");
		}
		else
		{
			prevBatchName.sprintf("batch_%u", batchIdx - 1);
		}
	}

	ANKI_CHECK(file.writeText("}"));
#endif

	return Error::NONE;
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
	for(U i = 0; i < descr.m_renderTargets.getSize(); ++i)
	{
		ctx.m_rts[i].m_usage = descr.m_renderTargets[i].m_usage;
	}

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
					const TextureUsageBit consumerUsage = consumer.m_texture.m_usage;
					TextureUsageBit& crntUsage = ctx.m_rts[consumer.m_texture.m_handle].m_usage;

					if(crntUsage != consumerUsage)
					{
						batch.m_rtBarriersBefore.emplaceBack(
							consumer.m_texture.m_handle, crntUsage, consumerUsage, consumer.m_texture.m_surface);

						crntUsage = consumerUsage;
					}
				}
				else
				{
					// TODO
				}
			}

			// For all producers
			for(const RenderPassDependency& producer : pass.m_producers)
			{
				if(producer.m_isTexture)
				{
					const TextureUsageBit producerUsage = producer.m_texture.m_usage;
					TextureUsageBit& crntUsage = ctx.m_rts[producer.m_texture.m_handle].m_usage;

					if(crntUsage != producerUsage)
					{
						batch.m_rtBarriersAfter.emplaceBack(
							producer.m_texture.m_handle, producerUsage, crntUsage, producer.m_texture.m_surface);

						crntUsage = producerUsage;
					}
				}
				else
				{
					// TODO
				}
			}
		}
	}
}

} // end namespace anki
