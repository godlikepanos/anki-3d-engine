#include "anki/gl/GlProgramHandle.h"
#include "anki/gl/GlManager.h"
#include "anki/gl/GlClientBufferHandle.h"
#include "anki/gl/GlHandleDeferredDeleter.h"
#include "anki/gl/GlProgram.h"

namespace anki {

//==============================================================================
/// Create program job
class GlProgramCreateJob: public GlJob
{
public:
	GlProgramHandle m_prog;
	GLenum m_type;
	GlClientBufferHandle m_source;

	GlProgramCreateJob(GlProgramHandle prog, 
		GLenum type, GlClientBufferHandle source)
		: m_prog(prog), m_type(type), m_source(source)
	{}

	void operator()(GlJobChain* jobs)
	{
		GlProgram p(m_type, (const char*)m_source.getBaseAddress(), 
			jobs->getJobManager().getManager()._getAllocator());
		m_prog._get() = std::move(p);

		GlHandleState oldState = m_prog._setState(GlHandleState::CREATED);
		ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
		(void)oldState;
	}
};

//==============================================================================
GlProgramHandle::GlProgramHandle()
{}

//==============================================================================
GlProgramHandle::GlProgramHandle(GlJobChainHandle& jobs, 
	GLenum type, const GlClientBufferHandle& source)
{
	typedef GlGlobalHeapAllocator<GlProgram> Alloc;
	typedef GlDeleteObjectJob<GlProgram, Alloc> DeleteJob;
	typedef GlHandleDeferredDeleter<GlProgram, Alloc, DeleteJob> Deleter;

	*static_cast<Base::Base*>(this) = Base::Base(
		&jobs._get().getJobManager().getManager(),
		jobs._get().getGlobalAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	jobs._pushBackNewJob<GlProgramCreateJob>(*this, type, source);
}

//==============================================================================
GlProgramHandle::~GlProgramHandle()
{}

//==============================================================================
GLenum GlProgramHandle::getType() const
{
	serializeOnGetter();
	return _get().getType();
}

//==============================================================================
const GlProgramData::ProgramVector<GlProgramVariable>& 
	GlProgramHandle::getVariables() const
{
	serializeOnGetter();
	return _get().getVariables();
}

//==============================================================================
const GlProgramData::ProgramVector<GlProgramBlock>& 
	GlProgramHandle::getBlocks() const
{
	serializeOnGetter();
	return _get().getBlocks();
}

//==============================================================================
const GlProgramVariable& GlProgramHandle::findVariable(const char* name) const
{
	serializeOnGetter();
	return _get().findVariable(name);
}

//==============================================================================
const GlProgramBlock& GlProgramHandle::findBlock(const char* name) const
{
	serializeOnGetter();
	return _get().findBlock(name);
}

//==============================================================================
const GlProgramVariable* 
	GlProgramHandle::tryFindVariable(const char* name) const
{
	serializeOnGetter();
	return _get().tryFindVariable(name);
}

//==============================================================================
const GlProgramBlock* GlProgramHandle::tryFindBlock(const char* name) const
{
	serializeOnGetter();
	return _get().tryFindBlock(name);
}

} // end namespace anki

