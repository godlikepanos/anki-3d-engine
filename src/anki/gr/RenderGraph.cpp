// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/RenderGraph.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Texture.h>

namespace anki
{

RenderGraph::RenderGraph(GrManager* gr)
	: m_gr(gr)
{
	ANKI_ASSERT(gr);
}

RenderGraphHandle RenderGraph::importRenderTarget(CString name, TexturePtr tex)
{
	const PtrSize crntSize = m_renderTargets.getSize();
	m_renderTargets.resize(m_gr->getAllocator(), crntSize + 1);

	RenderTarget& target = m_renderTargets.getBack();
	target.m_imported = true;
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

	return crntSize;
}

void RenderGraph::bake()
{
	// Walk the graph and create pass batches
	/*
	DynamicArray<PassBatch> batches;
	List<Pass> remainingPasses = m_passes;
	while(!remainingPasses.isEmpty())
	{
		PassBatch newBatch;

		for(pass : remainingPasses)
		{
			if(pass.noDependencies() || pass.fullfiledDependencies())
			{
				newBatch.pushBack(pass);
				remainingPasses.erase(pass);
			}
		}
	}
	*/

	// Find out what barriers we need between passes
	/*
	for(batch : batches)
	{
		consumers = gatherConsumers(batch);

		for(c : consumers)
		{
			lastProducer = findLastProducer(c);
		}


	}
	*/
}

} // end namespace anki
