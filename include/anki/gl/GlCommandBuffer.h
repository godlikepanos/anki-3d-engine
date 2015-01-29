// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_COMMAND_BUFFER_H
#define ANKI_GL_GL_COMMAND_BUFFER_H

#include "anki/gl/GlCommon.h"
#include "anki/util/Assert.h"
#include "anki/util/Allocator.h"

namespace anki {

// Forward 
class GlQueue;
class GlCommandBuffer;

/// @addtogroup opengl_private
/// @{

/// The base of all GL commands
class GlCommand
{
public:
	GlCommand* m_nextCommand = nullptr;

	virtual ~GlCommand()
	{}

	/// Execute command
	virtual ANKI_USE_RESULT Error operator()(GlCommandBuffer*) = 0;
};

/// A common command that deletes an object
template<typename T, typename TAlloc>
class GlDeleteObjectCommand: public GlCommand
{
public:
	T* m_ptr;
	TAlloc m_alloc;

	GlDeleteObjectCommand(T* ptr, TAlloc alloc)
	:	m_ptr(ptr), 
		m_alloc(alloc)
	{
		ANKI_ASSERT(m_ptr);
	}

	Error operator()(GlCommandBuffer*) override
	{
		m_alloc.deleteInstance(m_ptr);
		return ErrorCode::NONE;
	}
};

/// Command buffer initialization hints. They are used to optimize the 
/// allocators of a command buffer
class GlCommandBufferInitHints
{
	friend class GlCommandBuffer;

private:
	enum
	{
		MAX_CHUNK_SIZE = 4 * 1024 * 1024 // 4MB
	};

	PtrSize m_chunkSize = 1024;
};

/// A number of GL commands organized in a chain
class GlCommandBuffer: public NonCopyable
{
public:
	/// Default constructor
	GlCommandBuffer() = default;

	~GlCommandBuffer()
	{
		destroy();
	}

	/// Default constructor
	/// @param server The command buffers server
	/// @param hints Hints to optimize the command's allocator
	ANKI_USE_RESULT Error create(GlQueue* server, 
		const GlCommandBufferInitHints& hints);

	/// Get he allocator
	GlCommandBufferAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	GlAllocator<U8> getGlobalAllocator() const;

	GlQueue& getQueue()
	{
		return *m_server;
	}

	const GlQueue& getQueue() const
	{
		return *m_server;
	}

	/// Compute initialization hints
	GlCommandBufferInitHints computeInitHints() const;

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
	GlQueue* m_server = nullptr;
	GlCommand* m_firstCommand = nullptr;
	GlCommand* m_lastCommand = nullptr;
	GlCommandBufferAllocator<U8> m_alloc;
	Bool8 m_immutable = false;

#if ANKI_DEBUG
	Bool8 m_executed = false;
#endif

	void destroy();
};

//==============================================================================
template<typename TCommand, typename... TArgs>
void GlCommandBuffer::pushBackNewCommand(TArgs&&... args)
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
/// @}

} // end namespace anki

#endif

