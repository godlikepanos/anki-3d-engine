// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_ASYNC_LOADER_H
#define ANKI_RESOURCE_ASYNC_LOADER_H

#include "anki/resource/Common.h"
#include "anki/util/Thread.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Asynchronous resource loader.
class AsyncLoader
{
public:
	/// Loader asynchronous task.
	class Task
	{
		friend class AsyncLoader;

	public:
		virtual ~Task()
		{}

		virtual ANKI_USE_RESULT Error operator()() = 0;

	private:
		Task* m_next = nullptr;
	};

	AsyncLoader();

	~AsyncLoader();

	ANKI_USE_RESULT Error create(const HeapAllocator<U8>& alloc);

	/// Create a new asynchronous loading task.
	template<typename TTask, typename... TArgs>
	ANKI_USE_RESULT Error newTask(TArgs&&... args);

private:
	HeapAllocator<U8> m_alloc;
	Thread m_thread;
	Mutex m_mtx;
	ConditionVariable m_condVar;
	Task* m_head = nullptr;
	Task* m_tail = nullptr;
	Bool8 m_quit = false;

	/// Thread callback
	static ANKI_USE_RESULT Error threadCallback(Thread::Info& info);

	Error threadWorker();

	void stop();
};

template<typename TTask, typename... TArgs>
Error AsyncLoader::newTask(TArgs&&... args)
{
	TTask* newTask = m_alloc.template newInstance<TTask>(
		std::forward<TArgs>(args)...);

	// Append task to the list
	{
		LockGuard<Mutex> lock(m_mtx);
		if(m_tail != nullptr)
		{
			ANKI_ASSERT(m_tail->m_next == nullptr);
			ANKI_ASSERT(m_head != nullptr);

			m_tail->m_next = newTask;
			m_tail = newTask;
		}
		else
		{
			ANKI_ASSERT(m_head == nullptr);
			m_head = m_tail = newTask;
		}
	}

	// Wake up the thread
	m_condVar.notifyOne();

	return ErrorCode::NONE;
}

/// @}

} // end namespace anki

#endif

