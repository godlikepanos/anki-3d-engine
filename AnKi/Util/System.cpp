// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/System.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/Thread.h>
#include <cstdio>

#if ANKI_POSIX
#	include <unistd.h>
#	include <signal.h>
#elif ANKI_OS_WINDOWS
#	include <AnKi/Util/Win32Minimal.h>
#else
#	error "Unimplemented"
#endif

// For print backtrace
#if ANKI_POSIX && !ANKI_OS_ANDROID
#	include <execinfo.h>
#	include <cstdlib>
#endif

#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#	include <fcntl.h>
#endif

#if ANKI_OS_LINUX
#	include <spawn.h>
#	include <poll.h>
#	include <fcntl.h>
#	include <sys/wait.h>
#	include <cerrno>
#	include <cstring>

// The process' environment. POSIX requires the application to declare it
extern char** environ;
#endif

namespace anki {

U32 getCpuCoresCount()
{
#if ANKI_POSIX
	return U32(sysconf(_SC_NPROCESSORS_ONLN));
#elif ANKI_OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#else
#	error "Unimplemented"
#endif
}

void backtraceInternal(const Function<void(CString)>& lambda)
{
#if ANKI_POSIX && !ANKI_OS_ANDROID
	// Get addresses's for all entries on the stack
	const U32 maxStackSize = 64;
	void** array = static_cast<void**>(malloc(maxStackSize * sizeof(void*)));
	if(array)
	{
		const I32 size = ::backtrace(array, I32(maxStackSize));

		// Get symbols
		char** strings = backtrace_symbols(array, size);

		if(strings)
		{
			for(I32 i = 0; i < size; ++i)
			{
				lambda(strings[i]);
			}

			free(strings);
		}

		free(array);
	}
#else
	lambda("backtrace() not supported in " ANKI_OS_STR);
#endif
}

Bool runningFromATerminal()
{
#if ANKI_POSIX
	return isatty(fileno(stdin));
#else
	return false;
#endif
}

std::tm getLocalTime()
{
	std::time_t t = std::time(nullptr);
	std::tm tm;

#if ANKI_POSIX
	localtime_r(&t, &tm);
#elif ANKI_OS_WINDOWS
	localtime_s(&tm, &t);
#else
#	error See file
#endif

	return tm;
}

#if ANKI_OS_ANDROID
/// Get the name of the apk. Doesn't use File to open files because /proc files are a bit special.
static Error getAndroidApkName(BaseString<MemoryPoolPtrWrapper<HeapMemoryPool>>& name)
{
	const pid_t pid = getpid();

	BaseString<MemoryPoolPtrWrapper<HeapMemoryPool>> path(name.getMemoryPool());
	path.sprintf("/proc/%d/cmdline", pid);

	const int fd = open(path.cstr(), O_RDONLY);
	if(fd < 0)
	{
		ANKI_UTIL_LOGE("open() failed for: %s", path.cstr());
		return Error::kFunctionFailed;
	}

	Array<char, 128> tmp;
	const ssize_t readBytes = read(fd, &tmp[0], sizeof(tmp));
	if(readBytes < 0 || readBytes == 0)
	{
		close(fd);
		ANKI_UTIL_LOGE("read() failed for: %s", path.cstr());
		return Error::kFunctionFailed;
	}

	name = BaseString<MemoryPoolPtrWrapper<HeapMemoryPool>>('?', readBytes, name.getMemoryPool());
	memcpy(&name[0], &tmp[0], readBytes);

	close(fd);
	return Error::kNone;
}

void* getAndroidCommandLineArguments(int& argc, char**& argv)
{
	argc = 0;
	argv = 0;

	ANKI_ASSERT(g_androidApp);
	JNIEnv* env;
	g_androidApp->activity->vm->AttachCurrentThread(&env, NULL);

	// Call getIntent().getStringExtra()
	jobject me = g_androidApp->activity->clazz;

	jclass acl = env->GetObjectClass(me); // class pointer of NativeActivity;
	jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
	jobject intent = env->CallObjectMethod(me, giid); // Got our intent

	jclass icl = env->GetObjectClass(intent); // class pointer of Intent
	jmethodID gseid = env->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

	jstring jsParam1 = static_cast<jstring>(env->CallObjectMethod(intent, gseid, env->NewStringUTF("cmd")));

	// Parse the command line args
	HeapMemoryPool pool(allocAligned, nullptr, "getAndroidCommandLineArguments temp");
	BaseStringList<MemoryPoolPtrWrapper<HeapMemoryPool>> args(&pool);

	if(jsParam1)
	{
		const char* param1 = env->GetStringUTFChars(jsParam1, 0);
		args.splitString(param1, ' ');
		env->ReleaseStringUTFChars(jsParam1, param1);
	}

	// Add the apk name
	BaseString<MemoryPoolPtrWrapper<HeapMemoryPool>> apkName(&pool);
	if(!getAndroidApkName(apkName))
	{
		args.pushFront(apkName);
	}
	else
	{
		args.pushFront("unknown_apk");
	}

	// Allocate memory for all
	U32 allStringsSize = 0;
	for(const auto& s : args)
	{
		allStringsSize += s.getLength() + 1;
		++argc;
	}

	const PtrSize bufferSize = allStringsSize + sizeof(char*) * argc;
	void* buffer = mallocAligned(bufferSize, ANKI_SAFE_ALIGNMENT);

	// Set argv
	argv = static_cast<char**>(buffer);

	char* cbuffer = static_cast<char*>(buffer);
	cbuffer += sizeof(char*) * argc;

	U32 count = 0;
	for(const auto& s : args)
	{
		memcpy(cbuffer, &s[0], s.getLength() + 1);

		argv[count++] = &cbuffer[0];

		cbuffer += s.getLength() + 1;
	}
	ANKI_ASSERT(ptrToNumber(cbuffer) == ptrToNumber(buffer) + bufferSize);

	return buffer;
}

void cleanupGetAndroidCommandLineArguments(void* ptr)
{
	ANKI_ASSERT(ptr);
	freeAligned(ptr);
}
#endif

// The 1st thing that executes before main
void preMain()
{
	Logger::allocateSingleton();
	ANKI_UTIL_LOGV("Pre main executed. This should be the 1st message");
	Thread::setCurrentThreadName("AnKiMain");
}

// The last thing that executes after main
void postMain()
{
	ANKI_UTIL_LOGV("Post main executed. This should be the last message");
	Logger::freeSingleton();
}

#if ANKI_OS_WINDOWS
String errorMessageToString(DWORD errorMessageID)
{
	if(errorMessageID == 0)
	{
		return "No error";
	}

	LPSTR messageBuffer = nullptr;

	const PtrSize size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
										errorMessageID, ANKI_MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

	String message(messageBuffer, messageBuffer + size);
	LocalFree(messageBuffer);

	return message;
}

// Append an argument to a command line quoted the way CommandLineToArgvW and the CRT will parse it back
static void appendCommandLineArgument(String& cmdLine, CString arg)
{
	if(!cmdLine.isEmpty())
	{
		cmdLine += " ";
	}

	const Bool needsQuotes = arg.isEmpty() || strpbrk(arg.cstr(), " \t\n\v\"") != nullptr;
	if(!needsQuotes)
	{
		cmdLine += arg;
		return;
	}

	// Backslashes are literal unless they precede a quote, in which case they are escapes
	cmdLine += "\"";
	U32 backslashCount = 0;
	for(const Char* c = arg.cstr(); *c != '\0'; ++c)
	{
		if(*c == '\\')
		{
			++backslashCount;
			continue;
		}

		const U32 escapedBackslashCount = (*c == '"') ? backslashCount * 2 + 1 : backslashCount;
		for(U32 i = 0; i < escapedBackslashCount; ++i)
		{
			cmdLine += "\\";
		}
		cmdLine.append(c, c + 1);
		backslashCount = 0;
	}

	// Double the trailing backslashes so they don't escape the closing quote
	for(U32 i = 0; i < backslashCount * 2; ++i)
	{
		cmdLine += "\\";
	}
	cmdLine += "\"";
}

static Error readPipeUntilEof(HANDLE pipe, String& out)
{
	while(true)
	{
		Array<Char, 16 * 1024> buff;
		DWORD bytesRead = 0;
		if(!ReadFile(pipe, buff.getBegin(), DWORD(buff.getSize()), &bytesRead, nullptr))
		{
			const DWORD err = GetLastError();
			if(err == ERROR_BROKEN_PIPE)
			{
				// The child closed its end, that's the EOF
				return Error::kNone;
			}

			ANKI_UTIL_LOGE("ReadFile() failed: %s", errorMessageToString(err).cstr());
			return Error::kFunctionFailed;
		}

		if(bytesRead > 0)
		{
			out.append(buff.getBegin(), buff.getBegin() + bytesRead);
		}
	}
}
#endif

Error invokeProcess(CString executable, ConstWeakArray<CString> arguments, String* stdOut, String* stdErr, I32& exitCode)
{
	exitCode = -1;

#if ANKI_OS_LINUX
	constexpr U32 kStreamCount = 2;
	const Array<String*, kStreamCount> outStrings = {stdOut, stdErr};
	const Array<int, kStreamCount> childFds = {STDOUT_FILENO, STDERR_FILENO};

	// Read and write ends of the pipes. -1 if the stream is not captured
	Array2d<int, kStreamCount, 2> pipes;
	for(U32 i = 0; i < kStreamCount; ++i)
	{
		pipes[i][0] = pipes[i][1] = -1;
	}

	auto closeFd = [](int& fd) {
		if(fd >= 0)
		{
			close(fd);
			fd = -1;
		}
	};

	auto closeAllPipes = [&]() {
		for(U32 i = 0; i < kStreamCount; ++i)
		{
			closeFd(pipes[i][0]);
			closeFd(pipes[i][1]);
		}
	};

	for(U32 i = 0; i < kStreamCount; ++i)
	{
		if(outStrings[i])
		{
			outStrings[i]->destroy();

			// O_CLOEXEC so the child doesn't inherit the pipe ends, only the dup2'ed copies
			if(pipe2(&pipes[i][0], O_CLOEXEC) != 0)
			{
				ANKI_UTIL_LOGE("pipe2() failed: %s", strerror(errno));
				closeAllPipes();
				return Error::kFunctionFailed;
			}
		}
	}

	// Setup the child's standard streams. Non-captured ones go to /dev/null
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
	for(U32 i = 0; i < kStreamCount; ++i)
	{
		if(pipes[i][1] >= 0)
		{
			posix_spawn_file_actions_adddup2(&actions, pipes[i][1], childFds[i]);
		}
		else
		{
			posix_spawn_file_actions_addopen(&actions, childFds[i], "/dev/null", O_WRONLY, 0);
		}
	}

	DynamicArray<Char*> argv;
	argv.resize(arguments.getSize() + 2);
	argv[0] = const_cast<Char*>(executable.cstr());
	for(U32 i = 0; i < arguments.getSize(); ++i)
	{
		argv[i + 1] = const_cast<Char*>(arguments[i].cstr());
	}
	argv.getBack() = nullptr;

	pid_t pid;
	const int spawnErr = posix_spawnp(&pid, executable.cstr(), &actions, nullptr, argv.getBegin(), environ);
	posix_spawn_file_actions_destroy(&actions);

	// Close the write ends in the parent or the reads below will never see EOF
	for(U32 i = 0; i < kStreamCount; ++i)
	{
		closeFd(pipes[i][1]);
	}

	if(spawnErr != 0)
	{
		ANKI_UTIL_LOGE("posix_spawnp() failed for %s: %s", executable.cstr(), strerror(spawnErr));
		closeAllPipes();
		return Error::kFunctionFailed;
	}

	// Drain both pipes at the same time. Reading them one after the other can deadlock if the child fills the other one
	Error err = Error::kNone;
	while(!err)
	{
		Array<pollfd, kStreamCount> pollFds;
		Array<U32, kStreamCount> pollFdStreams;
		U32 pollFdCount = 0;
		for(U32 i = 0; i < kStreamCount; ++i)
		{
			if(pipes[i][0] >= 0)
			{
				pollFds[pollFdCount] = {pipes[i][0], POLLIN, 0};
				pollFdStreams[pollFdCount] = i;
				++pollFdCount;
			}
		}

		if(pollFdCount == 0)
		{
			break;
		}

		if(poll(pollFds.getBegin(), pollFdCount, -1) < 0)
		{
			if(errno != EINTR)
			{
				ANKI_UTIL_LOGE("poll() failed: %s", strerror(errno));
				err = Error::kFunctionFailed;
			}
			continue;
		}

		for(U32 p = 0; p < pollFdCount; ++p)
		{
			if(pollFds[p].revents == 0)
			{
				continue;
			}

			const U32 stream = pollFdStreams[p];
			Array<Char, 16 * 1024> buff;
			const ssize_t bytesRead = read(pipes[stream][0], buff.getBegin(), buff.getSize());
			if(bytesRead > 0)
			{
				outStrings[stream]->append(buff.getBegin(), buff.getBegin() + bytesRead);
			}
			else if(bytesRead == 0)
			{
				closeFd(pipes[stream][0]);
			}
			else if(errno != EINTR)
			{
				ANKI_UTIL_LOGE("read() failed: %s", strerror(errno));
				err = Error::kFunctionFailed;
			}
		}
	}

	// Always reap the child, even on error, to avoid zombies. If we bailed early closing the pipes will make its writes fail
	closeAllPipes();

	int status;
	while(waitpid(pid, &status, 0) < 0)
	{
		if(errno != EINTR)
		{
			ANKI_UTIL_LOGE("waitpid() failed: %s", strerror(errno));
			return Error::kFunctionFailed;
		}
	}

	if(WIFEXITED(status))
	{
		exitCode = WEXITSTATUS(status);
	}
	else
	{
		// Same convention as the shells
		exitCode = 128 + WTERMSIG(status);
		ANKI_UTIL_LOGW("Process %s was terminated by signal %d", executable.cstr(), WTERMSIG(status));
	}

	// A read error means the captured output is incomplete but the exit code is still valid
	return err;
#elif ANKI_OS_WINDOWS
	constexpr U32 kStreamCount = 2;
	const Array<String*, kStreamCount> outStrings = {stdOut, stdErr};

	// Read and write ends of the pipes. nullptr if the stream is not captured
	Array2d<HANDLE, kStreamCount, 2> pipes = {};

	// Feeds stdin and the non-captured streams
	HANDLE nullDevice = nullptr;

	auto closeHandle = [](HANDLE& h) {
		if(h)
		{
			CloseHandle(h);
			h = nullptr;
		}
	};

	auto closeAllHandles = [&]() {
		for(U32 i = 0; i < kStreamCount; ++i)
		{
			closeHandle(pipes[i][0]);
			closeHandle(pipes[i][1]);
		}
		closeHandle(nullDevice);
	};

	// All handles are created non-inheritable and only the ones the child needs become inheritable. Combined with the handle list below it
	// stops concurrent invokeProcess() calls from leaking pipe ends to each other's children, which would stall the EOF
	nullDevice = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if(nullDevice == INVALID_HANDLE_VALUE)
	{
		nullDevice = nullptr;
		ANKI_UTIL_LOGE("CreateFileA(NUL) failed: %s", errorMessageToString(GetLastError()).cstr());
		return Error::kFunctionFailed;
	}

	Array<HANDLE, kStreamCount + 1> inheritedHandles;
	U32 inheritedHandleCount = 0;
	inheritedHandles[inheritedHandleCount++] = nullDevice;

	for(U32 i = 0; i < kStreamCount; ++i)
	{
		if(outStrings[i])
		{
			outStrings[i]->destroy();

			if(!CreatePipe(&pipes[i][0], &pipes[i][1], nullptr, 0))
			{
				ANKI_UTIL_LOGE("CreatePipe() failed: %s", errorMessageToString(GetLastError()).cstr());
				closeAllHandles();
				return Error::kFunctionFailed;
			}

			inheritedHandles[inheritedHandleCount++] = pipes[i][1];
		}
	}

	for(U32 i = 0; i < inheritedHandleCount; ++i)
	{
		if(!SetHandleInformation(inheritedHandles[i], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT))
		{
			ANKI_UTIL_LOGE("SetHandleInformation() failed: %s", errorMessageToString(GetLastError()).cstr());
			closeAllHandles();
			return Error::kFunctionFailed;
		}
	}

	// Restrict inheritance to exactly these handles
	SIZE_T attribListSize = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &attribListSize); // Fails by design, it only returns the size
	DynamicArray<U64> attribListStorage;
	attribListStorage.resize(U32((attribListSize + sizeof(U64) - 1) / sizeof(U64)));
	const LPPROC_THREAD_ATTRIBUTE_LIST attribList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attribListStorage.getBegin());
	if(!InitializeProcThreadAttributeList(attribList, 1, 0, &attribListSize))
	{
		ANKI_UTIL_LOGE("InitializeProcThreadAttributeList() failed: %s", errorMessageToString(GetLastError()).cstr());
		closeAllHandles();
		return Error::kFunctionFailed;
	}

	if(!UpdateProcThreadAttribute(attribList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inheritedHandles.getBegin(),
								  inheritedHandleCount * sizeof(HANDLE), nullptr, nullptr))
	{
		ANKI_UTIL_LOGE("UpdateProcThreadAttribute() failed: %s", errorMessageToString(GetLastError()).cstr());
		DeleteProcThreadAttributeList(attribList);
		closeAllHandles();
		return Error::kFunctionFailed;
	}

	String cmdLine;
	appendCommandLineArgument(cmdLine, executable);
	for(CString arg : arguments)
	{
		appendCommandLineArgument(cmdLine, arg);
	}

	STARTUPINFOEXA startupInfo = {};
	startupInfo.StartupInfo.cb = sizeof(startupInfo);
	startupInfo.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
	startupInfo.StartupInfo.hStdInput = nullDevice;
	startupInfo.StartupInfo.hStdOutput = (pipes[0][1]) ? pipes[0][1] : nullDevice;
	startupInfo.StartupInfo.hStdError = (pipes[1][1]) ? pipes[1][1] : nullDevice;
	startupInfo.lpAttributeList = attribList;

	// A null application name makes CreateProcess search for the executable, like posix_spawnp does
	PROCESS_INFORMATION procInfo = {};
	const BOOL created = CreateProcessA(nullptr, &cmdLine[0], nullptr, nullptr, true, CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT, nullptr,
										nullptr, &startupInfo.StartupInfo, &procInfo);
	const DWORD createErr = GetLastError();
	DeleteProcThreadAttributeList(attribList);

	// Close the child's ends in the parent or the reads below will never see EOF
	for(U32 i = 0; i < kStreamCount; ++i)
	{
		closeHandle(pipes[i][1]);
	}
	closeHandle(nullDevice);

	if(!created)
	{
		ANKI_UTIL_LOGE("CreateProcessA() failed for %s: %s", executable.cstr(), errorMessageToString(createErr).cstr());
		closeAllHandles();
		return Error::kFunctionFailed;
	}

	CloseHandle(procInfo.hThread);

	// Anonymous pipes can't be waited on together so if both are captured drain stdout in another thread. Reading them one after the other can
	// deadlock if the child fills the other one
	Error err = Error::kNone;
	if(pipes[0][0] && pipes[1][0])
	{
		class StdoutReadInfo
		{
		public:
			HANDLE m_pipe;
			String* m_out;
		} stdoutReadInfo = {pipes[0][0], stdOut};

		Thread stdoutThread("AnKiProcStdout");
		stdoutThread.start(&stdoutReadInfo, [](ThreadCallbackInfo& info) -> Error {
			StdoutReadInfo& readInfo = *static_cast<StdoutReadInfo*>(info.m_userData);
			return readPipeUntilEof(readInfo.m_pipe, *readInfo.m_out);
		});

		err = readPipeUntilEof(pipes[1][0], *stdErr);

		// If stderr failed early close it so the child's writes to it fail instead of blocking and stdout eventually reaches EOF
		closeHandle(pipes[1][0]);

		const Error stdoutErr = stdoutThread.join();
		if(!err)
		{
			err = stdoutErr;
		}
	}
	else
	{
		for(U32 i = 0; i < kStreamCount; ++i)
		{
			if(pipes[i][0])
			{
				err = readPipeUntilEof(pipes[i][0], *outStrings[i]);
			}
		}
	}

	closeAllHandles();

	// Always wait for the child, even on error
	DWORD childExitCode;
	if(WaitForSingleObject(procInfo.hProcess, INFINITE) != WAIT_OBJECT_0 || !GetExitCodeProcess(procInfo.hProcess, &childExitCode))
	{
		ANKI_UTIL_LOGE("Waiting for process %s failed: %s", executable.cstr(), errorMessageToString(GetLastError()).cstr());
		CloseHandle(procInfo.hProcess);
		return Error::kFunctionFailed;
	}

	CloseHandle(procInfo.hProcess);
	exitCode = I32(childExitCode);

	// A read error means the captured output is incomplete but the exit code is still valid
	return err;
#else
	(void)executable;
	(void)arguments;
	(void)stdOut;
	(void)stdErr;
	ANKI_ASSERT(!"TODO");
	return Error::kFunctionFailed;
#endif
}

U32 getCurrentProcessId()
{
#if ANKI_OS_WINDOWS
	return GetCurrentProcessId();
#elif ANKI_POSIX
	return getpid();
#endif
}

} // end namespace anki
