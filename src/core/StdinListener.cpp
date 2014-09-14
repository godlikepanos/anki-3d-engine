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
StdinListener::StdinListener(HeapAllocator<String>& alloc)
:	m_q(alloc),
	m_thrd("anki_stdin")
{
	m_thrd.start(this, workingFunc);	
}

//==============================================================================
StdinListener::~StdinListener()
{
	m_quit = true;
	// TODO write(0, );
	m_thrd.join();
}

//==============================================================================
I StdinListener::workingFunc(Thread::Info& info)
{
	StdinListener& self = *reinterpret_cast<StdinListener*>(info.m_userData);
	Array<char, 512> buff;

	while(!self.m_quit)
	{
		I m = read(0, &buff[0], sizeof(buff));
		buff[m] = '\0';

		self.m_mtx.lock();
		auto alloc = self.m_q.get_allocator();
		self.m_q.emplace_back(&buff[0], alloc);
		self.m_mtx.unlock();
	}

	return 1;
}

//==============================================================================
String StdinListener::getLine()
{
	String ret;
	m_mtx.lock();
	
	if(!m_q.empty())
	{
		ret = m_q.front();
		m_q.pop_front();
	}

	m_mtx.unlock();

	return ret;
}

} // end namespace anki

