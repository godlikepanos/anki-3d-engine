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

class RenderGraph::PassBatch
{
public:
	DynamicArrayAuto<Pass*> m_passes;

	PassBatch(const StackAllocator<U8>& alloc)
		: m_passes(alloc)
	{
	}
};

class RenderGraph::BakeContext
{
public:
	BakeContext(const StackAllocator<U8>& alloc)
		: m_batches(alloc)
	{
	}

	DynamicArrayAuto<PassBatch*> m_batches;
	BitSet<MAX_PASSES> m_passIsInBatch = {false};
};

RenderGraph::RenderGraph(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
	ANKI_ASSERT(manager);

	m_tmpAlloc = StackAllocator<U8>(m_gr->getAllocator().getMemoryPool().getAllocationCallback(),
		m_gr->getAllocator().getMemoryPool().getAllocationCallbackUserData(),
		512_B);
}

RenderGraph::~RenderGraph()
{
	// TODO
}

template<typename T>
void RenderGraph::increaseStorage(T*& oldStorage, U32& count, U32& storage)
{
	T* newStorage;
	if(count == storage)
	{
		storage = max<U32>(2, storage * 2);
		newStorage = m_tmpAlloc.newArray<T>(storage);
		for(U i = 0; i < count; ++i)
		{
			newStorage[i] = oldStorage[i];
		}
	}
	else
	{
		newStorage = oldStorage;
	}

	++count;
	oldStorage = newStorage;
}

void RenderGraph::reset()
{
	for(U i = 0; i < m_renderTargetsCount; ++i)
	{
		m_renderTargets[i].m_tex.reset(nullptr);
	}
	m_renderTargetsCount = 0;
	m_renderTargetsStorage = 0;

	// for(Pass* pass : m_passes)
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

		for(const Pass* pass : ctx.m_batches[batchIdx]->m_passes)
		{
			for(const Pass* dep : pass->m_dependsOn)
			{
				ANKI_CHECK(file.writeText("\t\t%s->%s;\n", &pass->m_name[0], &dep->m_name[0]));
			}
		}

		ANKI_CHECK(file.writeText("\t}\n"));
	}

	ANKI_CHECK(file.writeText("}"));
	return Error::NONE;
}

RenderGraphHandle RenderGraph::pushRenderTarget(CString name, TexturePtr tex, Bool imported)
{
	increaseStorage(m_renderTargets, m_renderTargetsCount, m_renderTargetsStorage);

	RenderTarget& target = m_renderTargets[m_renderTargetsCount - 1];
	target.m_imported = imported;
	target.m_tex = tex;

	if(name.getLength())
	{
		ANKI_ASSERT(name.getLength() <= MAX_GR_OBJECT_NAME_LENGTH);
		strcpy(&target.m_name[0], &name[0]);
	}
	else
	{
		static const char* NA = "N/A";
		strcpy(&target.m_name[0], NA);
	}

	return (m_renderTargetsCount - 1) & TEXTURE_TYPE;
}

RenderGraphHandle RenderGraph::importRenderTarget(CString name, TexturePtr tex)
{
	return pushRenderTarget(name, tex, true);
}

RenderGraphHandle RenderGraph::newRenderTarget(CString name, const TextureInitInfo& texInf)
{
	auto alloc = m_gr->getAllocator();

	// Find a cache entry
	auto it = m_renderTargetCache.find(texInf);
	RenderTargetCacheEntry* entry = nullptr;
	if(ANKI_UNLIKELY(it == m_renderTargetCache.getEnd()))
	{
		// Didn't found the entry, create a new one

		entry = alloc.newInstance<RenderTargetCacheEntry>();
		m_renderTargetCache.pushBack(alloc, texInf, entry);
	}
	else
	{
		entry = *it;
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

		tex = m_gr->newInstance<Texture>(texInf);

		ANKI_ASSERT(entry->m_texturesInUse == 0);
		entry->m_textures.resize(alloc, entry->m_textures.getSize() + 1);
		entry->m_textures[entry->m_textures.getSize() - 1] = tex;
		++entry->m_texturesInUse;
	}

	// Create the render target
	return pushRenderTarget(name, tex, false);
}

