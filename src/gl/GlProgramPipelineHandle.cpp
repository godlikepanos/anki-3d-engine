#include "anki/gl/GlProgramPipelineHandle.h"
#include "anki/gl/GlProgramPipeline.h"
#include "anki/gl/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
GlProgramPipelineHandle::GlProgramPipelineHandle()
{}

//==============================================================================
GlProgramPipelineHandle::GlProgramPipelineHandle(
	GlJobChainHandle& jobs,
	std::initializer_list<GlProgramHandle> iprogs)
{
	Array<GlProgramHandle, 6> progs;

	U count = 0;
	for(GlProgramHandle prog : iprogs)
	{
		progs[count++] = prog;
	}

	commonConstructor(jobs, &progs[0], &progs[0] + count);
}

//==============================================================================
GlProgramPipelineHandle::~GlProgramPipelineHandle()
{}

//==============================================================================
void GlProgramPipelineHandle::commonConstructor(
	GlJobChainHandle& jobs,
	const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
{
	class Job: public GlJob
	{
	public:
		GlProgramPipelineHandle m_ppline;
		Array<GlProgramHandle, 6> m_progs;
		U8 m_progsCount;

		Job(GlProgramPipelineHandle& ppline, 
			const GlProgramHandle* progsBegin, const GlProgramHandle* progsEnd)
			: m_ppline(ppline)
		{
			m_progsCount = 0;
			const GlProgramHandle* prog = progsBegin;
			do
			{
				m_progs[m_progsCount++] = *prog;
			} while(++prog != progsEnd);
		}

		void operator()(GlJobChain*)
		{
			GlProgramPipeline ppline(&m_progs[0], &m_progs[0] + m_progsCount);
			m_ppline._get() = std::move(ppline);
			GlHandleState oldState = m_ppline._setState(GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;
		}
	};

	typedef GlGlobalHeapAllocator<GlProgramPipeline> Alloc;
	typedef GlDeleteObjectJob<GlProgramPipeline, Alloc> DeleteJob;
	typedef GlHandleDeferredDeleter<GlProgramPipeline, Alloc, DeleteJob> 
		Deleter;

	*static_cast<Base::Base*>(this) = Base::Base(
		&jobs._get().getJobManager().getManager(),
		jobs._get().getGlobalAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	jobs._pushBackNewJob<Job>(*this, progsBegin, progsEnd);
}

//==============================================================================
void GlProgramPipelineHandle::bind(GlJobChainHandle& jobs)
{
	ANKI_ASSERT(isCreated());

	class Job: public GlJob
	{
	public:
		GlProgramPipelineHandle m_ppline;

		Job(GlProgramPipelineHandle& ppline)
			: m_ppline(ppline)
		{}

		void operator()(GlJobChain* jobs)
		{
			GlState& state = jobs->getJobManager().getState();

			if(state.m_crntPpline != m_ppline._get().getGlName())
			{
				m_ppline._get().bind();

				state.m_crntPpline = m_ppline._get().getGlName();
			}
		}
	};

	jobs._pushBackNewJob<Job>(*this);
}

//==============================================================================
GlProgramHandle GlProgramPipelineHandle::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	serializeOnGetter();
	return _get().getAttachedProgram(type);
}

} // end namespace anki

