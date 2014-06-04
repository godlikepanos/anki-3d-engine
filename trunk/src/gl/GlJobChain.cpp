#include "anki/gl/GlJobChain.h"
#include "anki/gl/GlJobManager.h"
#include "anki/gl/GlManager.h"
#include "anki/gl/GlError.h"
#include "anki/core/Logger.h"
#include "anki/core/Counters.h"
#include <cstring>

namespace anki {

//==============================================================================
GlJobChain::GlJobChain(GlJobManager* server, const GlJobChainInitHints& hints)
	:	m_server(server),
		m_alloc(GlJobChainAllocator<GlJob*>(ChainMemoryPool(
			hints.m_chunkSize, 
			hints.m_maxChunkSize, 
			ChainMemoryPool::ADD,
			hints.m_chunkSize)))
{
	//std::cout << hints.m_chunkSize << std::endl;
	ANKI_ASSERT(m_server);
}

//==============================================================================
GlJobChain& GlJobChain::operator=(GlJobChain&& b)
{
	destroy();
	m_server = b.m_server; 
	b.m_server = nullptr;
	
	m_firstJob = b.m_firstJob;
	b.m_firstJob = nullptr;
	
	m_lastJob = b.m_lastJob;
	b.m_lastJob = nullptr;

	m_alloc = b.m_alloc;
	b.m_alloc = GlJobChainAllocator<U8>();

	m_immutable = b.m_immutable;
	b.m_immutable = false;

	return *this;
}

//==============================================================================
void GlJobChain::destroy()
{
#if ANKI_DEBUG
	if(!m_executed && m_firstJob)
	{
		ANKI_LOGW("Chain contains jobs but never executed. "
			"This should only happen on exceptions");
	}
#endif

	GlJob* job = m_firstJob;
	while(job != nullptr)
	{
		GlJob* next = job->nextJob; // Get next before deleting
		m_alloc.deleteInstance(job);
		job = next;
	}

	ANKI_ASSERT(m_alloc.getMemoryPool().getUsersCount() == 1 
		&& "Someone is holding a reference to the chain's allocator");

	m_alloc = GlJobChainAllocator<U8>();
}

//==============================================================================
GlGlobalHeapAllocator<U8> GlJobChain::getGlobalAllocator() const
{
	return m_server->getManager()._getAllocator();
}

//==============================================================================
void GlJobChain::executeAllJobs()
{
	ANKI_ASSERT(m_firstJob != nullptr && "Empty job chain");
	ANKI_ASSERT(m_lastJob != nullptr && "Empty job chain");
#if ANKI_DEBUG
	m_executed = true;
#endif
	
	GlJob* job = m_firstJob;

	while(job != nullptr)
	{
		(*job)(this);
		ANKI_CHECK_GL_ERROR();

		job = job->nextJob;
	}
}

//==============================================================================
GlJobChainInitHints GlJobChain::computeInitHints() const
{
	GlJobChainInitHints out;
	out.m_chunkSize = m_alloc.getMemoryPool().getAllocatedSize() + 16;

	ANKI_COUNTER_INC(GL_JOB_CHAINS_SIZE, 
		U64(m_alloc.getMemoryPool().getAllocatedSize()));

	return out;
}

} // end namespace anki
