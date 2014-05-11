#include "anki/gl/GlJobManager.h"
#include "anki/gl/GlJobChain.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlManager.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
GlJobManager::GlJobManager(GlManager* manager)
	:	m_manager(manager), 
		m_tail(0), 
		m_head(0),
		m_renderingThreadSignal(0)
{
	ANKI_ASSERT(m_manager);
}

//==============================================================================
GlJobManager::~GlJobManager()
{}

//==============================================================================
void GlJobManager::flushJobChain(GlJobChainHandle& jobs)
{
	jobs._get().makeImmutable();

#if !ANKI_JOB_MANAGER_DISABLE_ASYNC
	{
		std::unique_lock<std::mutex> lock(m_mtx);

		// Set jobc
		U64 diff = m_tail - m_head;

		if(diff < m_queue.size())
		{
			U64 idx = m_tail % m_queue.size();

			m_queue[idx] = jobs;
			++m_tail;
		}
		else
		{
			ANKI_LOGW("Rendering queue to small");
		}
	}

	m_condVar.notify_one(); // Wake the thread
#else
	jobs._executeAllJobs();
#endif
}

//==============================================================================
void GlJobManager::finishJobChain(GlJobChainHandle& jobs)
{
#if !ANKI_JOB_MANAGER_DISABLE_ASYNC
	flushJobChain(jobs);

	flushJobChain(m_syncJobs);
	m_sync.wait();
#else
	flushJobChain(jobs);
#endif
}

//==============================================================================
void GlJobManager::start(
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
	m_swapBuffersJobs = GlJobChainHandle(m_manager);
	m_swapBuffersJobs.pushBackUserJob(swapBuffersInternal, this);

#if !ANKI_JOB_MANAGER_DISABLE_ASYNC
	// Start thread
	m_thread = std::thread(&GlJobManager::threadLoop, this);

	// Create sync job chain
	m_syncJobs = GlJobChainHandle(m_manager);
	m_sync = GlClientSyncHandle(m_syncJobs);
	m_sync.sync(m_syncJobs);
#else
	prepare();
#endif
}

//==============================================================================
void GlJobManager::stop()
{
#if !ANKI_JOB_MANAGER_DISABLE_ASYNC
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_renderingThreadSignal = 1;
	}
	m_thread.join();
#else
	finish();
#endif
}

//==============================================================================
void GlJobManager::prepare()
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
void GlJobManager::finish()
{
	// Iterate the queue and release the refcounts
	for(U i = 0; i < m_queue.size(); i++)
	{
		m_queue[i] = GlJobChainHandle();
	}

	// Delete default VAO
	glDeleteVertexArrays(1, &m_defaultVao);

	// Cleanup
	glFinish();
	(*m_makeCurrent)(nullptr);
}

//==============================================================================
void GlJobManager::threadLoop()
{
	setCurrentThreadName("anki_gl");

	prepare();

	while(1)
	{
		GlJobChainHandle jobc;

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
			// Pop a job
			jobc = m_queue[idx];
			m_queue[idx] = GlJobChainHandle(); // Insert empty jobchain

			++m_head;
		}

		// Exec jobs of chain
		jobc._executeAllJobs();
	}

	finish();
}

//==============================================================================
void GlJobManager::syncClientServer()
{
#if !ANKI_JOB_MANAGER_DISABLE_ASYNC
	flushJobChain(m_syncJobs);
	m_sync.wait();
#endif
}

//==============================================================================
void GlJobManager::swapBuffersInternal(void* ptr)
{
	ANKI_ASSERT(ptr);
	GlJobManager& self = *reinterpret_cast<GlJobManager*>(ptr);

	self.m_swapBuffersCallback(self.m_swapBuffersCbData);

	// Notify the main thread that we are done
	{
		std::unique_lock<std::mutex> lock(self.m_frameMtx);
		self.m_frameWait = false;
	}

	self.m_frameCondVar.notify_one();
}

//==============================================================================
void GlJobManager::swapBuffers()
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
	flushJobChain(m_swapBuffersJobs);
}

} // end namespace anki

