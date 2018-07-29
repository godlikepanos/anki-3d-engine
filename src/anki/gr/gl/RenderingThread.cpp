// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/Fence.h>
#include <anki/gr/gl/FenceImpl.h>
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
		// Make sure that all GPU and CPU work is done
		glFlush();
		glFinish();
		m_renderingThread->m_syncBarrier.wait();
		return Error::NONE;
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
		// Blit from the fake FB to the real default FB
		const GrManagerImpl& gr = *static_cast<const GrManagerImpl*>(state.m_manager);
		const FramebufferImpl& fb = static_cast<FramebufferImpl&>(*gr.m_fakeDefaultFb);
		const U width = gr.m_fakeFbTex->getWidth();
		const U height = gr.m_fakeFbTex->getHeight();
		glBlitNamedFramebuffer(
			fb.getGlName(), 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// Swap buffers
		m_renderingThread->swapBuffersInternal();
		return Error::NONE;
	}
};

/// Empty command
class EmptyCommand final : public GlCommand
{
public:
	ANKI_USE_RESULT Error operator()(GlState&)
	{
		return Error::NONE;
	}
};

RenderingThread::RenderingThread(GrManagerImpl* manager)
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

void RenderingThread::flushCommandBuffer(CommandBufferPtr cmdb, FencePtr* fence)
{
	// Create a fence
	if(fence)
	{
		FencePtr& f = *fence;
		FenceImpl* fenceImpl = m_manager->getAllocator().newInstance<FenceImpl>(m_manager.get(), "N/A");
		f.reset(fenceImpl);

		class CreateFenceCmd final : public GlCommand
		{
		public:
			FencePtr m_fence;

			CreateFenceCmd(FencePtr fence)
				: m_fence(fence)
			{
			}

			Error operator()(GlState&)
			{
				static_cast<FenceImpl&>(*m_fence).m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				return Error::NONE;
			}
		};

		static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateFenceCmd>(f);
	}

	static_cast<CommandBufferImpl&>(*cmdb).makeImmutable();

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

void RenderingThread::start()
{
	ANKI_ASSERT(m_tail == 0 && m_head == 0);
	m_queue.create(m_manager->getAllocator(), QUEUE_SIZE);

	// Swap buffers stuff
	m_swapBuffersCommands = m_manager->newCommandBuffer(CommandBufferInitInfo());

	static_cast<CommandBufferImpl&>(*m_swapBuffersCommands).pushBackNewCommand<SwapBuffersCommand>(this);
	// Just in case noone swaps
	static_cast<CommandBufferImpl&>(*m_swapBuffersCommands).makeExecuted();

	m_manager->pinContextToCurrentThread(false);

	// Start thread
	m_thread.start(this, threadCallback);

	// Create sync command buffer
	m_syncCommands = m_manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*m_syncCommands).pushBackNewCommand<SyncCommand>(this);

	m_emptyCmdb = m_manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*m_emptyCmdb).pushBackNewCommand<EmptyCommand>();
}

void RenderingThread::stop()
{
	syncClientServer();
	m_renderingThreadSignal = 1;
	flushCommandBuffer(m_emptyCmdb, nullptr);

	Error err = m_thread.join();
	(void)err;
}

void RenderingThread::prepare()
{
	m_manager->pinContextToCurrentThread(true);

	// Ignore the first error
	glGetError();

	ANKI_GL_LOGI("OpenGL async thread started: OpenGL version \"%s\", GLSL version \"%s\"",
		reinterpret_cast<const char*>(glGetString(GL_VERSION)),
		reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

	// Get thread id
	m_serverThreadId = Thread::getCurrentThreadId();

	// Init state
	m_manager->getState().initRenderThread();
}

void RenderingThread::finish()
{
	// Iterate the queue and release the refcounts
	for(U i = 0; i < m_queue.getSize(); i++)
	{
		if(m_queue[i].isCreated())
		{
			// Fake that it's executed to avoid warnings
			static_cast<CommandBufferImpl&>(*m_queue[i]).makeExecuted();

			// Release
			m_queue[i] = CommandBufferPtr();
		}
	}

	// Cleanup GL
	m_manager->getState().destroy();

	// Cleanup
	glFinish();
	m_manager->pinContextToCurrentThread(false);
}

Error RenderingThread::threadCallback(ThreadCallbackInfo& info)
{
	RenderingThread* thread = static_cast<RenderingThread*>(info.m_userData);
	thread->threadLoop();
	return Error::NONE;
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

		Error err = Error::NONE;
		{
			ANKI_TRACE_SCOPED_EVENT(GL_THREAD);
			err = static_cast<CommandBufferImpl&>(*cmd).executeAllCommands();
		}

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

	flushCommandBuffer(m_syncCommands, nullptr);
	m_syncBarrier.wait();
}

void RenderingThread::swapBuffersInternal()
{
	ANKI_TRACE_SCOPED_EVENT(SWAP_BUFFERS);

	// Do the swap buffers
	m_manager->swapBuffers();

	// Notify the main thread that we are done
	{
		LockGuard<Mutex> lock(m_frameMtx);
		m_frameWait = false;

		m_frameCondVar.notifyOne();
	}
}

void RenderingThread::swapBuffers()
{
	ANKI_TRACE_SCOPED_EVENT(SWAP_BUFFERS);
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
	flushCommandBuffer(m_swapBuffersCommands, nullptr);
}

} // end namespace anki
