// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlQueue.h"
#include "anki/gl/GlCommandBuffer.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlDevice.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
GlQueue::GlQueue(GlDevice* manager, 
	AllocAlignedCallback allocCb, void* allocCbUserData)
	:	m_manager(manager), 
		m_allocCb(allocCb),
		m_allocCbUserData(allocCbUserData),
		m_tail(0), 
		m_head(0),
		m_renderingThreadSignal(0)
{
	ANKI_ASSERT(m_manager);
}

//==============================================================================
GlQueue::~GlQueue()
{}

//==============================================================================
void GlQueue::flushCommandChain(GlCommandBufferHandle& commands)
{
	commands._get().makeImmutable();

#if !ANKI_QUEUE_DISABLE_ASYNC
	{
		std::unique_lock<std::mutex> lock(m_mtx);

		if(m_error.size() > 0)
		{
			throw ANKI_EXCEPTION("GL rendering thread failed with error:\n%s",
				&m_error[0]);
		}

		// Set commandc
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

	m_condVar.notify_one(); // Wake the thread
#else
	commands._executeAllCommands();
#endif
}

//==============================================================================
void GlQueue::finishCommandChain(GlCommandBufferHandle& commands)
{
#if !ANKI_QUEUE_DISABLE_ASYNC
	flushCommandChain(commands);

	flushCommandChain(m_syncCommands);
	m_sync.wait();
#else
	flushCommandChain(commands);
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
	m_swapBuffersCommands = GlCommandBufferHandle(m_manager);
	m_swapBuffersCommands.pushBackUserCommand(swapBuffersInternal, this);

#if !ANKI_QUEUE_DISABLE_ASYNC
	// Start thread
	m_thread = std::thread(&GlQueue::threadLoop, this);

	// Create sync command buffer
	m_syncCommands = GlCommandBufferHandle(m_manager);
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
		std::unique_lock<std::mutex> lock(m_mtx);
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
	m_serverThreadId = std::this_thread::get_id();

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
void GlQueue::threadLoop()
{
	//setCurrentThreadName("anki_gl");

	prepare();

	while(1)
	{
		GlCommandBufferHandle commandc;

		// Wait for something
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			while(m_tail == m_head)
			{
				m_condVar.wait(lock);
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
			std::unique_lock<std::mutex> lock(m_mtx);
			m_error = e.what();
		}
	}

	finish();
}

//==============================================================================
void GlQueue::syncClientServer()
{
#if !ANKI_QUEUE_DISABLE_ASYNC
	flushCommandChain(m_syncCommands);
	m_sync.wait();
#endif
}

//==============================================================================
void GlQueue::swapBuffersInternal(void* ptr)
{
	ANKI_ASSERT(ptr);
	GlQueue& self = *reinterpret_cast<GlQueue*>(ptr);

	self.m_swapBuffersCallback(self.m_swapBuffersCbData);

	// Notify the main thread that we are done
	{
		std::unique_lock<std::mutex> lock(self.m_frameMtx);
		self.m_frameWait = false;
	}

	self.m_frameCondVar.notify_one();
}

//==============================================================================
void GlQueue::swapBuffers()
{
	// Wait for the rendering thread to finish swap buffers...
	{
		std::unique_lock<std::mutex> lock(m_frameMtx);
		while(m_frameWait)
		{
			m_frameCondVar.wait(lock);
		}

		m_frameWait = true;
	}

	// ...and then flush a new swap buffers
	flushCommandChain(m_swapBuffersCommands);
}

} // end namespace anki

