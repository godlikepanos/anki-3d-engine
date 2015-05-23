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

//==============================================================================
class SyncCommand final: public GlCommand
{
	ANKI_USE_RESULT Error operator()(CommandBufferImpl* cmd)
	{
		cmd->getManager().getImplementation().
			getRenderingThread().m_syncBarrier.wait();
		return ErrorCode::NONE;
	}
};

//==============================================================================
// RenderingThread                                                             =
//==============================================================================

//==============================================================================
RenderingThread::RenderingThread(GrManager* manager)
:	m_manager(manager),
	m_tail(0),
	m_head(0),
	m_renderingThreadSignal(0),
	m_thread("anki_gl"),
	m_state(manager)
{
	ANKI_ASSERT(m_manager);
}

//==============================================================================
RenderingThread::~RenderingThread()
{}

//==============================================================================
void RenderingThread::flushCommandBuffer(CommandBufferPtr& commands)
{
	commands.get().makeImmutable();

#if !ANKI_DISABLE_GL_RENDERING_THREAD
	{
		LockGuard<Mutex> lock(m_mtx);

		// Set commands
		U64 diff = m_tail - m_head;

		if(diff < m_queue.size())
		{
			U64 idx = m_tail % m_queue.size();

			m_queue[idx] = commands;
			++m_tail;
		}
		else
		{
			ANKI_LOGW("Rendering queue to small");
		}
	}

	m_condVar.notifyOne(); // Wake the thread
#else
	Error err = commands.get().executeAllCommands();
	if(err)
	{
		ANKI_LOGF("Error in command buffer execution");
	}
#endif
}

//==============================================================================
void RenderingThread::finishCommandBuffer(CommandBufferPtr& commands)
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	flushCommandBuffer(commands);

	syncClientServer();
#else
	flushCommandBuffer(commands);
#endif
}

//==============================================================================
Error RenderingThread::start(
	MakeCurrentCallback makeCurrentCb, void* makeCurrentCbData, void* ctx,
	SwapBuffersCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerMessages)
{
	ANKI_ASSERT(m_tail == 0 && m_head == 0);
	m_state.m_registerMessages = registerMessages;

	// Context
	ANKI_ASSERT(ctx != nullptr && makeCurrentCb != nullptr);
	m_ctx = ctx;
	m_makeCurrentCbData = makeCurrentCbData;
	m_makeCurrentCb = makeCurrentCb;

	// Swap buffers stuff
	ANKI_ASSERT(swapBuffersCallback != nullptr);
	m_swapBuffersCallback = swapBuffersCallback;
	m_swapBuffersCbData = swapBuffersCbData;
	m_swapBuffersCommands.create(m_manager);
	m_swapBuffersCommands.pushBackUserCommand(swapBuffersInternal, this);

#if !ANKI_DISABLE_GL_RENDERING_THREAD
	// Start thread
	m_thread.start(this, threadCallback);

	// Create sync command buffer
	m_syncCommands.create(m_manager);

	m_syncCommands.get().pushBackNewCommand<SyncCommand>();
#else
	prepare();

	ANKI_LOGW("GL queue works in synchronous mode");
#endif

	return ErrorCode::NONE;
}

//==============================================================================
void RenderingThread::stop()
{
#if !ANKI_DISABLE_GL_RENDERING_THREAD
	{
		LockGuard<Mutex> lock(m_mtx);
		m_renderingThreadSignal = 1;

		// Set some dummy values in order to unlock the cond var
		m_tail = m_queue.size() + 1;
		m_head = m_tail + 1;
	}
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

	// Create copy FBO
	glGenFramebuffers(1, &m_copyFbo);
}

//==============================================================================
void RenderingThread::finish()
{
	// Iterate the queue and release the refcounts
	for(U i = 0; i < m_queue.size(); i++)
	{
		if(m_queue[i].isCreated())
		{
			// Fake that it's executed to avoid warnings
			m_queue[i].get().makeExecuted();

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

			U64 idx = m_head % m_queue.size();
			// Pop a command
			cmd = m_queue[idx];
			m_queue[idx] = CommandBufferPtr(); // Insert empty cmd buffer

			++m_head;
		}

		Error err = cmd.get().executeAllCommands();

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
Error RenderingThread::swapBuffersInternal(void* ptr)
{
	ANKI_ASSERT(ptr);
	RenderingThread& self = *static_cast<RenderingThread*>(ptr);

	// Do the swap buffers
	self.m_swapBuffersCallback(self.m_swapBuffersCbData);

	// Notify the main thread that we are done
	{
		LockGuard<Mutex> lock(self.m_frameMtx);
		self.m_frameWait = false;
	}

	self.m_frameCondVar.notifyOne();

	return ErrorCode::NONE;
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

