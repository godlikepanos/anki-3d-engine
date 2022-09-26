// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>

// Forward
struct reproc_t;

namespace anki {

/// @addtogroup util_system
/// @{

/// @memberof Process
enum class ProcessStatus : U8
{
	kRunning,
	kNotRunning
};

/// @memberof Process
enum class ProcessKillSignal : U8
{
	kNormal,
	kForce
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
	Error start(CString executable, ConstWeakArray<CString> arguments = {}, ConstWeakArray<CString> environment = {});

	/// Wait for the process to finish.
	/// @param timeout The time to wait. If it's negative wait forever.
	/// @param[out] status The exit status
	/// @param[out] exitCode The exit code if the process has finished.
	Error wait(Second timeout = -1.0, ProcessStatus* status = nullptr, I32* exitCode = nullptr);

	/// Get the status.
	Error getStatus(ProcessStatus& status);

	/// Kill the process. Need to call wait after killing the process.
	Error kill(ProcessKillSignal k);

	/// Read from stdout.
	Error readFromStdout(StringRaii& text);

	/// Read from stderr.
	Error readFromStderr(StringRaii& text);

	/// Cleanup a finished process. Call this if you want to start a new process again. Need to have waited before
	/// calling destroy.
	void destroy();

private:
	reproc_t* m_handle = nullptr;

	Error readCommon(I32 reprocStream, StringRaii& text);
};
/// @}

} // end namespace anki
