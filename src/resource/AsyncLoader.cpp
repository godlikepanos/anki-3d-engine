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

	if(m_head != nullptr)
	{
		ANKI_ASSERT(m_tail != nullptr);
		ANKI_LOGW("Stoping loading thread while there is work to do");

		AsyncLoaderTask* task = m_head;

		do
		{
			AsyncLoaderTask* next = task->m_next;
			m_alloc.deleteInstance(task);
			task = next;
		} while(task != nullptr);
	}
}

//==============================================================================
Error AsyncLoader::create(const HeapAllocator<U8>& alloc)
{
	m_alloc = alloc;
	m_thread.start(this, threadCallback);
	return ErrorCode::NONE;
}

//==============================================================================
void AsyncLoader::stop()
{
	{
		LockGuard<Mutex> lock(m_mtx);
		m_quit = true;
	}

	m_condVar.notifyOne();
	Error err = m_thread.join();
	(void)err;
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
		AsyncLoaderTask* task;

		{
			// Wait for something
			LockGuard<Mutex> lock(m_mtx);
			while(m_head == nullptr && m_quit == false)
			{
				m_condVar.wait(m_mtx);
			}

			// Do some work
			if(m_quit)
			{
				break;
			}

			task = m_head;

			// Update the queue
			if(m_head->m_next == nullptr)
			{
				ANKI_ASSERT(m_tail == m_head);
				m_head = m_tail = nullptr;
			}
			else
			{
				m_head = m_head->m_next;
			}
		}

		// Exec the task
		err = (*task)();
		if(err)
		{
			ANKI_LOGE("Async loader task failed");
		}

		// Delete the task
		m_alloc.deleteInstance(task);
	}

	return err;
}

} // end namespace anki
