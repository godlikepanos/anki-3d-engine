// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

static StatCounter g_asyncTasksInFlightStatVar(StatCategory::kMisc, "Async loader tasks", StatFlag::kNone);

AsyncLoader::AsyncLoader()
	: m_thread("AsyncLoad")
{
	m_thread.start(this, threadCallback);
}

AsyncLoader::~AsyncLoader()
{
	stop();

	if(!m_taskQueue.isEmpty())
	{
		ANKI_RESOURCE_LOGW("Stoping loading thread while there is work to do");

		while(!m_taskQueue.isEmpty())
		{
			AsyncLoaderTask* task = &m_taskQueue.getFront();
			m_taskQueue.popFront();
			deleteInstance(ResourceMemoryPool::getSingleton(), task);
		}
	}
}

void AsyncLoader::stop()
{
	{
		LockGuard<Mutex> lock(m_mtx);
		m_quit = true;
		m_condVar.notifyOne();
	}

	[[maybe_unused]] Error err = m_thread.join();
}

Error AsyncLoader::threadCallback(ThreadCallbackInfo& info)
{
	AsyncLoader& self = *reinterpret_cast<AsyncLoader*>(info.m_userData);
	return self.threadWorker();
}

Error AsyncLoader::threadWorker()
{
	Error err = Error::kNone;

	while(!err)
	{
		AsyncLoaderTask* task = nullptr;
		Bool quit = false;

		{
			// Wait for something
			LockGuard<Mutex> lock(m_mtx);
			while(m_taskQueue.isEmpty() && !m_quit)
			{
				m_condVar.wait(m_mtx);
			}

			// Do some work
			if(m_quit)
			{
				quit = true;
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
		else
		{
			// Exec the task
			ANKI_ASSERT(task);
			AsyncLoaderTaskContext ctx;

			{
				ANKI_TRACE_SCOPED_EVENT(RsrcAsyncTask);
				err = (*task)(ctx);
				g_asyncTasksInFlightStatVar.decrement(1);
			}

			if(!err)
			{
				m_tasksInFlightCount.fetchSub(1);
			}
			else
			{
				ANKI_RESOURCE_LOGE("Async loader task failed");
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
				deleteInstance(ResourceMemoryPool::getSingleton(), task);
			}
		}
	}

	return err;
}

void AsyncLoader::submitTask(AsyncLoaderTask* task)
{
	ANKI_ASSERT(task);

	m_tasksInFlightCount.fetchAdd(1);
	g_asyncTasksInFlightStatVar.increment(1);

	LockGuard<Mutex> lock(m_mtx);
	m_taskQueue.pushBack(task);
	m_condVar.notifyOne();
}

} // end namespace anki
