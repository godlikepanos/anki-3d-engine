// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/StdinListener.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/Thread.h>
#include <unistd.h>

namespace anki {

StdinListener::~StdinListener()
{
	m_quit = true;
	Error err = m_thrd.join();
	if(err)
	{
		ANKI_CORE_LOGE("Error when joining StdinListener");
	}

	for(String& s : m_q)
	{
		s.destroy(*m_pool);
	}

	m_q.destroy(*m_pool);
}

Error StdinListener::create(HeapMemoryPool* pool)
{
	ANKI_ASSERT(pool);
	m_pool = pool;
	m_thrd.start(this, workingFunc);

	return Error::kNone;
}

Error StdinListener::workingFunc(ThreadCallbackInfo& info)
{
	StdinListener& self = *reinterpret_cast<StdinListener*>(info.m_userData);
	Array<char, 512> buff;

	while(!self.m_quit)
	{
		I m = read(0, &buff[0], sizeof(buff));
		buff[m] = '\0';

		LockGuard<Mutex>(self.m_mtx);
		self.m_q.emplaceBack(*self.m_pool);

		self.m_q.getBack().create(*self.m_pool, &buff[0]);
	}

	return Error::kNone;
}

String StdinListener::getLine()
{
	String ret;
	m_mtx.lock();

	if(!m_q.isEmpty())
	{
		ret = std::move(m_q.getFront());
		m_q.popFront(*m_pool);
	}

	m_mtx.unlock();

	return ret;
}

} // end namespace anki
