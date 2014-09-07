// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlQueue.h"
#include "anki/gl/GlCommandBuffer.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlDevice.h"
#include "anki/core/Logger.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
GlQueue::GlQueue(GlDevice* device, 
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
GlQueue::~GlQueue()
{}

//==============================================================================
void GlQueue::flushCommandBuffer(GlCommandBufferHandle& commands)
{
	commands._get().makeImmutable();

#if !ANKI_QUEUE_DISABLE_ASYNC
	{
		LockGuard<Mutex> lock(m_mtx);

		if(!m_error.isEmpty())
		{
			throw ANKI_EXCEPTION("GL rendering thread failed with error:\n%s",
				&m_error[0]);
		}

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
	commands._executeAllCommands();
#endif
}

//==============================================================================
void GlQueue::finishCommandBuffer(GlCommandBufferHandle& commands)
{
#if !ANKI_QUEUE_DISABLE_ASYNC
	flushCommandBuffer(commands);

	flushCommandBuffer(m_syncCommands);
	m_sync.wait();
#else
	flushCommandBuffer(commands);
#endif
}

//==============================================================================
void GlQueue::start(
	GlCallback makeCurrentCallback, void* context, 
	GlCallback swapBuffersCallback, void* swapBuffersCbData,
	Bool registerMessages)
{
	ANKI_ASSERT(m_tail == 0 && m_head == 0);
	m_state.m_registerMessages = registerMessages;

	// Context
	ANKI_ASSERT(context != nullptr && makeCurrentCallback != nullptr);
	m_ctx = context;
	m_makeCurrent = makeCurrentCallback;

	// Swap buffers stuff
	ANKI_ASSERT(swapBuffersCallback != nullptr);
	m_swapBuffersCallback = swapBuffersCallback;
	m_swapBuffersCbData = swapBuffersCbData;
	m_swapBuffersCommands = GlCommandBufferHandle(m_device);
	m_swapBuffersCommands.pushBackUserCommand(swapBuffersInternal, this);

#if !ANKI_QUEUE_DISABLE_ASYNC
	// Start thread
	m_thread.start(this, threadCallback);

	// Create sync command buffer
	m_syncCommands = GlCommandBufferHandle(m_device);
	m_sync = GlClientSyncHandle(m_syncCommands);
	m_sync.sync(m_syncCommands);
#else
	prepare();
#endif
}

//==============================================================================
void GlQueue::stop()
{
#if !ANKI_QUEUE_DISABLE_ASYNC
	{
		LockGuard<Mutex> lock(m_mtx);
		m_renderingThreadSignal = 1;

		// Set some dummy values in order to unlock the cond var
		m_tail = m_queue.size() + 1;
		m_head = m_tail + 1;
	}
	m_thread.join();
#else
	finish();
#endif
}

//==============================================================================
void GlQueue::prepare()
{
	ANKI_ASSERT(m_makeCurrent && m_ctx);
	(*m_makeCurrent)(m_ctx);

	// Ignore the first error
	glGetError();

	ANKI_LOGI("OpenGL info: OGL %s, GLSL %s",
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
void GlQueue::finish()
{
	// Iterate the queue and release the refcounts
	for(U i = 0; i < m_queue.size(); i++)
	{
		if(m_queue[i].isCreated())
		{
			// Fake that it's executed to avoid warnings
			m_queue[i]._get().makeExecuted();

			// Release
			m_queue[i] = GlCommandBufferHandle();
		}
	}

	// Delete default VAO
	glDeleteVertexArrays(1, &m_defaultVao);

	// Cleanup
	glFinish();
	(*m_makeCurrent)(nullptr);
}

//==============================================================================
I GlQueue::threadCallback(Thread::Info& info)
{
	GlQueue* queue = reinterpret_cast<GlQueue*>(info.m_userData);
	queue->threadLoop();
	return 0;
}

//==============================================================================
void GlQueue::threadLoop()
{
	prepare();

	while(1)
	{
		GlCommandBufferHandle commandc;

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
			commandc = m_queue[idx];
			m_queue[idx] = GlCommandBufferHandle(); // Insert empty cmd buffer

			++m_head;
		}

		try
		{
			// Exec commands of chain
			commandc._executeAllCommands();
		}
		catch(const std::exception& e)
		{
			LockGuard<Mutex> lock(m_mtx);
			m_error = e.what();
		}
	}

	finish();
}

//==============================================================================
void GlQueue::syncClientServer()
{
#if !ANKI_QUEUE_DISABLE_ASYNC
	flushCommandBuffer(m_syncCommands);
	m_sync.wait();
#endif
}

//==============================================================================
void GlQueue::swapBuffersInternal(void* ptr)
{
	ANKI_ASSERT(ptr);
	GlQueue& self = *reinterpret_cast<GlQueue*>(ptr);

	// Do the swap buffers
	self.m_swapBuffersCallback(self.m_swapBuffersCbData);

	// Notify the main thread that we are done
	{
		LockGuard<Mutex> lock(self.m_frameMtx);
		self.m_frameWait = false;
	}

	self.m_frameCondVar.notifyOne();
}

//==============================================================================
void GlQueue::swapBuffers()
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

