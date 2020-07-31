// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// This file includes minimal declarations of Win32 API to avoid Windows.h polution.

#pragma once

// Minimal Windows.h declaration. Only what's needed.
extern "C" {

#define ANKI_WINAPI __stdcall
#define ANKI_WINBASEAPI __declspec(dllimport)
#define ANKI_DECLARE_HANDLE(name) \
	struct name##__; \
	typedef struct name##__* name

// Types
typedef void VOID;
typedef int BOOL;
typedef char CHAR;
typedef short SHORT;
typedef int INT;
typedef long LONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 ULONG_PTR;
typedef __int64 LONG_PTR;
typedef __int64 LONGLONG;
typedef ULONG_PTR SIZE_T;
typedef LONG HRESULT;
typedef BYTE BOOLEAN;
typedef ULONG_PTR DWORD_PTR;
typedef DWORD* LPDWORD;
typedef void* HANDLE;
typedef void* PVOID;
typedef void* LPVOID;
typedef const CHAR *LPCSTR, *PCSTR;
typedef const CHAR* PCZZSTR;
typedef CHAR* LPSTR;
ANKI_DECLARE_HANDLE(HWND);

typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
typedef DWORD(ANKI_WINAPI* PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION, CRITICAL_SECTION, *LPCRITICAL_SECTION, *PCRITICAL_SECTION;
typedef struct _RTL_SRWLOCK RTL_SRWLOCK, *PSRWLOCK;
typedef struct _RTL_CONDITION_VARIABLE RTL_CONDITION_VARIABLE, *PCONDITION_VARIABLE;

typedef struct _SHFILEOPSTRUCTA* LPSHFILEOPSTRUCTA;
typedef struct _WIN32_FIND_DATAA WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef union _LARGE_INTEGER LARGE_INTEGER;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _SYSTEM_INFO SYSTEM_INFO, *LPSYSTEM_INFO;

// Thread & locks
ANKI_WINBASEAPI HANDLE ANKI_WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize,
												LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter,
												DWORD dwCreationFlags, LPDWORD lpThreadId);
ANKI_WINBASEAPI DWORD_PTR ANKI_WINAPI SetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask);
ANKI_WINBASEAPI DWORD ANKI_WINAPI WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);
ANKI_WINBASEAPI BOOL ANKI_WINAPI CloseHandle(HANDLE hObject);
ANKI_WINBASEAPI DWORD ANKI_WINAPI GetCurrentThreadId(VOID);

ANKI_WINBASEAPI VOID ANKI_WINAPI InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
ANKI_WINBASEAPI VOID ANKI_WINAPI EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
ANKI_WINBASEAPI BOOL ANKI_WINAPI TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
ANKI_WINBASEAPI VOID ANKI_WINAPI LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
ANKI_WINBASEAPI VOID ANKI_WINAPI DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

ANKI_WINBASEAPI VOID ANKI_WINAPI InitializeSRWLock(PSRWLOCK SRWLock);
ANKI_WINBASEAPI VOID ANKI_WINAPI AcquireSRWLockExclusive(PSRWLOCK SRWLock);
ANKI_WINBASEAPI VOID ANKI_WINAPI AcquireSRWLockShared(PSRWLOCK SRWLock);
ANKI_WINBASEAPI VOID ANKI_WINAPI ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
ANKI_WINBASEAPI VOID ANKI_WINAPI ReleaseSRWLockShared(PSRWLOCK SRWLock);
ANKI_WINBASEAPI BOOLEAN ANKI_WINAPI TryAcquireSRWLockExclusive(PSRWLOCK SRWLock);
ANKI_WINBASEAPI BOOLEAN ANKI_WINAPI TryAcquireSRWLockShared(PSRWLOCK SRWLock);

ANKI_WINBASEAPI VOID ANKI_WINAPI InitializeConditionVariable(PCONDITION_VARIABLE ConditionVariable);
ANKI_WINBASEAPI BOOL ANKI_WINAPI SleepConditionVariableCS(PCONDITION_VARIABLE ConditionVariable,
														  PCRITICAL_SECTION CriticalSection, DWORD dwMilliseconds);
ANKI_WINBASEAPI VOID ANKI_WINAPI WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable);
ANKI_WINBASEAPI VOID ANKI_WINAPI WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable);

// Filesystem
ANKI_WINBASEAPI DWORD ANKI_WINAPI GetFileAttributesA(LPCSTR lpFileName);
ANKI_WINBASEAPI int ANKI_WINAPI SHFileOperationA(LPSHFILEOPSTRUCTA lpFileOp);
ANKI_WINBASEAPI BOOL ANKI_WINAPI CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
ANKI_WINBASEAPI HRESULT ANKI_WINAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
ANKI_WINBASEAPI HANDLE ANKI_WINAPI FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
ANKI_WINBASEAPI BOOL ANKI_WINAPI FindClose(HANDLE hFindFile);
ANKI_WINBASEAPI BOOL ANKI_WINAPI FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);

