// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Thread.h>

namespace anki {

thread_local Array<Char, Thread::kThreadNameMaxLength + 1> Thread::m_nameTls = {};

Thread::Thread(const Char* name)
{
	if(name)
	{
		PtrSize len = strlen(name);
		len = min<PtrSize>(len, kThreadNameMaxLength);
		memcpy(&m_name[0], &name[0], len);
		m_name[len] = '\0';
	}
	else
	{
		memcpy(&m_name[0], kDefaultThreadName, strlen(kDefaultThreadName) + 1);
	}
}

const Char* Thread::getCurrentThreadName()
{
	if(m_nameTls[0] == '\0')
	{
		return kDefaultThreadName;
	}
	else
	{
		return &m_nameTls[0];
	}
}

} // end namespace anki
