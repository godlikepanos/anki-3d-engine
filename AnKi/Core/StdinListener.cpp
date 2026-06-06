// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
}

Error StdinListener::init()
{
	m_thread = new Thread("StdinListener"); // Allocate but never free. It's a pain to join the thread since when it's blocked in read()
	m_thread->start(this, workingFunc);
	return Error::kNone;
}

Error StdinListener::workingFunc(ThreadCallbackInfo& info)
{
	StdinListener& self = *reinterpret_cast<StdinListener*>(info.m_userData);
	Array<char, 512> buff;

	while(true)
	{
		const I m = read(0, &buff[0], sizeof(buff));
		if(m > 0)
		{
			buff[m] = '\0';

			LockGuard lock(self.m_mtx);
			self.m_q.pushBack(buff.getBegin());
		}
	}

	return Error::kNone;
}

Bool StdinListener::getLine(String& line)
{
	Bool gotLine = false;

	LockGuard lock(m_mtx);

	if(!m_q.isEmpty())
	{
		line = std::move(m_q.getFront());
		m_q.popFront();
		gotLine = true;
	}

	return gotLine;
}

} // end namespace anki
