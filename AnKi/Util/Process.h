// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/Enum.h>

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

/// @memberof Process
enum class ProcessOptions : U8
{
	kNone = 0,
	kOpenStdout = 1 << 0,
	kOpenStderr = 1 << 1,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(ProcessOptions)

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
	Error start(CString executable, ConstWeakArray<CString> arguments = {}, ConstWeakArray<CString> environment = {},
				ProcessOptions options = ProcessOptions::kOpenStderr | ProcessOptions::kOpenStdout);

	/// Same as the other start().
	Error start(CString executable, const DynamicArray<String>& arguments, const DynamicArray<String>& environment,
				ProcessOptions options = ProcessOptions::kOpenStderr | ProcessOptions::kOpenStdout);

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
	Error readFromStdout(String& text);

	/// Read from stderr.
	Error readFromStderr(String& text);

	/// Cleanup a finished process. Call this if you want to start a new process again. Need to have waited before
	/// calling destroy.
	void destroy();

	/// Call a process and wait.
	/// @param executable The executable to start.
	/// @param arguments The command line arguments.
	/// @param stdOut Optional stdout.
	/// @param stdErr Optional stderr.
	/// @param exitCode Exit code.
	static Error callProcess(CString executable, ConstWeakArray<CString> arguments, String* stdOut, String* stdErr,
							 I32& exitCode);

private:
	static constexpr U32 kMaxArgs = 64;
	static constexpr U32 kMaxEnv = 32;

	reproc_t* m_handle = nullptr;

	Error readCommon(I32 reprocStream, String& text);

	Error startInternal(const Char* arguments[], const Char* environment[], ProcessOptions options);
};

/// Get the current process ID.
ANKI_PURE U32 getCurrentProcessId();
/// @}

} // end namespace anki
