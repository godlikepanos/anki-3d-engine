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
StdinListener::StdinListener()
:	m_thrd("anki_stdin")
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
		self.m_q.push(String(&buff[0]));
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
		m_q.pop();
	}

	m_mtx.unlock();

	return ret;
}

} // end namespace anki

