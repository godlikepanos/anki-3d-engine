// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
static Error getAndroidApkName(StringRaii& name)
{
	const pid_t pid = getpid();

	StringRaii path(&name.getMemoryPool());
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

	name.create('?', readBytes);
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
	StringListRaii args(&pool);

	if(jsParam1)
	{
		const char* param1 = env->GetStringUTFChars(jsParam1, 0);
		args.splitString(param1, ' ');
		env->ReleaseStringUTFChars(jsParam1, param1);
	}

	// Add the apk name
	StringRaii apkName(&pool);
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
	for(const String& s : args)
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
	for(const String& s : args)
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

void preMainInit()
{
	Thread::setCurrentThreadName("Main");
}

} // end namespace anki
