// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_RENDERING_THREAD_H
#define ANKI_GR_GL_RENDERING_THREAD_H

#include "anki/gr/CommandBufferPtr.h"
#include "anki/gr/gl/GlState.h"
#include "anki/util/Thread.h"

namespace anki {

/// @addtogroup opengl
/// @{

#define ANKI_DISABLE_GL_RENDERING_THREAD 1

/// Command queue. It's essentialy a queue of command buffers waiting for
/// execution and a server
class RenderingThread
{
	friend class SyncCommand;

public:
	RenderingThread(GrManager* device);

	~RenderingThread();

	GlState& getState()
	{
		return m_state;
	}

	const GlState& getState() const
	{
		return m_state;
	}

	GLuint getCopyFbo() const
	{
		return m_copyFbo;
	}

	/// Start the working thread
	/// @note Don't free the context before calling #stop
	ANKI_USE_RESULT Error start(
		MakeCurrentCallback makeCurrentCb, void* makeCurrentCbData, void* ctx,
		SwapBuffersCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerMessages);

	/// Stop the working thread
	void stop();

	/// Push a command buffer to the queue for deferred execution
	void flushCommandBuffer(CommandBufferPtr& commands);

	/// Push a command buffer to the queue and wait for it
	void finishCommandBuffer(CommandBufferPtr& commands);

	/// Sync the client and server
	void syncClientServer();

	/// Return true if this is the main thread
	Bool isServerThread() const
	{
		return m_serverThreadId == Thread::getCurrentThreadId();
	}

	/// Swap buffers
	void swapBuffers();

private:
	GrManager* m_manager = nullptr;

	Array<CommandBufferPtr, 128> m_queue; ///< Command queue
	U64 m_tail; ///< Tail of queue
	U64 m_head; ///< Head of queue. Points to the end
	U8 m_renderingThreadSignal; ///< Signal to the thread
	Mutex m_mtx; ///< Wake the thread
	ConditionVariable m_condVar; ///< To wake up the thread
	Thread m_thread;

	void* m_makeCurrentCbData = nullptr; ///< Pointer first param of makecurrent
	void* m_ctx = nullptr; ///< Pointer to the system GL context
	MakeCurrentCallback m_makeCurrentCb; ///< Making a context current

	CommandBufferPtr m_swapBuffersCommands;
	SwapBuffersCallback m_swapBuffersCallback;
	void* m_swapBuffersCbData;
	ConditionVariable m_frameCondVar;
	Mutex m_frameMtx;
	Bool8 m_frameWait = false;

	Thread::Id m_serverThreadId;

	GlState m_state;
	GLuint m_defaultVao;

	/// A special command buffer that is called every time we want to wait for
	/// the server
	CommandBufferPtr m_syncCommands;
	Barrier m_syncBarrier{2};

	GLuint m_copyFbo = MAX_U32; ///< FBO for copying from tex to buffer.

	/// The function that the thread runs
	static ANKI_USE_RESULT Error threadCallback(Thread::Info&);
	void threadLoop();
	void prepare();
	void finish();

	static Error swapBuffersInternal(void* self);
};
/// @}

} // end namespace anki

#endif
