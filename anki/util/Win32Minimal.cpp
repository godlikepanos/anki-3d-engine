// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// This file is mainly used to test that Windows.h works with our own minimal version of it.

#include "Win32Minimal.h"
#include <Windows.h>

#define ANKI_T_STRUCT(type) \
	static_assert(sizeof(::type) == sizeof(anki::type), "Sizeof mismatch of the Windows.h type and AnKi's fake one"); \
	static_assert(alignof(::type) == alignof(anki::type), \
				  "Alignment mismatch of the Windows.h type and AnKi's fake one");

#define ANKI_T_OFFSETOF(type, member) \
	static_assert(offsetof(::type, member) == offsetof(anki::type, member), "See file");

ANKI_T_STRUCT(CRITICAL_SECTION)
ANKI_T_STRUCT(SRWLOCK)
ANKI_T_STRUCT(CONDITION_VARIABLE)
ANKI_T_STRUCT(SHFILEOPSTRUCTA)
ANKI_T_STRUCT(SECURITY_ATTRIBUTES)
ANKI_T_STRUCT(WIN32_FIND_DATAA)
ANKI_T_OFFSETOF(WIN32_FIND_DATAA, cAlternateFileName)
ANKI_T_STRUCT(LARGE_INTEGER)
ANKI_T_STRUCT(COORD)
ANKI_T_STRUCT(CONSOLE_SCREEN_BUFFER_INFO)
ANKI_T_STRUCT(SYSTEM_INFO)
ANKI_T_STRUCT(FILETIME)
ANKI_T_STRUCT(SMALL_RECT)