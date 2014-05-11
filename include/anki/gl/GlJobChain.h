#ifndef ANKI_GL_GL_JOB_CHAIN_H
#define ANKI_GL_GL_JOB_CHAIN_H

#include "anki/gl/GlCommon.h"
#include "anki/util/Assert.h"
#include "anki/util/Allocator.h"

namespace anki {

// Forward 
class GlJobManager;
class GlJobChain;

/// @addtogroup opengl_private
/// @{

/// The base of all GL jobs
class GlJob
{
public:
	GlJob* nextJob = nullptr;

	virtual ~GlJob()
	{}

	/// Execute job
	virtual void operator()(GlJobChain*) = 0;
};

/// A common job that deletes an object
template<typename T, typename TAlloc>
class GlDeleteObjectJob: public GlJob
{
public:
	T* m_ptr;
	TAlloc m_alloc;

	GlDeleteObjectJob(T* ptr, TAlloc alloc)
		: m_ptr(ptr), m_alloc(alloc)
	{
		ANKI_ASSERT(m_ptr);
	}

	void operator()(GlJobChain*)
	{
		m_alloc.deleteInstance(m_ptr);
	}
};

/// Job chain initialization hints. They are used to optimize the allocators
/// of a job chain
class GlJobChainInitHints
{
	friend class GlJobChain;

private:
	static const PtrSize m_maxChunkSize = 1024 * 1024; // 1MB

	PtrSize m_chunkSize = 1024;
};

/// A number of GL jobs organized in a chain
class GlJobChain: public NonCopyable
{
public:
	/// Default constructor
	/// @param server The job chains server
	/// @param hints Hints to optimize the job's allocator
	GlJobChain(GlJobManager* server, const GlJobChainInitHints& hints);

	/// Move
	GlJobChain(GlJobChain&& b)
	{
		*this = std::move(b);
	}

	~GlJobChain()
	{
		destroy();
	}

	/// Move
	GlJobChain& operator=(GlJobChain&& b);

	/// Get he allocator
	GlJobChainAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	GlGlobalHeapAllocator<U8> getGlobalAllocator() const;

	GlJobManager& getJobManager()
	{
		return *m_server;
	}

	const GlJobManager& getJobManager() const
	{
		return *m_server;
	}

	/// Compute initialization hints
	GlJobChainInitHints computeInitHints() const;

	/// Create a new job and add it to the chain
	template<typename TJob, typename... TArgs>
	void pushBackNewJob(TArgs&&... args)
	{
		ANKI_ASSERT(m_immutable == false);
		TJob* newJob = 
			m_alloc.template newInstance<TJob>(std::forward<TArgs>(args)...);

		if(m_firstJob != nullptr)
		{
			ANKI_ASSERT(m_lastJob != nullptr);
			ANKI_ASSERT(m_lastJob->nextJob == nullptr);
			m_lastJob->nextJob = newJob;
			m_lastJob = newJob;
		}
		else
		{
			m_firstJob = newJob;
			m_lastJob = newJob;
		}
	}

	/// Execute all jobs
	void executeAllJobs();

	/// Make immutable
	void makeImmutable()
	{
		m_immutable = true;
	}

private:
	GlJobManager* m_server;
	GlJob* m_firstJob = nullptr;
	GlJob* m_lastJob = nullptr;
	GlJobChainAllocator<U8> m_alloc;
	Bool8 m_immutable = false;

#if ANKI_DEBUG
	Bool8 m_executed = false;
#endif

	void destroy();
};

/// @}

} // end namespace anki

#endif

