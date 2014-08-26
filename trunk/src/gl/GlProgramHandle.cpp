// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlProgramHandle.h"
#include "anki/gl/GlDevice.h"
#include "anki/gl/GlClientBufferHandle.h"
#include "anki/gl/GlHandleDeferredDeleter.h"
#include "anki/gl/GlProgram.h"

namespace anki {

//==============================================================================
/// Create program command
class GlProgramCreateCommand: public GlCommand
{
public:
	GlProgramHandle m_prog;
	GLenum m_type;
	GlClientBufferHandle m_source;

	GlProgramCreateCommand(GlProgramHandle prog, 
		GLenum type, GlClientBufferHandle source)
	:	m_prog(prog), 
		m_type(type), 
		m_source(source)
	{}

	void operator()(GlCommandBuffer* commands)
	{
		GlProgram p(m_type, (const char*)m_source.getBaseAddress(), 
			commands->getQueue().getDevice()._getAllocator(),
			commands->getQueue().getDevice()._getCacheDirectory());
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
GlProgramHandle::GlProgramHandle(GlCommandBufferHandle& commands, 
	GLenum type, const GlClientBufferHandle& source)
{
	using Alloc = GlGlobalHeapAllocator<GlProgram>;
	using DeleteCommand = GlDeleteObjectCommand<GlProgram, Alloc>;
	using Deleter = GlHandleDeferredDeleter<GlProgram, Alloc, DeleteCommand>;

	*static_cast<Base::Base*>(this) = Base::Base(
		&commands._get().getQueue().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	commands._pushBackNewCommand<GlProgramCreateCommand>(*this, type, source);
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

