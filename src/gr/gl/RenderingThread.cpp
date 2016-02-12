// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/util/Logger.h>
#include <anki/core/Trace.h>
#include <cstdlib>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

#define ANKI_DISABLE_GL_RENDERING_THREAD 0

/// Sync rendering thread command.
class SyncCommand final : public GlCommand
{
public:
	RenderingThread* m_renderingThread;

	SyncCommand(RenderingThread* renderingThread)
		: m_renderingThread(renderingThread)
	{
	}

	ANKI_USE_RESULT Error operator()(GlState&)
	{
		m_renderingThread->m_syncBarrier.wait();
		return ErrorCode::NONE;
	}
};

/// Swap buffers command.
class SwapBuffersCommand final : public GlCommand
{
public:
	RenderingThread* m_renderingThread;

	SwapBuffersCommand(RenderingThread* renderingThread)
		: m_renderingThread(renderingThread)
	{
	}

	ANKI_USE_RESULT Error operator()(GlState& state)
	{
		m_renderingThread->swapBuffersInternal(state);
		return ErrorCode::NONE;
	}
};

/// Empty command
class EmptyCommand final : public GlCommand
{
public:
	ANKI_USE_RESULT Error operator()(GlState&)
	{
		return ErrorCode::NONE;
	}
};

//==============================================================================
// RenderingThread                                                             =
//==============================================================================

//==============================================================================
RenderingThread::RenderingThread(GrManager* manager)
	: m_manager(manager)
	, m_tail(0)
	, m_head(0)
	, m_renderingThreadSignal(0)
	, m_thread("anki_gl")
	, m_state(manager)
{
	ANKI_ASSERT(m_manager);
}

//==============================================================================
RenderingThread::~RenderingThread()
{
	m_queue.destroy(m_manager->getAllocator());
}

//==============================================================================
void RenderingThread::flushCommandBuffer(CommandBufferPtr commands)
{
	commands->getImplementation().makeImmutable();

#if !ANKI_DISABLE_GL_RENDERING_THREAD
	{
		LockGuard<Mutex> lock(m_mtx);

		// Set commands
		U64 diff = m_tail - m_head;

		if(diff < m_queue.getSize())
		{
			U64 idx = m_tail % m_queue.getSize();

			m_queue[idx] = commands;
			++m_tail;
		}
		else
		{
			ANKI_LOGW("Rendering queue too small");
		}
	}

	m_condVar.notifyOne(); // Wake the thread
#else
	Error err = commands->getImplementation().executeAllCommands();
	if(err)
	{
		ANKI_LOGF("Error in command buffer execution");
	}
#endif
}

//==============================================================================
void RenderingThread::finishCommandBuffer(CommandBufferPtr commands)
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	flushCommandBuffer(commands);

	syncClientServer();
#else
	flushCommandBuffer(commands);
#endif
}

//==============================================================================
void RenderingThread::start(WeakPtr<GrManagerInterface> interface,
	Bool registerMessages,
	const ConfigSet& config)
{
	ANKI_ASSERT(m_tail == 0 && m_head == 0);
	ANKI_ASSERT(interface);
	m_interface = interface;
	m_state.m_registerMessages = registerMessages;
	m_queue.create(m_manager->getAllocator(), QUEUE_SIZE);

	// Swap buffers stuff
	m_swapBuffersCommands =
		m_manager->newInstance<CommandBuffer>(CommandBufferInitInfo());
	m_swapBuffersCommands->getImplementation()
		.pushBackNewCommand<SwapBuffersCommand>(this);

	m_state.init0(config);

#if !ANKI_DISABLE_GL_RENDERING_THREAD
	// Start thread
	m_thread.start(this, threadCallback);

	// Create sync command buffer
	m_syncCommands =
		m_manager->newInstance<CommandBuffer>(CommandBufferInitInfo());
	m_syncCommands->getImplementation().pushBackNewCommand<SyncCommand>(this);

	m_emptyCmdb =
		m_manager->newInstance<CommandBuffer>(CommandBufferInitInfo());
	m_emptyCmdb->getImplementation().pushBackNewCommand<EmptyCommand>();
#else
	prepare();

	ANKI_LOGW("GL queue works in synchronous mode");
#endif
}

