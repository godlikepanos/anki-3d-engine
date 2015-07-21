// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/util/Logger.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Sync rendering thread command.
class SyncCommand final: public GlCommand
{
public:
	RenderingThread* m_renderingThread;

	SyncCommand(RenderingThread* renderingThread)
		: m_renderingThread(renderingThread)
	{}

	ANKI_USE_RESULT Error operator()(GlState&)
	{
		m_renderingThread->m_syncBarrier.wait();
		return ErrorCode::NONE;
	}
};

/// Swap buffers command.
class SwapBuffersCommand final: public GlCommand
{
public:
	RenderingThread* m_renderingThread;

	SwapBuffersCommand(RenderingThread* renderingThread)
		: m_renderingThread(renderingThread)
	{}

	ANKI_USE_RESULT Error operator()(GlState&)
	{
		m_renderingThread->swapBuffersInternal();
		return ErrorCode::NONE;
	}
};

/// Empty command
class EmptyCommand final: public GlCommand
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
void RenderingThread::start(
	MakeCurrentCallback makeCurrentCb, void* makeCurrentCbData, void* ctx,
	SwapBuffersCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerMessages)
{
	ANKI_ASSERT(m_tail == 0 && m_head == 0);
	m_state.m_registerMessages = registerMessages;
	m_queue.create(m_manager->getAllocator(), QUEUE_SIZE);

	// Context
	ANKI_ASSERT(ctx != nullptr && makeCurrentCb != nullptr);
	m_ctx = ctx;
	m_makeCurrentCbData = makeCurrentCbData;
	m_makeCurrentCb = makeCurrentCb;

	// Swap buffers stuff
	ANKI_ASSERT(swapBuffersCallback != nullptr);
	m_swapBuffersCallback = swapBuffersCallback;
	m_swapBuffersCbData = swapBuffersCbData;
	m_swapBuffersCommands = m_manager->newInstance<CommandBuffer>();
	m_swapBuffersCommands->getImplementation().
		pushBackNewCommand<SwapBuffersCommand>(this);

#if !ANKI_DISABLE_GL_RENDERING_THREAD
	// Start thread
	m_thread.start(this, threadCallback);

	// Create sync command buffer
	m_syncCommands = m_manager->newInstance<CommandBuffer>();
	m_syncCommands->getImplementation().pushBackNewCommand<SyncCommand>(this);

	m_emptyCmdb = m_manager->newInstance<CommandBuffer>();
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
	ANKI_ASSERT(m_makeCurrentCb && m_ctx);
	(*m_makeCurrentCb)(m_makeCurrentCbData, m_ctx);

	// Ignore the first error
	glGetError();

	ANKI_LOGI("OpenGL async thread started: OpenGL version %s, GLSL version %s",
		reinterpret_cast<const char*>(glGetString(GL_VERSION)),
		reinterpret_cast<const char*>(
		glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// Get thread id
	m_serverThreadId = Thread::getCurrentThreadId();

	// Init state
	m_state.init();

	// Create default VAO
	glGenVertexArrays(1, &m_defaultVao);
	glBindVertexArray(m_defaultVao);
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
	glDeleteVertexArrays(1, &m_defaultVao);
	m_state.destroy();

	// Cleanup
	glFinish();
	(*m_makeCurrentCb)(m_makeCurrentCbData, nullptr);
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

		Error err = cmd->getImplementation().executeAllCommands();

		if(err)
		{
			ANKI_LOGE("Error in rendering thread. Aborting\n");
			abort();
		}
	}

	finish();
}

//==============================================================================
void RenderingThread::syncClientServer()
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	flushCommandBuffer(m_syncCommands);
	m_syncBarrier.wait();
#endif
}

//==============================================================================
void RenderingThread::swapBuffersInternal()
{
	// Do the swap buffers
	m_swapBuffersCallback(m_swapBuffersCbData);

	// Notify the main thread that we are done
	{
		LockGuard<Mutex> lock(m_frameMtx);
		m_frameWait = false;
	}

	m_frameCondVar.notifyOne();
}

//==============================================================================
void RenderingThread::swapBuffers()
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	// Wait for the rendering thread to finish swap buffers...
	{
		LockGuard<Mutex> lock(m_frameMtx);
		while(m_frameWait)
		{
			ANKI_COUNTER_START_TIMER(GL_SERVER_WAIT_TIME);
			m_frameCondVar.wait(m_frameMtx);
			ANKI_COUNTER_STOP_TIMER_INC(GL_SERVER_WAIT_TIME);
		}

		m_frameWait = true;
	}
#endif
	// ...and then flush a new swap buffers
	flushCommandBuffer(m_swapBuffersCommands);
}

} // end namespace anki

