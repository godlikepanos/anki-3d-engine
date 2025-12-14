// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

ANKI_SVAR(AsyncTasksInFlight, StatCategory::kMisc, "Async loader tasks", StatFlag::kNone)

AsyncLoader::AsyncLoader()
	: m_thread("AsyncLoad")
{
	m_thread.start(this, threadCallback);
}

AsyncLoader::~AsyncLoader()
{
	stop();

	for(auto& queue : m_taskQueues)
	{
		if(!queue.isEmpty())
		{
			ANKI_RESOURCE_LOGW("Stoping loading thread while there is work to do");

			while(!queue.isEmpty())
			{
				AsyncLoaderTask* task = queue.popFront();
				deleteInstance(ResourceMemoryPool::getSingleton(), task);
			}
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
		AsyncLoaderPriority taskPriority = AsyncLoaderPriority::kCount;
		Bool quit = false;

		// Block until there is work to do
		{
			LockGuard<Mutex> lock(m_mtx);
			while(m_taskQueues[AsyncLoaderPriority::kHigh].isEmpty() && m_taskQueues[AsyncLoaderPriority::kMedium].isEmpty()
				  && m_taskQueues[AsyncLoaderPriority::kLow].isEmpty() && !m_quit)
			{
				m_condVar.wait(m_mtx);
			}

			if(m_quit)
			{
				quit = true;
			}
			else
			{
				for(AsyncLoaderPriority priority : EnumIterable<AsyncLoaderPriority>())
				{
					if(!m_taskQueues[priority].isEmpty())
					{
						task = m_taskQueues[priority].popFront();
						taskPriority = priority;
						break;
					}
				}
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
			ctx.m_priority = taskPriority;

			{
				// HighRezTimer::sleep(250.0_ms);
				ANKI_TRACE_SCOPED_EVENT(RsrcAsyncTask);
				err = (*task)(ctx);
				g_svarAsyncTasksInFlight.decrement(1u);
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
				m_taskQueues[ctx.m_priority].pushBack(task);
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

void AsyncLoader::submitTask(AsyncLoaderTask* task, AsyncLoaderPriority priority)
{
	ANKI_ASSERT(task);

	m_tasksInFlightCount.fetchAdd(1);
	g_svarAsyncTasksInFlight.increment(1);

	LockGuard<Mutex> lock(m_mtx);
	m_taskQueues[priority].pushBack(task);
	m_condVar.notifyOne();
}

} // end namespace anki
