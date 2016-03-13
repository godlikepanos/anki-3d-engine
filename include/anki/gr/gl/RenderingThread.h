// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/GlState.h>
#include <anki/util/Thread.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Command queue. It's essentialy a queue of command buffers waiting for
/// execution and a server
class RenderingThread
{
	friend class SyncCommand;
	friend class SwapBuffersCommand;

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

	/// Start the working thread
	/// @note Don't free the context before calling #stop
	void start(Bool registerMessages, const ConfigSet& config);

	/// Stop the working thread
	void stop();

	/// Push a command buffer to the queue for deferred execution
	void flushCommandBuffer(CommandBufferPtr commands);

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
	WeakPtr<GrManager> m_manager;

	static const U QUEUE_SIZE = 1024 * 2;
	DArray<CommandBufferPtr> m_queue; ///< Command queue
	U64 m_tail; ///< Tail of queue
	U64 m_head; ///< Head of queue. Points to the end
	U8 m_renderingThreadSignal; ///< Signal to the thread
	Mutex m_mtx; ///< Wake the thread
	ConditionVariable m_condVar; ///< To wake up the thread
	Thread m_thread;

	CommandBufferPtr m_swapBuffersCommands;
	ConditionVariable m_frameCondVar;
	Mutex m_frameMtx;
	Bool8 m_frameWait = false;

	Thread::Id m_serverThreadId;

	GlState m_state;

	/// A special command buffer that is called every time we want to wait for
	/// the server
	CommandBufferPtr m_syncCommands;
	Barrier m_syncBarrier{2};
	SpinLock m_syncLock;

	/// Command buffer with an empty command.
	CommandBufferPtr m_emptyCmdb;

	/// The function that the thread runs
	static ANKI_USE_RESULT Error threadCallback(Thread::Info&);
	void threadLoop();
	void prepare();
	void finish();

	void swapBuffersInternal(GlState& state);
};
/// @}

} // end namespace anki
