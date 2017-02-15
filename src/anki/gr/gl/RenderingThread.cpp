// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/util/Logger.h>
#include <anki/core/Trace.h>
#include <cstdlib>

namespace anki
{

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

	ANKI_USE_RESULT Error operator()(GlState&)
	{
		m_renderingThread->swapBuffersInternal();
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

RenderingThread::RenderingThread(GrManager* manager)
	: m_manager(manager)
	, m_tail(0)
	, m_head(0)
	, m_renderingThreadSignal(0)
	, m_thread("anki_gl")
{
	ANKI_ASSERT(m_manager);
}

RenderingThread::~RenderingThread()
{
	m_queue.destroy(m_manager->getAllocator());
}

void RenderingThread::flushCommandBuffer(CommandBufferPtr cmdb)
{
	cmdb->m_impl->makeImmutable();

	{
		LockGuard<Mutex> lock(m_mtx);

		// Set commands
		U64 diff = m_tail - m_head;

		if(diff < m_queue.getSize())
		{
			U64 idx = m_tail % m_queue.getSize();

			m_queue[idx] = cmdb;
			++m_tail;
		}
		else
		{
			ANKI_GL_LOGW("Rendering queue too small");
		}

		m_condVar.notifyOne(); // Wake the thread
	}
}

void RenderingThread::finishCommandBuffer(CommandBufferPtr commands)
{
	flushCommandBuffer(commands);

	syncClientServer();
}

void RenderingThread::start()
{
	ANKI_ASSERT(m_tail == 0 && m_head == 0);
	m_queue.create(m_manager->getAllocator(), QUEUE_SIZE);

	// Swap buffers stuff
	m_swapBuffersCommands = m_manager->newInstance<CommandBuffer>(CommandBufferInitInfo());
	m_swapBuffersCommands->m_impl->pushBackNewCommand<SwapBuffersCommand>(this);
	// Just in case noone swaps
	m_swapBuffersCommands->m_impl->makeExecuted();

	m_manager->getImplementation().pinContextToCurrentThread(false);

	// Start thread
	m_thread.start(this, threadCallback);

	// Create sync command buffer
	m_syncCommands = m_manager->newInstance<CommandBuffer>(CommandBufferInitInfo());
	m_syncCommands->m_impl->pushBackNewCommand<SyncCommand>(this);

	m_emptyCmdb = m_manager->newInstance<CommandBuffer>(CommandBufferInitInfo());
	m_emptyCmdb->m_impl->pushBackNewCommand<EmptyCommand>();
}

void RenderingThread::stop()
{
	syncClientServer();
	m_renderingThreadSignal = 1;
	flushCommandBuffer(m_emptyCmdb);

	Error err = m_thread.join();
	(void)err;
}

void RenderingThread::prepare()
{
	m_manager->getImplementation().pinContextToCurrentThread(true);

	// Ignore the first error
	glGetError();

	ANKI_GL_LOGI("OpenGL async thread started: OpenGL version \"%s\", GLSL version \"%s\"",
		reinterpret_cast<const char*>(glGetString(GL_VERSION)),
		reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// Get thread id
	m_serverThreadId = Thread::getCurrentThreadId();

	// Init state
	m_manager->getImplementation().getState().initRenderThread();
}

void RenderingThread::finish()
{
	// Iterate the queue and release the refcounts
	for(U i = 0; i < m_queue.getSize(); i++)
	{
		if(m_queue[i].isCreated())
		{
			// Fake that it's executed to avoid warnings
			m_queue[i]->m_impl->makeExecuted();

			// Release
			m_queue[i] = CommandBufferPtr();
		}
	}

	// Cleanup GL
	m_manager->getImplementation().getState().destroy();

	// Cleanup
	glFinish();
	m_manager->getImplementation().pinContextToCurrentThread(false);
}

Error RenderingThread::threadCallback(ThreadCallbackInfo& info)
{
	RenderingThread* thread = static_cast<RenderingThread*>(info.m_userData);
	thread->threadLoop();
	return ErrorCode::NONE;
}

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
		Error err = cmd->m_impl->executeAllCommands();
		ANKI_TRACE_STOP_EVENT(GL_THREAD);

		if(err)
		{
			ANKI_GL_LOGE("Error in rendering thread. Aborting");
			abort();
		}
	}

	finish();
}

void RenderingThread::syncClientServer()
{
	// Lock because there is only one barrier. If multiple threads call
	// syncClientServer all of them will hit the same barrier.
	LockGuard<SpinLock> lock(m_syncLock);

	flushCommandBuffer(m_syncCommands);
	m_syncBarrier.wait();
}

void RenderingThread::swapBuffersInternal()
{
	ANKI_TRACE_START_EVENT(SWAP_BUFFERS);

	// Do the swap buffers
	m_manager->getImplementation().swapBuffers();

	// Notify the main thread that we are done
	{
		LockGuard<Mutex> lock(m_frameMtx);
		m_frameWait = false;

		m_frameCondVar.notifyOne();
	}

	ANKI_TRACE_STOP_EVENT(SWAP_BUFFERS);
}

void RenderingThread::swapBuffers()
{
	ANKI_TRACE_START_EVENT(SWAP_BUFFERS);
	// Wait for the rendering thread to finish swap buffers...
	{
		LockGuard<Mutex> lock(m_frameMtx);
		while(m_frameWait)
		{
			m_frameCondVar.wait(m_frameMtx);
		}

		m_frameWait = true;
	}

	// ...and then flush a new swap buffers
	flushCommandBuffer(m_swapBuffersCommands);
	ANKI_TRACE_STOP_EVENT(SWAP_BUFFERS);
}

} // end namespace anki
