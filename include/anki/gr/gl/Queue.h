// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_QUEUE_H
#define ANKI_GR_GL_QUEUE_H

#include "anki/gr/CommandBufferHandle.h"
#include "anki/gr/GlSyncHandles.h"
#include "anki/gr/gl/State.h"
#include "anki/util/Thread.h"

namespace anki {

// Forward
class GlDevice;

/// @addtogroup opengl_private
/// @{

/// Command queue. It's essentialy a queue of command buffers waiting for 
/// execution and a server
class Queue
{
public:
	Queue(GlDevice* device, 
		AllocAlignedCallback alloc, void* allocUserData);

	~Queue();

	GlDevice& getDevice()
	{
		ANKI_ASSERT(m_device);
		return *m_device;
	}

	const GlDevice& getDevice() const
	{
		ANKI_ASSERT(m_device);
		return *m_device;
	}

	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	void* getAllocationCallbackUserData() const
	{
		return m_allocCbUserData;
	}

	State& getState()
	{
		return m_state;
	}

	const State& getState() const
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
		GlMakeCurrentCallback makeCurrentCb, void* makeCurrentCbData, void* ctx,
		GlCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerMessages);

	/// Stop the working thread
	void stop();

	/// Push a command buffer to the queue for deferred execution
	void flushCommandBuffer(CommandBufferHandle& commands);

	/// Push a command buffer to the queue and wait for it
	void finishCommandBuffer(CommandBufferHandle& commands);

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
	GlDevice* m_device = nullptr;
	AllocAlignedCallback m_allocCb;
	void* m_allocCbUserData;

	Array<CommandBufferHandle, 128> m_queue; ///< Command queue
	U64 m_tail; ///< Tail of queue
	U64 m_head; ///< Head of queue. Points to the end
	U8 m_renderingThreadSignal; ///< Signal to the thread
	Mutex m_mtx; ///< Wake the thread
	ConditionVariable m_condVar; ///< To wake up the thread
	Thread m_thread;

	void* m_makeCurrentCbData = nullptr; ///< Pointer first param of makecurrent
	void* m_ctx = nullptr; ///< Pointer to the system GL context
	GlMakeCurrentCallback m_makeCurrentCb; ///< Making a context current

	CommandBufferHandle m_swapBuffersCommands;
	GlCallback m_swapBuffersCallback;
	void* m_swapBuffersCbData;
	ConditionVariable m_frameCondVar;
	Mutex m_frameMtx;
	Bool8 m_frameWait = false;

	Thread::Id m_serverThreadId;
	
	State m_state;
	GLuint m_defaultVao;

	/// A special command buffer that is called every time we want to wait for 
	/// the server
	CommandBufferHandle m_syncCommands;
	GlClientSyncHandle m_sync;

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
