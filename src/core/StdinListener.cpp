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
void StdinListener::workingFunc()
{
	setCurrentThreadName("anki_stdin");
	Array<char, 512> buff;

	while(1)
	{
		int m = read(0, &buff[0], sizeof(buff));
		buff[m] = '\0';
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			m_q.push(String(&buff[0]));
		}
	}
}

//==============================================================================
String StdinListener::getLine()
{
	String ret;
	std::lock_guard<std::mutex> lock(m_mtx);
	if(!m_q.empty())
	{
		ret = m_q.front();
		m_q.pop();
	}
	return ret;
}

//==============================================================================
void StdinListener::start()
{
	m_thrd = std::thread(&StdinListener::workingFunc, this);
}

} // end namespace anki

