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

/// Render pass or compute job.
class RenderGraph::Pass
{
public:
	FramebufferPtr m_framebuffer;

	Storage<RenderGraphDependency> m_consumers;
	Storage<RenderGraphDependency> m_producers;

	Storage<Pass*> m_dependsOn;

	U32 m_index;
	Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
};

/// Render target.
class RenderGraph::RenderTarget
{
public:
	TexturePtr m_tex;
	Bool8 m_imported = false;
	Array<char, MAX_GR_OBJECT_NAME_LENGTH + 1> m_name;
};

/// A collection of passes that can execute in parallel.
class RenderGraph::PassBatch
{
public:
	Storage<Pass*> m_passes;
};

class RenderGraph::BakeContext
{
public:
	Storage<PassBatch> m_batches;

	BitSet<MAX_PASSES> m_passIsInBatch = {false};
};

template<typename T>
void RenderGraph::Storage<T>::pushBack(StackAllocator<U8>& alloc, T&& x)
{
	if(m_count == m_storage)
	{
		m_storage = max<U32>(2, m_storage * 2);
		T* newStorage = alloc.newArray<T>(m_storage);
		for(U i = 0; i < m_count; ++i)
		{
			newStorage[i] = std::move(m_arr[i]);
		}

		if(m_count)
		{
			alloc.deleteArray(m_arr, m_count);
		}

		m_arr = newStorage;
	}

	m_arr[m_count] = std::move(x);
	++m_count;
}

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

void RenderGraph::reset()
{
	for(RenderTarget& rt : m_renderTargets)
	{
		rt.m_tex.reset(nullptr);
	}
	m_renderTargets.reset();

	for(Pass& pass : m_passes)
	{
		pass.m_framebuffer.reset(nullptr);
	}
	m_passes.reset();
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

		for(const Pass* pass : ctx.m_batches[batchIdx].m_passes)
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
	RenderTarget target;
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

	m_renderTargets.pushBack(m_tmpAlloc, std::move(target));
	return (m_renderTargets.getSize() - 1) & TEXTURE_TYPE;
}

RenderGraphHandle RenderGraph::importRenderTarget(CString name, TexturePtr tex)
{
	return pushRenderTarget(name, tex, true);
}

RenderGraphHandle RenderGraph::newRenderTarget(const TextureInitInfo& texInf)
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
	return pushRenderTarget(texInf.getName(), tex, false);
}

Bool RenderGraph::passADependsOnB(const Pass& a, const Pass& b)
{
	for(const RenderGraphDependency& consumer : a.m_consumers)
	{
		for(const RenderGraphDependency& producer : a.m_producers)
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
	Pass newPass;
	newPass.m_index = m_passes.getSize();

	// Set name
	if(name.getLength())
	{
		ANKI_ASSERT(name.getLength() <= MAX_GR_OBJECT_NAME_LENGTH);
		strcpy(&newPass.m_name[0], &name[0]);
	}
	else
	{
		static const char* NA = "N/A";
		strcpy(&newPass.m_name[0], NA);
	}

	// Set the dependencies
	if(consumers.getSize())
	{
		newPass.m_consumers.m_arr = m_tmpAlloc.newArray<RenderGraphDependency>(consumers.getSize());
		for(U i = 0; i < consumers.getSize(); ++i)
		{
			newPass.m_consumers.m_arr[i] = consumers[i];
		}

		newPass.m_consumers.m_count = newPass.m_consumers.m_storage = consumers.getSize();
	}

	if(producers.getSize())
	{
		newPass.m_producers.m_arr = m_tmpAlloc.newArray<RenderGraphDependency>(producers.getSize());
		for(U i = 0; i < producers.getSize(); ++i)
		{
			newPass.m_producers.m_arr[i] = producers[i];
		}

		newPass.m_producers.m_count = newPass.m_producers.m_storage = producers.getSize();
	}

	// Find the dependencies
	for(Pass& otherPass : m_passes)
	{
		if(passADependsOnB(newPass, otherPass))
		{
			newPass.m_dependsOn.pushBack(m_tmpAlloc, &otherPass);
		}
	}

	// Push pass to the passes
	m_passes.pushBack(m_tmpAlloc, std::move(newPass));
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
	BakeContext ctx;

	// Walk the graph and create pass batches
	U passesInBatchCount = 0;
	while(passesInBatchCount < m_passes.getSize())
	{
		PassBatch batch;

		for(U i = 0; i < m_passes.getSize(); ++i)
		{
			if(!ctx.m_passIsInBatch.get(i) && !passHasUnmetDependencies(ctx, m_passes[i]))
			{
				// Add to the batch
				batch.m_passes.pushBack(m_tmpAlloc, &m_passes[i]);
			}
		}

		// Push back batch
		ctx.m_batches.pushBack(m_tmpAlloc, std::move(batch));

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
