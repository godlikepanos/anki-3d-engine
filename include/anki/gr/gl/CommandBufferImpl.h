// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_COMMAND_BUFFER_IMPL_H
#define ANKI_GR_GL_COMMAND_BUFFER_IMPL_H

#include "anki/gr/GrObject.h"
#include "anki/gr/GrManager.h"
#include "anki/util/Assert.h"
#include "anki/util/Allocator.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// The base of all GL commands
class GlCommand
{
public:
	GlCommand* m_nextCommand = nullptr;

	virtual ~GlCommand()
	{}

	/// Execute command
	virtual ANKI_USE_RESULT Error operator()(CommandBufferImpl*) = 0;
};

/// A number of GL commands organized in a chain
class CommandBufferImpl: public GrObject, public NonCopyable
{
public:
	using InitHints = CommandBufferImplInitHints;

	/// Default constructor
	CommandBufferImpl(GrManager* manager)
	:	GrObject(manager)
	{}

	~CommandBufferImpl()
	{
		destroy();
	}

	/// Default constructor
	/// @param server The command buffers server
	/// @param hints Hints to optimize the command's allocator
	ANKI_USE_RESULT Error create(const InitHints& hints);

	/// Get he allocator
	CommandBufferAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	/// Compute initialization hints
	InitHints computeInitHints() const;

	/// Create a new command and add it to the chain
	template<typename TCommand, typename... TArgs>
	void pushBackNewCommand(TArgs&&... args);

	/// Execute all commands
	ANKI_USE_RESULT Error executeAllCommands();

	/// Fake that it's been executed
	void makeExecuted()
	{
#if ANKI_DEBUG
		m_executed = true;
#endif
	}

	/// Make immutable
	void makeImmutable()
	{
		m_immutable = true;
	}

private:
	GlCommand* m_firstCommand = nullptr;
	GlCommand* m_lastCommand = nullptr;
	CommandBufferAllocator<U8> m_alloc;
	Bool8 m_immutable = false;

#if ANKI_DEBUG
	Bool8 m_executed = false;
#endif

	void destroy();
};

//==============================================================================
template<typename TCommand, typename... TArgs>
void CommandBufferImpl::pushBackNewCommand(TArgs&&... args)
{
	ANKI_ASSERT(m_immutable == false);
	TCommand* newCommand = m_alloc.template newInstance<TCommand>(
		std::forward<TArgs>(args)...);

	if(m_firstCommand != nullptr)
	{
		ANKI_ASSERT(m_lastCommand != nullptr);
		ANKI_ASSERT(m_lastCommand->m_nextCommand == nullptr);
		m_lastCommand->m_nextCommand = newCommand;
		m_lastCommand = newCommand;
	}
	else
	{
		ANKI_ASSERT(m_lastCommand == nullptr);
		m_firstCommand = newCommand;
		m_lastCommand = newCommand;
	}
}

/// A common command that deletes an object.
template<typename T>
class DeleteObjectCommand: public GlCommand
{
public:
	T* m_ptr;

	DeleteObjectCommand(T* ptr)
	:	m_ptr(ptr)
	{
		ANKI_ASSERT(m_ptr);
	}

	Error operator()(CommandBufferImpl* cmdb) override
	{
		cmdb->getManager().getAllocator().deleteInstance(m_ptr);
		return ErrorCode::NONE;
	}
};
/// @}

} // end namespace anki

#endif