//==============================================================================
void RenderingThread::stop()
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	syncClientServer();
	m_renderingThreadSignal = 1;
	flushCommandBuffer(m_emptyCmdb);

	Error err = m_thread.join();
	(void)err;
#else
	finish();
#endif
}

//==============================================================================
void RenderingThread::prepare()
{
	m_interface->makeCurrentCommand(true);

	// Ignore the first error
	glGetError();

	ANKI_LOGI("OpenGL async thread started: OpenGL version %s, GLSL version %s",
		reinterpret_cast<const char*>(glGetString(GL_VERSION)),
		reinterpret_cast<const char*>(
				  glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// Get thread id
	m_serverThreadId = Thread::getCurrentThreadId();

	// Init state
	m_state.init1();
}

//==============================================================================
void RenderingThread::finish()
{
	// Iterate the queue and release the refcounts
	for(U i = 0; i < m_queue.getSize(); i++)
	{
		if(m_queue[i].isCreated())
		{
			// Fake that it's executed to avoid warnings
			m_queue[i]->getImplementation().makeExecuted();

			// Release
			m_queue[i] = CommandBufferPtr();
		}
	}

	// Cleanup GL
	m_state.destroy();

	// Cleanup
	glFinish();
	m_interface->makeCurrentCommand(false);
}

//==============================================================================
Error RenderingThread::threadCallback(Thread::Info& info)
{
	RenderingThread* thread = static_cast<RenderingThread*>(info.m_userData);
	thread->threadLoop();
	return ErrorCode::NONE;
}

//==============================================================================
void RenderingThread::threadLoop()
{
	prepare();

	while(1)
	{
		CommandBufferPtr cmd;

		// Wait for something
		{
			LockGuard<Mutex> lock(m_mtx);
			while(m_tail == m_head)
			{
				m_condVar.wait(m_mtx);
			}

			// Check signals
			if(m_renderingThreadSignal == 1)
			{
				// Requested to stop
				break;
			}

			U64 idx = m_head % m_queue.getSize();
			// Pop a command
			cmd = m_queue[idx];
			m_queue[idx] = CommandBufferPtr(); // Insert empty cmd buffer

			++m_head;
		}

		ANKI_TRACE_START_EVENT(GL_THREAD);
		Error err = cmd->getImplementation().executeAllCommands();
		ANKI_TRACE_STOP_EVENT(GL_THREAD);

		if(err)
		{
			ANKI_LOGE("Error in rendering thread. Aborting");
			abort();
		}
	}

	finish();
}

//==============================================================================
void RenderingThread::syncClientServer()
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	// Lock because there is only one barrier. If multiple threads call
	// syncClientServer all of them will hit the same barrier.
	LockGuard<SpinLock> lock(m_syncLock);

	flushCommandBuffer(m_syncCommands);
	m_syncBarrier.wait();
#endif
}

//==============================================================================
void RenderingThread::swapBuffersInternal(GlState& state)
{
	ANKI_TRACE_START_EVENT(SWAP_BUFFERS);

	// Do the swap buffers
	m_interface->swapBuffersCommand();

	// Notify the main thread that we are done
	{
		LockGuard<Mutex> lock(m_frameMtx);
		m_frameWait = false;
	}

	m_frameCondVar.notifyOne();
	state.checkDynamicMemoryConsumption();

	ANKI_TRACE_STOP_EVENT(SWAP_BUFFERS);
}

//==============================================================================
void RenderingThread::swapBuffers()
{
	ANKI_TRACE_START_EVENT(SWAP_BUFFERS);
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	// Wait for the rendering thread to finish swap buffers...
	{
		LockGuard<Mutex> lock(m_frameMtx);
		while(m_frameWait)
		{
			m_frameCondVar.wait(m_frameMtx);
		}

		m_frameWait = true;
	}
#endif
	// ...and then flush a new swap buffers
	flushCommandBuffer(m_swapBuffersCommands);
	ANKI_TRACE_STOP_EVENT(SWAP_BUFFERS);
}

} // end namespace anki
