// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/core/StdinListener.h"
#include "anki/util/Array.h"
#include "anki/util/Thread.h"
#include <unistd.h>

namespace anki {

//==============================================================================
StdinListener::~StdinListener()
{
	m_quit = true;
	m_thrd.join();
	
	for(String& s : m_q)
	{
		s.destroy(m_alloc);
	}

	m_q.destroy(m_alloc);
}

//==============================================================================
Error StdinListener::create(HeapAllocator<String>& alloc)
{
	m_alloc = alloc;
	m_thrd.start(this, workingFunc);

	return ErrorCode::NONE;
}


//==============================================================================
Error StdinListener::workingFunc(Thread::Info& info)
{
	Error err = ErrorCode::NONE;
	StdinListener& self = *reinterpret_cast<StdinListener*>(info.m_userData);
	Array<char, 512> buff;

	while(!self.m_quit && !err)
	{
		I m = read(0, &buff[0], sizeof(buff));
		buff[m] = '\0';

		self.m_mtx.lock();
		auto alloc = self.m_alloc;
		err = self.m_q.emplaceBack(self.m_alloc);

		if(!err)
		{
			err = self.m_q.getBack().create(alloc, &buff[0]);
		}

		self.m_mtx.unlock();
	}

	return err;
}

//==============================================================================
String StdinListener::getLine()
{
	String ret;
	m_mtx.lock();
	
	if(!m_q.isEmpty())
	{
		ret = std::move(m_q.getFront());
		m_q.popFront(m_alloc);
	}

	m_mtx.unlock();

	return ret;
}

} // end namespace anki

