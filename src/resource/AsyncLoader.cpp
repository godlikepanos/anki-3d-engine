// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/AsyncLoader.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
AsyncLoader::AsyncLoader(const HeapAllocator<U8>& alloc)
:	m_alloc(alloc),
	m_thread("anki_async_loader")
{
	m_thread.start(this, threadCallback);
}

//==============================================================================
AsyncLoader::~AsyncLoader()
{
	stop();

	if(m_head != nullptr)
	{
		ANKI_ASSERT(m_tail != nullptr);
		ANKI_LOGW("Stoping loading thread while there is work to do");

		Task* task = m_head;

		do
		{
			Task* next = task->m_next;
			m_alloc.deleteInstance(task);
			task = next;
		}while(task != nullptr);
	}
}

//==============================================================================
void AsyncLoader::stop()
{
	{
		LockGuard<Mutex> lock(m_mtx);
		m_quit = true;
	}

	m_condVar.notifyOne();
	m_thread.join();
}

//==============================================================================
I AsyncLoader::threadCallback(Thread::Info& info)
{
	AsyncLoader& self = 
		*reinterpret_cast<AsyncLoader*>(info.m_userData);

	self.threadWorker();

	return 0;
}

//==============================================================================
void AsyncLoader::threadWorker()
{
	while(true)
	{
		Task* task;

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
		(*task)();

		// Delete the task
		m_alloc.deleteInstance(task);
	}
}

} // end namespace anki

