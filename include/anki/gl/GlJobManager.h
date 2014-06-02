#ifndef ANKI_GL_GL_JOB_MANAGER_H
#define ANKI_GL_GL_JOB_MANAGER_H

#include "anki/gl/GlJobChainHandle.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlState.h"
#include "anki/util/Thread.h"

namespace anki {

// Forward
class GlManager;

/// @addtogroup opengl_private
/// @{

/// Job manager. It's essentialy a queue of job chains waiting for execution 
/// and a server
class GlJobManager
{
public:
	/// @name Contructors/Destructor
	/// @{
	GlJobManager(GlManager* manager);

	~GlJobManager();
	/// @}

	/// @name Accessors
	/// @{
	GlManager& getManager()
	{
		ANKI_ASSERT(m_manager);
		return *m_manager;
	}

	const GlManager& getManager() const
	{
		ANKI_ASSERT(m_manager);
		return *m_manager;
	}

	GlState& getState()
	{
		return m_state;
	}

	const GlState& getState() const
	{
		return m_state;
	}
	/// @}

	/// Start the working thread
	/// @note Don't free the context before calling #stop
	void start(
		GlCallback makeCurrentCallback, void* context, 
		GlCallback swapBuffersCallback, void* swapBuffersCbData,
		Bool registerMessages);

	/// Stop the working thread
	void stop();

	/// Push a job chain to the queue for deferred execution
	void flushJobChain(GlJobChainHandle& jobs);

	/// Push a job chain to the queue and wait for it
	void finishJobChain(GlJobChainHandle& jobs);

	/// Sync the client and server
	void syncClientServer();

	/// Return true if this is the main thread
	Bool isServerThread() const
	{
		return m_serverThreadId == std::this_thread::get_id();
	}

	/// Swap buffers
	void swapBuffers();

private:
	GlManager* m_manager = nullptr;

	Array<GlJobChainHandle, 32> m_queue; ///< Job queue
	U64 m_tail; ///< Tail of queue
	U64 m_head; ///< Head of queue. Points to the end
	U8 m_renderingThreadSignal; ///< Signal to the thread
	std::mutex m_mtx; ///< Wake the thread
	std::condition_variable m_condVar; ///< To wake up the thread
	std::thread m_thread;

	void* m_ctx = nullptr; ///< Pointer to the system GL context
	GlCallback m_makeCurrent; ///< Making a context current

	GlJobChainHandle m_swapBuffersJobs;
	GlCallback m_swapBuffersCallback;
	void* m_swapBuffersCbData;
	std::condition_variable m_frameCondVar;
	std::mutex m_frameMtx;
	Bool8 m_frameWait = false;

	std::thread::id m_serverThreadId;
	
	GlState m_state;
	GLuint m_defaultVao;

	/// A special jobchain that is called every time we want to wait for the
	/// server
	GlJobChainHandle m_syncJobs;
	GlClientSyncHandle m_sync;

	String m_error;

	/// The function that the thread runs
	void threadLoop();
	void prepare();
	void finish();

	static void swapBuffersInternal(void* self);
};

/// @}

} // end namespace anki

#endif
