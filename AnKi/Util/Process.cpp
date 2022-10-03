// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Process.h>
#include <AnKi/Util/Array.h>
#if !ANKI_OS_ANDROID
#	include <ThirdParty/Reproc/reproc/include/reproc/reproc.h>
#endif

namespace anki {

Process::~Process()
{
	destroy();
}

void Process::destroy()
{
#if !ANKI_OS_ANDROID
	if(m_handle)
	{
		ProcessStatus status;
		[[maybe_unused]] const Error err = getStatus(status);
		if(status == ProcessStatus::kRunning)
		{
			ANKI_UTIL_LOGE("Process is still running. Forgot to wait for it");
		}

		m_handle = reproc_destroy(m_handle);
	}
#endif
}

Error Process::start(CString executable, ConstWeakArray<CString> arguments, ConstWeakArray<CString> environment)
{
#if !ANKI_OS_ANDROID
	ANKI_ASSERT(m_handle == nullptr && "Already started");

	// Set args and env
	Array<const Char*, 64> args;
	Array<const Char*, 32> env;

	args[0] = executable.cstr();
	for(U32 i = 0; i < arguments.getSize(); ++i)
	{
		args[i + 1] = arguments[i].cstr();
		ANKI_ASSERT(args[i + 1]);
	}
	args[arguments.getSize() + 1] = nullptr;

	for(U32 i = 0; i < environment.getSize(); ++i)
	{
		env[i] = environment[i].cstr();
		ANKI_ASSERT(env[i]);
	}
	env[environment.getSize()] = nullptr;

	// Start process
	m_handle = reproc_new();

	reproc_options options = {};

	options.env.behavior = REPROC_ENV_EXTEND;
	if(environment.getSize())
	{
		options.env.extra = &env[0];
	}
	options.nonblocking = true;

	options.redirect.err.type = REPROC_REDIRECT_PIPE;

	I32 ret = reproc_start(m_handle, &args[0], options);
	if(ret < 0)
	{
		ANKI_UTIL_LOGE("reproc_start() failed: %s", reproc_strerror(ret));
		m_handle = reproc_destroy(m_handle);
		return Error::kUserData;
	}

	ret = reproc_close(m_handle, REPROC_STREAM_IN);
	if(ret < 0)
	{
		ANKI_UTIL_LOGE("reproc_close() failed: %s. Ignoring", reproc_strerror(ret));
		m_handle = reproc_destroy(m_handle);
		return Error::kUserData;
	}
#endif

	return Error::kNone;
}

Error Process::wait(Second timeout, ProcessStatus* pStatus, I32* pExitCode)
{
#if !ANKI_OS_ANDROID
	ANKI_ASSERT(m_handle);
	ProcessStatus status;
	I32 exitCode;

	// Compute timeout in ms
	I32 rtimeout;
	if(timeout < 0.0)
	{
		rtimeout = REPROC_INFINITE;
	}
	else
	{
		// Cap the timeout to 1h to avoid overflows when converting to ms
		if(timeout > 1.0_hour)
		{
			ANKI_UTIL_LOGW("Timeout unreasonably high (%f sec). Will cap it to 1h", timeout);
			timeout = 1.0_hour;
		}

		rtimeout = I32(timeout * 1000.0);
	}

	// Wait
	const I32 ret = reproc_wait(m_handle, rtimeout);
	if(ret == REPROC_ETIMEDOUT)
	{
		status = ProcessStatus::kRunning;
		exitCode = 0;
	}
	else
	{
		status = ProcessStatus::kNotRunning;
		exitCode = ret;
	}

	if(pStatus)
	{
		*pStatus = status;
	}

	if(pExitCode)
	{
		*pExitCode = exitCode;
	}

	ANKI_ASSERT(!(status == ProcessStatus::kRunning && timeout < 0.0));
#endif

	return Error::kNone;
}

Error Process::getStatus(ProcessStatus& status)
{
#if !ANKI_OS_ANDROID
	ANKI_ASSERT(m_handle);
	ANKI_CHECK(wait(0.0, &status, nullptr));
#endif

	return Error::kNone;
}

Error Process::kill(ProcessKillSignal k)
{
#if !ANKI_OS_ANDROID
	ANKI_ASSERT(m_handle);

	I32 ret;
	CString funcName;
	if(k == ProcessKillSignal::kNormal)
	{
		ret = reproc_terminate(m_handle);
		funcName = "reproc_terminate";
	}
	else
	{
		ANKI_ASSERT(k == ProcessKillSignal::kForce);
		ret = reproc_kill(m_handle);
		funcName = "reproc_kill";
	}

	if(ret < 0)
	{
		ANKI_UTIL_LOGE("%s() failed: %s", funcName.cstr(), reproc_strerror(ret));
		return Error::kFunctionFailed;
	}
#endif

	return Error::kNone;
}

Error Process::readFromStdout(StringRaii& text)
{
#if !ANKI_OS_ANDROID
	return readCommon(REPROC_STREAM_OUT, text);
#else
	return Error::kNone;
#endif
}

Error Process::readFromStderr(StringRaii& text)
{
#if !ANKI_OS_ANDROID
	return readCommon(REPROC_STREAM_ERR, text);
#else
	return Error::kNone;
#endif
}

#if !ANKI_OS_ANDROID
Error Process::readCommon(I32 reprocStream, StringRaii& text)
{
	ANKI_ASSERT(m_handle);

	// Limit the iterations in case the process writes to FD constantly
	U32 maxIterations = 16;
	while(maxIterations--)
	{
		Array<Char, 256> buff;

		const I32 ret =
			reproc_read(m_handle, REPROC_STREAM(reprocStream), reinterpret_cast<U8*>(&buff[0]), buff.getSize() - 1);

		if(ret == 0 || ret == REPROC_EPIPE || ret == REPROC_EWOULDBLOCK)
		{
			// No data or all data have bee read or something that I don't get
			break;
		}

		if(ret < 0)
		{
			ANKI_UTIL_LOGE("reproc_read() failed: %s", reproc_strerror(ret));
			return Error::kFunctionFailed;
		}

		buff[ret] = '\0';
		text.append(&buff[0]);
	}

	return Error::kNone;
}
#endif

} // end namespace anki
