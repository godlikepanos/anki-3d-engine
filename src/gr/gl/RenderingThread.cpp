// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/RenderingThread.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/GlDevice.h"
#include "anki/util/Logger.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
class SyncCommand: public GlCommand
{
	ANKI_USE_RESULT Error operator()(CommandBufferImpl* cmd)
	{
		cmd->getRenderingThread().m_syncBarrier.wait();
		return ErrorCode::NONE;
	}
};

//==============================================================================
// RenderingThread                                                             =
//==============================================================================

//==============================================================================
RenderingThread::RenderingThread(GlDevice* device, 
	AllocAlignedCallback allocCb, void* allocCbUserData)
:	m_device(device), 
	m_allocCb(allocCb),
	m_allocCbUserData(allocCbUserData),
	m_tail(0), 
	m_head(0),
	m_renderingThreadSignal(0),
	m_thread("anki_gl")
{
	ANKI_ASSERT(m_device);
}

//==============================================================================
RenderingThread::~RenderingThread()
{}

//==============================================================================
void RenderingThread::flushCommandBuffer(CommandBufferHandle& commands)
{
	commands._get().makeImmutable();

#if !ANKI_QUEUE_DISABLE_ASYNC
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
	Error err = commands._executeAllCommands();
	if(err)
	{
		ANKI_LOGE("Error in command buffer");
	}
#endif
}

//==============================================================================
void RenderingThread::finishCommandBuffer(CommandBufferHandle& commands)
{
#if !ANKI_QUEUE_DISABLE_ASYNC
	flushCommandBuffer(commands);

	flushCommandBuffer(m_syncCommands);
	m_syncBarrier.wait();
#else
	flushCommandBuffer(commands);
#endif
}

//==============================================================================
Error RenderingThread::start(
	GlMakeCurrentCallback makeCurrentCb, void* makeCurrentCbData, void* ctx,
	GlCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerMessages)
{
	Error err = ErrorCode::NONE;

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
	err = m_swapBuffersCommands.create(m_device);
	if(!err)
	{
		m_swapBuffersCommands.pushBackUserCommand(swapBuffersInternal, this);
	}

#if !ANKI_QUEUE_DISABLE_ASYNC
	Bool threadStarted = false;
	if(!err)
	{
		// Start thread
		m_thread.start(this, threadCallback);
		threadStarted = true;

		// Create sync command buffer
		err = m_syncCommands.create(m_device);
	}

	if(!err)
	{
		m_syncCommands._pushBackNewCommand<SyncCommand>();
	}

	if(err && threadStarted)
	{
		err = m_thread.join();
	}
#else
	prepare();
#endif

	return err;
}

//==============================================================================
void RenderingThread::stop()
{
#if !ANKI_QUEUE_DISABLE_ASYNC
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
			m_queue[i]._get().makeExecuted();

			// Release
			m_queue[i] = CommandBufferHandle();
		}
	}

	// Delete default VAO
	glDeleteVertexArrays(1, &m_defaultVao);

	// Cleanup
	glFinish();
	(*m_makeCurrentCb)(m_makeCurrentCbData, nullptr);
}

//==============================================================================
Error RenderingThread::threadCallback(Thread::Info& info)
{
	RenderingThread* queue = reinterpret_cast<RenderingThread*>(info.m_userData);
	queue->threadLoop();
	return ErrorCode::NONE;
}

//==============================================================================
void RenderingThread::threadLoop()
{
	prepare();

	while(1)
	{
		CommandBufferHandle cmd;

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
			m_queue[idx] = CommandBufferHandle(); // Insert empty cmd buffer

			++m_head;
		}

		Error err = cmd._executeAllCommands();

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
#if !ANKI_QUEUE_DISABLE_ASYNC
	flushCommandBuffer(m_syncCommands);
	m_syncBarrier.wait();
#endif
}

//==============================================================================
Error RenderingThread::swapBuffersInternal(void* ptr)
{
	ANKI_ASSERT(ptr);
	RenderingThread& self = *reinterpret_cast<RenderingThread*>(ptr);

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

	// ...and then flush a new swap buffers
	flushCommandBuffer(m_swapBuffersCommands);
}

} // end namespace anki