Bool RenderGraph::passADependsOnB(const Pass& a, const Pass& b)
{
	for(const RenderGraphDependency& consumer : a.m_consumers)
	{
		for(const RenderGraphDependency& producer : b.m_producers)
		{
			if(consumer.m_handle == producer.m_handle)
			{
				return true;
			}
		}
	}

	return false;
}

RenderGraphHandle RenderGraph::registerRenderPass(CString name,
	WeakArray<RenderTargetInfo> colorAttachments,
	RenderTargetInfo depthStencilAttachment,
	WeakArray<RenderGraphDependency> consumers,
	WeakArray<RenderGraphDependency> producers,
	RenderGraphWorkCallback callback,
	void* userData,
	U32 secondLevelCmdbsCount)
{
	// Allocate the new pass
	Pass* newPass = m_tmpAlloc.newInstance<Pass>();
	newPass->m_index = m_passes.getSize();

	// Set name
	if(name.getLength())
	{
		ANKI_ASSERT(name.getLength() <= MAX_GR_OBJECT_NAME_LENGTH);
		strcpy(&newPass->m_name[0], &name[0]);
	}
	else
	{
		static const char* NA = "N/A";
		strcpy(&newPass->m_name[0], NA);
	}

	// Set the dependencies
	// XXX

	// Find the dependencies
	for(Pass* prevPass : m_passes)
	{
		if(passADependsOnB(*newPass, *prevPass))
		{
			newPass->m_dependsOn.resize(m_tmpAlloc, newPass->m_dependsOn.getSize() + 1);
			newPass->m_dependsOn.getBack() = prevPass;
		}
	}

	// Push pass to the passes
	m_passes.resize(m_tmpAlloc, m_passes.getSize() + 1);
	m_passes[m_passes.getSize() - 1] = newPass;
	return (m_passes.getSize() - 1) & RT_TYPE;
}

Bool RenderGraph::passHasUnmetDependencies(const BakeContext& ctx, const Pass& pass) const
{
	Bool depends = false;

	if(ctx.m_batches.getSize() > 0)
	{
		// Check if the passes it depends on are in a batch

		for(const Pass* dep : pass.m_dependsOn)
		{
			if(ctx.m_passIsInBatch.get(dep->m_index) == false)
			{
				// Dependency pass is not in a batch
				depends = true;
				break;
			}
		}
	}
	else
	{
		// First batch, check if pass depends on any pass

		depends = pass.m_dependsOn.getSize() != 0;
	}

	return depends;
}

void RenderGraph::bake()
{
	BakeContext ctx(m_tmpAlloc);

	// Walk the graph and create pass batches
	U passesInBatchCount = 0;
	while(passesInBatchCount < m_passes.getSize())
	{
		PassBatch& batch = *m_tmpAlloc.newInstance<PassBatch>(m_tmpAlloc);

		for(U i = 0; i < m_passes.getSize(); ++i)
		{
			if(!ctx.m_passIsInBatch.get(i) && !passHasUnmetDependencies(ctx, *m_passes[i]))
			{
				// Add to the batch
				batch.m_passes.resize(batch.m_passes.getSize() + 1);
				batch.m_passes.getBack() = m_passes[i];
			}
		}

		// Push back batch
		ctx.m_batches.resize(ctx.m_batches.getSize() + 1);
		ctx.m_batches.getBack() = &batch;

		// Mark batch's passes done
		for(const Pass* pass : batch.m_passes)
		{
			ctx.m_passIsInBatch.set(pass->m_index);
		}
	}

	// Find out what barriers we need between passes
	/*for(PassBatch& batch : ctx.m_batches)
	{
		for(c : consumers)
		{
			lastProducer = findLastProducer(c);
		}


	}*/
}

} // end namespace anki
