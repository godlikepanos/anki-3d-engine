// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_QUEUE_H
#define ANKI_GL_GL_QUEUE_H

#include "anki/gl/GlCommandBufferHandle.h"
#include "anki/gl/GlSyncHandles.h"
#include "anki/gl/GlState.h"
#include "anki/util/Thread.h"

namespace anki {

// Forward
class GlDevice;

/// @addtogroup opengl_private
/// @{

/// Command queue. It's essentialy a queue of command buffers waiting for 
/// execution and a server
class GlQueue
{
public:
	/// @name Contructors/Destructor
	/// @{
	GlQueue(GlDevice* manager, 
		AllocAlignedCallback alloc, void* allocUserData);

	~GlQueue();
	/// @}

	/// @name Accessors
	/// @{
	GlDevice& getManager()
	{
		ANKI_ASSERT(m_manager);
		return *m_manager;
	}

	const GlDevice& getManager() const
	{
		ANKI_ASSERT(m_manager);
		return *m_manager;
	}

	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	void* getAllocationCallbackUserData() const
	{
		return m_allocCbUserData;
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

	/// Push a command buffer to the queue for deferred execution
	void flushCommandChain(GlCommandBufferHandle& commands);

	/// Push a command buffer to the queue and wait for it
	void finishCommandChain(GlCommandBufferHandle& commands);

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
	GlDevice* m_manager = nullptr;
	AllocAlignedCallback m_allocCb;
	void* m_allocCbUserData;

	Array<GlCommandBufferHandle, 32> m_queue; ///< Command queue
	U64 m_tail; ///< Tail of queue
	U64 m_head; ///< Head of queue. Points to the end
	U8 m_renderingThreadSignal; ///< Signal to the thread
	std::mutex m_mtx; ///< Wake the thread
	std::condition_variable m_condVar; ///< To wake up the thread
	std::thread m_thread;

	void* m_ctx = nullptr; ///< Pointer to the system GL context
	GlCallback m_makeCurrent; ///< Making a context current

	GlCommandBufferHandle m_swapBuffersCommands;
	GlCallback m_swapBuffersCallback;
	void* m_swapBuffersCbData;
	std::condition_variable m_frameCondVar;
	std::mutex m_frameMtx;
	Bool8 m_frameWait = false;

	std::thread::id m_serverThreadId;
	
	GlState m_state;
	GLuint m_defaultVao;

	/// A special command buffer that is called every time we want to wait for 
	/// the server
	GlCommandBufferHandle m_syncCommands;
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
