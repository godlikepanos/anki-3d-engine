// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Process.h>

namespace anki
{

Process::~Process()
{
	// TODO
}

Error Process::start(CString executable, ConstWeakArray<CString> arguments, ConstWeakArray<CString> environment)
{
	// TODO
	return Error::NONE;
}

Error Process::kill(ProcessKillSignal k)
{
	// TODO
	return Error::NONE;
}

Error Process::readFromStdout(StringAuto& text)
{
	// TODO
	return Error::NONE;
}

Error Process::readFromStderr(StringAuto& text)
{
	// TODO
	return Error::NONE;
}

Error Process::getStatus(ProcessStatus& status)
{
	// TODO
	return Error::NONE;
}

Error Process::wait(Second timeout, ProcessStatus* status, I32* exitCode)
{
	// TODO
	return Error::NONE;
}

} // end namespace anki
