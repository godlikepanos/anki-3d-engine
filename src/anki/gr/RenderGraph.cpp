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

	Batch(const StackAllocator<U8>& alloc)
		: m_passes(alloc)
	{
	}
};

class RenderGraph::BakeContext
{
public:
	DynamicArrayAuto<Pass> m_passes;
	BitSet<MAX_RENDER_GRAPH_PASSES> m_passIsInBatch = {false};
	const RenderGraphDescription* m_descr = nullptr;
	DynamicArrayAuto<Batch> m_batches;

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> m_satisfiedConsumerRts = {false}; ///< XXX
	BitSet<MAX_RENDER_GRAPH_BUFFERS> m_satisfiedConsumerBuffers = {false}; ///< XXX

	BakeContext(const StackAllocator<U8>& alloc)
		: m_passes(alloc)
		, m_batches(alloc)
	{
	}
};

RenderGraph::RenderGraph(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
	ANKI_ASSERT(manager);

	m_tmpAlloc = StackAllocator<U8>(getManager().getAllocator().getMemoryPool().getAllocationCallback(),
		getManager().getAllocator().getMemoryPool().getAllocationCallbackUserData(),
		512_B);
}

RenderGraph::~RenderGraph()
{
	reset();

	while(!m_renderTargetCache.isEmpty())
	{
		auto it = m_renderTargetCache.getBegin();
		RenderTargetCacheEntry* entry = &*it;
		m_renderTargetCache.erase(it);

		entry->m_textures.destroy(getAllocator());
		getAllocator().deleteInstance(entry);
	}
}

void RenderGraph::reset()
{
	// TODO
}

Error RenderGraph::dumpDependencyDotFile(const BakeContext& ctx, CString path) const
{
	File file;
	ANKI_CHECK(file.open(StringAuto(m_tmpAlloc).sprintf("%s/rgraph.dot", &path[0]).toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("digraph {\n"));

	for(U batchIdx = 0; batchIdx < ctx.m_batches.getSize(); ++batchIdx)
	{
		ANKI_CHECK(file.writeText("\tsubgraph cluster_%u {\n", batchIdx));
		ANKI_CHECK(file.writeText("\t\tlabel=\"batch_%u\";\n", batchIdx));

		for(U32 passIdx : ctx.m_batches[batchIdx].m_passes)
		{
			for(U32 depIdx : ctx.m_passes[passIdx].m_dependsOn)
			{
				ANKI_CHECK(file.writeText("\t\t\"%s\"->\"%s\";\n",
					&ctx.m_descr->m_passes[depIdx]->m_name[0],
					&ctx.m_descr->m_passes[passIdx]->m_name[0]));
			}

			if(ctx.m_passes[passIdx].m_dependsOn.getSize() == 0)
			{
				ANKI_CHECK(file.writeText("\t\tNONE->\"%s\";\n", &ctx.m_descr->m_passes[passIdx]->m_name[0]));
			}
		}

		ANKI_CHECK(file.writeText("\t}\n"));
	}

	ANKI_CHECK(file.writeText("}"));
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

		entry = alloc.newInstance<RenderTargetCacheEntry>();
		m_renderTargetCache.pushBack(initInf, entry);
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

	BitSet<MAX_RENDER_GRAPH_RENDER_TARGETS> rtIntersection =
		(a.m_consumerRtMask & ~ctx.m_satisfiedConsumerRts) & b.m_producerRtMask;
	if(rtIntersection.getAny())
	{
		// There is overlap
		for(const RenderPassDependency& dep : a.m_consumers)
		{
			if(dep.m_isTexture && rtIntersection.get(dep.m_renderTargetHandle))
			{
				depends = true;
				ANKI_ASSERT(!ctx.m_satisfiedConsumerRts.get(dep.m_renderTargetHandle));
				ctx.m_satisfiedConsumerRts.set(dep.m_renderTargetHandle);
			}
		}
	}

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
	BakeContext ctx(m_tmpAlloc);
	ctx.m_descr = &descr;

	// Find the dependencies between passes
	for(U i = 0; i < passCount; ++i)
	{
		ctx.m_satisfiedConsumerRts.unsetAll();
		ctx.m_satisfiedConsumerBuffers.unsetAll();

		ctx.m_passes.emplaceBack(m_tmpAlloc);
		const RenderPassBase& crntPass = *descr.m_passes[i];
		for(U j = 0; j < i; ++j)
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
		Batch batch(m_tmpAlloc);

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

#if 1
	if(dumpDependencyDotFile(ctx, "./"))
	{
		ANKI_LOGF("Won't recover on debug code");
	}
#endif
}

} // end namespace anki
