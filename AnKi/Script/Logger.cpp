// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <AnKi/Script/LuaBinder.h>

namespace anki {

// Wrap function logi.
static inline int wraplogi(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	ANKI_SCRIPT_LOGI("%s", arg0);

	return 0;
}

// Wrap function loge.
static inline int wraploge(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	ANKI_SCRIPT_LOGE("%s", arg0);

	return 0;
}

// Wrap function logw.
static inline int wraplogw(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	ANKI_SCRIPT_LOGW("%s", arg0);

	return 0;
}

// Wrap function logv.
static inline int wraplogv(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	ANKI_SCRIPT_LOGV("%s", arg0);

	return 0;
}

// Wrap the module.
void wrapModuleLogger(lua_State* l)
{
	LuaBinder::pushLuaCFunc(l, "logi", wraplogi);
	LuaBinder::pushLuaCFunc(l, "loge", wraploge);
	LuaBinder::pushLuaCFunc(l, "logw", wraplogw);
	LuaBinder::pushLuaCFunc(l, "logv", wraplogv);
}

} // end namespace anki
