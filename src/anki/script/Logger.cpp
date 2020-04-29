// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <anki/script/LuaBinder.h>

namespace anki
{

/// Pre-wrap function logi.
static inline int pwraplogi(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 1, arg0)))
	{
		return -1;
	}

	// Call the function
	ANKI_SCRIPT_LOGI("%s", arg0);

	return 0;
}

/// Wrap function logi.
static int wraplogi(lua_State* l)
{
	int res = pwraplogi(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap function loge.
static inline int pwraploge(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 1, arg0)))
	{
		return -1;
	}

	// Call the function
	ANKI_SCRIPT_LOGE("%s", arg0);

	return 0;
}

/// Wrap function loge.
static int wraploge(lua_State* l)
{
	int res = pwraploge(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap function logw.
static inline int pwraplogw(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 1, arg0)))
	{
		return -1;
	}

	// Call the function
	ANKI_SCRIPT_LOGW("%s", arg0);

	return 0;
}

/// Wrap function logw.
static int wraplogw(lua_State* l)
{
	int res = pwraplogw(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap the module.
void wrapModuleLogger(lua_State* l)
{
	LuaBinder::pushLuaCFunc(l, "logi", wraplogi);
	LuaBinder::pushLuaCFunc(l, "loge", wraploge);
	LuaBinder::pushLuaCFunc(l, "logw", wraplogw);
}

} // end namespace anki
