// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/CommandBuffer.h>
#include <anki/util/Thread.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Command queue. It's essentialy a queue of command buffers waiting for execution and a server
class RenderingThread
{
	friend class SyncCommand;
	friend class SwapBuffersCommand;

public:
	RenderingThread(GrManagerImpl* device);

	~RenderingThread();

	/// Start the working thread
	/// @note Don't free the context before calling #stop
	void start();

	/// Stop the working thread
	void stop();

	/// Push a command buffer to the queue for deferred execution
	void flushCommandBuffer(CommandBufferPtr commands, FencePtr* fence);

	/// Push a command buffer to the queue and wait for it
	void finishCommandBuffer(CommandBufferPtr commands);

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
	WeakPtr<GrManagerImpl> m_manager;

	static const U QUEUE_SIZE = 1024 * 2;
	DynamicArray<CommandBufferPtr> m_queue; ///< Command queue
	U64 m_tail; ///< Tail of queue
	U64 m_head; ///< Head of queue. Points to the end
	U8 m_renderingThreadSignal; ///< Signal to the thread
	Mutex m_mtx; ///< Wake the thread
	ConditionVariable m_condVar; ///< To wake up the thread
	Thread m_thread;

	/// @name Swap_buffers_vars
	/// @{
	CommandBufferPtr m_swapBuffersCommands;
	ConditionVariable m_frameCondVar;
	Mutex m_frameMtx;
	Bool m_frameWait = false;
	/// @}

	ThreadId m_serverThreadId;

	/// A special command buffer that is called every time we want to wait for the server
	CommandBufferPtr m_syncCommands;
	Barrier m_syncBarrier{2};
	SpinLock m_syncLock;

	/// Command buffer with an empty command.
	CommandBufferPtr m_emptyCmdb;

	/// The function that the thread runs
	static ANKI_USE_RESULT Error threadCallback(ThreadCallbackInfo&);
	void threadLoop();
	void prepare();
	void finish();

	void swapBuffersInternal();
};
/// @}

} // end namespace anki
