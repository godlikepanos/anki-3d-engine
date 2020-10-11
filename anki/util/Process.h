// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Array.h>
#include <anki/util/String.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup util_system
/// @{

/// @memberof Process
enum class ProcessStatus : U8
{
	RUNNING,
	NOT_RUNNING,
	NORMAL_EXIT,
	CRASH_EXIT
};

/// @memberof Process
enum class ProcessKillSignal : U8
{
	NORMAL,
	FORCE
};

/// Executes external processes.
class Process
{
public:
	Process() = default;

	~Process();

	Process(const Process&) = delete;

	Process& operator=(const Process&) = delete;

	/// Start a process.
	/// @param executable The executable to start.
	/// @param arguments The command line arguments.
	/// @param environment The environment variables.
	ANKI_USE_RESULT Error start(CString executable, ConstWeakArray<CString> arguments,
								ConstWeakArray<CString> environment);

	/// Wait for the process to finish.
	/// @param timeout The time to wait. If 0.0 wait forever.
	/// @param[out] status The exit status
	/// @param[out] exitCode The exit code if the process has finished.
	ANKI_USE_RESULT Error wait(Second timeout = 0.0, ProcessStatus* status = nullptr, I32* exitCode = nullptr);

	/// Get the status.
	ANKI_USE_RESULT Error getStatus(ProcessStatus& status);

	/// Kill the process.
	ANKI_USE_RESULT Error kill(ProcessKillSignal k);

	/// Read from stdout.
	ANKI_USE_RESULT Error readFromStdout(StringAuto& text);

	/// Read from stderr.
	ANKI_USE_RESULT Error readFromStderr(StringAuto& text);

private:
#if ANKI_POSIX
	static constexpr int DEFAULT_EXIT_CODE = -1;

	int m_pid = -1;
	int m_exitCode = DEFAULT_EXIT_CODE;
	ProcessStatus m_status = ProcessStatus::NOT_RUNNING;
	Array<int, 2> m_stdoutPipe = {-1, -1};
	Array<int, 2> m_stderrPipe = {-1, -1};

	void destroyPipes();

	ANKI_USE_RESULT Error createPipes();

	ANKI_USE_RESULT Error readFromFd(int fd, StringAuto& text) const;

	/// Update some members.
	ANKI_USE_RESULT Error refresh(int waitpidOptions);
#else
	// TODO
#endif
};
/// @}

} // end namespace anki
