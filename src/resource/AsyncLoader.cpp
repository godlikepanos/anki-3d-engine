// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/AsyncLoader.h>
#include <anki/util/Logger.h>

namespace anki
{

//==============================================================================
AsyncLoader::AsyncLoader()
	: m_thread("anki_asyload")
{
}

//==============================================================================
AsyncLoader::~AsyncLoader()
{
	stop();

	if(!m_taskQueue.isEmpty())
	{
		ANKI_LOGW("Stoping loading thread while there is work to do");

		while(!m_taskQueue.isEmpty())
		{
			AsyncLoaderTask* task = &m_taskQueue.getFront();
			m_taskQueue.popFront();
			m_alloc.deleteInstance(task);
		}
	}
}

//==============================================================================
void AsyncLoader::init(const HeapAllocator<U8>& alloc)
{
	m_alloc = alloc;
	m_thread.start(this, threadCallback);
}

//==============================================================================
void AsyncLoader::stop()
{
	{
		LockGuard<Mutex> lock(m_mtx);
		m_quit = true;
		m_condVar.notifyOne();
	}

	Error err = m_thread.join();
	(void)err;
}

//==============================================================================
void AsyncLoader::pause()
{
	{
		LockGuard<Mutex> lock(m_mtx);
		m_paused = true;
		m_sync = true;
		m_condVar.notifyOne();
	}

	m_barrier.wait();
}

//==============================================================================
void AsyncLoader::resume()
{
	LockGuard<Mutex> lock(m_mtx);
	m_paused = false;
	m_condVar.notifyOne();
}

//==============================================================================
Error AsyncLoader::threadCallback(Thread::Info& info)
{
	AsyncLoader& self = *reinterpret_cast<AsyncLoader*>(info.m_userData);
	return self.threadWorker();
}

//==============================================================================
Error AsyncLoader::threadWorker()
{
	Error err = ErrorCode::NONE;

	while(!err)
	{
		AsyncLoaderTask* task = nullptr;
		Bool quit = false;
		Bool sync = false;

		{
			// Wait for something
			LockGuard<Mutex> lock(m_mtx);
			while((m_taskQueue.isEmpty() || m_paused) && !m_quit && !m_sync)
			{
				m_condVar.wait(m_mtx);
			}

			// Do some work
			if(m_quit)
			{
				quit = true;
			}
			else if(m_sync)
			{
				sync = true;
				m_sync = false;
			}
			else
			{
				task = &m_taskQueue.getFront();
				m_taskQueue.popFront();
			}
		}

		if(quit)
		{
			break;
		}
		else if(sync)
		{
			m_barrier.wait();
		}
		else
		{
			// Exec the task
			ANKI_ASSERT(task);
			AsyncLoaderTaskContext ctx;
			err = (*task)(ctx);
			if(err)
			{
				ANKI_LOGE("Async loader task failed");
			}

			// Do other stuff
			if(ctx.m_resubmitTask)
			{
				LockGuard<Mutex> lock(m_mtx);
				m_taskQueue.pushBack(task);
			}
			else
			{
				// Delete the task
				m_alloc.deleteInstance(task);
			}

			if(ctx.m_pause)
			{
				LockGuard<Mutex> lock(m_mtx);
				m_paused = true;
			}
		}
	}

	return err;
}

} // end namespace anki