// Other
ANKI_WINBASEAPI DWORD ANKI_WINAPI GetLastError(VOID);
ANKI_WINBASEAPI BOOL ANKI_WINAPI QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
ANKI_WINBASEAPI BOOL ANKI_WINAPI QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
ANKI_WINBASEAPI VOID ANKI_WINAPI Sleep(DWORD dwMilliseconds);
ANKI_WINBASEAPI HANDLE ANKI_WINAPI GetStdHandle(DWORD nStdHandle);
ANKI_WINBASEAPI BOOL ANKI_WINAPI GetConsoleScreenBufferInfo(HANDLE hConsoleOutput,
															PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo);
ANKI_WINBASEAPI BOOL ANKI_WINAPI SetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes);
ANKI_WINBASEAPI VOID ANKI_WINAPI GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);

#undef ANKI_WINBASEAPI
#undef ANKI_DECLARE_HANDLE

} // end extern "C"

// AnKi's wrappers for various Windows.h structs. Required because we can't declare those structs in the global
// namespace since there are files that might include this header and Windows.h at the same time.
namespace anki
{

// Consts
constexpr DWORD INVALID_FILE_ATTRIBUTES = (DWORD)-1;
constexpr DWORD FILE_ATTRIBUTE_DIRECTORY = 0x00000010;
constexpr DWORD MAX_PATH = 260;
static const HANDLE INVALID_HANDLE_VALUE = (HANDLE)(LONG_PTR)-1;
constexpr DWORD ERROR_NO_MORE_FILES = 18L;
constexpr WORD FO_DELETE = 0x0003;
constexpr WORD FOF_NOCONFIRMATION = 0x0010;
constexpr WORD FOF_NOERRORUI = 0x0400;
constexpr WORD FOF_SILENT = 0x0004;
constexpr WORD CSIDL_PROFILE = 0x0028;
constexpr DWORD STD_OUTPUT_HANDLE = (DWORD)-11;
constexpr HRESULT S_OK = 0;
constexpr DWORD INFINITE = 0xFFFFFFFF;

constexpr WORD FOREGROUND_BLUE = 0x0001;
constexpr WORD FOREGROUND_GREEN = 0x0002;
constexpr WORD FOREGROUND_RED = 0x0004;
constexpr WORD FOREGROUND_INTENSITY = 0x0008;
constexpr WORD BACKGROUND_BLUE = 0x0010;
constexpr WORD BACKGROUND_GREEN = 0x0020;
constexpr WORD BACKGROUND_RED = 0x0040;

// Types
typedef union _LARGE_INTEGER
{
	struct
	{
		ULONG LowPart;
		LONG HighPart;
	} DUMMYSTRUCTNAME;
	struct
	{
		ULONG LowPart;
		LONG HighPart;
	} u;
	LONGLONG QuadPart;
} LARGE_INTEGER;

typedef struct _SECURITY_ATTRIBUTES
{
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY* Flink;
	struct _LIST_ENTRY* Blink;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _RTL_CRITICAL_SECTION_DEBUG
{
	WORD Type;
	WORD CreatorBackTraceIndex;
	struct _RTL_CRITICAL_SECTION* CriticalSection;
	LIST_ENTRY ProcessLocksList;
	DWORD EntryCount;
	DWORD ContentionCount;
	DWORD Flags;
	WORD CreatorBackTraceIndexHigh;
	WORD SpareWORD;
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;

#pragma pack(push, 8)
typedef struct _RTL_CRITICAL_SECTION
{
	PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
	LONG LockCount;
	LONG RecursionCount;
	HANDLE OwningThread;
	HANDLE LockSemaphore;
	ULONG_PTR SpinCount;
} CRITICAL_SECTION, *LPCRITICAL_SECTION, *PCRITICAL_SECTION;
#pragma pack(pop)

typedef struct _RTL_SRWLOCK
{
	PVOID Ptr;
} SRWLOCK, *PSRWLOCK;

typedef struct _RTL_CONDITION_VARIABLE
{
	PVOID Ptr;
} CONDITION_VARIABLE, *PRTL_CONDITION_VARIABLE, *PCONDITION_VARIABLE;

typedef WORD FILEOP_FLAGS;

typedef struct _SHFILEOPSTRUCTA
{
	HWND hwnd;
	UINT wFunc;
	PCZZSTR pFrom;
	PCZZSTR pTo;
	FILEOP_FLAGS fFlags;
	BOOL fAnyOperationsAborted;
	LPVOID hNameMappings;
	PCSTR lpszProgressTitle;
} SHFILEOPSTRUCTA, *LPSHFILEOPSTRUCTA;

typedef struct _FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _WIN32_FIND_DATAA
{
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	CHAR cFileName[MAX_PATH];
	CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef struct _COORD
{
	SHORT X;
	SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT
{
	SHORT Left;
	SHORT Top;
	SHORT Right;
	SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO
{
	COORD dwSize;
	COORD dwCursorPosition;
	WORD wAttributes;
	SMALL_RECT srWindow;
	COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _SYSTEM_INFO
{
	union
	{
		DWORD dwOemId; // Obsolete field...do not use
		struct
		{
			WORD wProcessorArchitecture;
			WORD wReserved;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
	DWORD dwPageSize;
	LPVOID lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress;
	DWORD_PTR dwActiveProcessorMask;
	DWORD dwNumberOfProcessors;
	DWORD dwProcessorType;
	DWORD dwAllocationGranularity;
	WORD wProcessorLevel;
	WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

// Critical section
inline void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	::InitializeCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
}

inline void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	::EnterCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
}

inline BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	return ::TryEnterCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
}

inline void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	::LeaveCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
}

inline void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	::DeleteCriticalSection(reinterpret_cast<::LPCRITICAL_SECTION>(lpCriticalSection));
}

// Shared lock
inline void InitializeSRWLock(PSRWLOCK SRWLock)
{
	::InitializeSRWLock(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

inline void AcquireSRWLockExclusive(PSRWLOCK SRWLock)
{
	::AcquireSRWLockExclusive(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

inline void AcquireSRWLockShared(PSRWLOCK SRWLock)
{
	::AcquireSRWLockShared(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

inline void ReleaseSRWLockExclusive(PSRWLOCK SRWLock)
{
	::ReleaseSRWLockExclusive(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

inline void ReleaseSRWLockShared(PSRWLOCK SRWLock)
{
	::ReleaseSRWLockShared(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

inline BOOL TryAcquireSRWLockExclusive(PSRWLOCK SRWLock)
{
	return ::TryAcquireSRWLockExclusive(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

inline BOOL TryAcquireSRWLockShared(PSRWLOCK SRWLock)
{
	return ::TryAcquireSRWLockShared(reinterpret_cast<::PSRWLOCK>(SRWLock));
}

// Condition var
inline void InitializeConditionVariable(PCONDITION_VARIABLE ConditionVariable)
{
	::InitializeConditionVariable(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable));
}

inline BOOL SleepConditionVariableCS(PCONDITION_VARIABLE ConditionVariable, PCRITICAL_SECTION CriticalSection,
									 DWORD dwMilliseconds)
{
	return ::SleepConditionVariableCS(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable),
									  reinterpret_cast<::PCRITICAL_SECTION>(CriticalSection), dwMilliseconds);
}

inline void WakeAllConditionVariable(PCONDITION_VARIABLE ConditionVariable)
{
	::WakeAllConditionVariable(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable));
}

inline void WakeConditionVariable(PCONDITION_VARIABLE ConditionVariable)
{
	::WakeConditionVariable(reinterpret_cast<::PCONDITION_VARIABLE>(ConditionVariable));
}

// Filesystem
inline DWORD GetFileAttributesA(LPCSTR lpFileName)
{
	return ::GetFileAttributesA(lpFileName);
}

inline int SHFileOperationA(LPSHFILEOPSTRUCTA lpFileOp)
{
	return ::SHFileOperationA(reinterpret_cast<::LPSHFILEOPSTRUCTA>(lpFileOp));
}

inline BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return ::CreateDirectoryA(lpPathName, reinterpret_cast<::LPSECURITY_ATTRIBUTES>(lpSecurityAttributes));
}

inline HRESULT SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
{
	return ::SHGetFolderPathA(hwnd, csidl, hToken, dwFlags, pszPath);
}

inline HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	return ::FindFirstFileA(lpFileName, reinterpret_cast<::LPWIN32_FIND_DATAA>(lpFindFileData));
}

inline BOOL FindClose(HANDLE hFindFile)
{
	return ::FindClose(hFindFile);
}

inline BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	return ::FindNextFileA(hFindFile, reinterpret_cast<::LPWIN32_FIND_DATAA>(lpFindFileData));
}

// Other
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
	return ::QueryPerformanceFrequency(reinterpret_cast<::LARGE_INTEGER*>(lpFrequency));
}

inline BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	return ::QueryPerformanceCounter(reinterpret_cast<::LARGE_INTEGER*>(lpPerformanceCount));
}

inline BOOL GetConsoleScreenBufferInfo(HANDLE hConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo)
{
	return ::GetConsoleScreenBufferInfo(hConsoleOutput,
										reinterpret_cast<::PCONSOLE_SCREEN_BUFFER_INFO>(lpConsoleScreenBufferInfo));
}

inline VOID GetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
	::GetSystemInfo(reinterpret_cast<::LPSYSTEM_INFO>(lpSystemInfo));
}

} // end namespace anki
