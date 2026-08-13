// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <AnKi/Script/LuaBinder.h>

namespace anki {

// Wrap function round
static inline int wrapround(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	const F32 ret = std::round(arg0);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap function getRandomRange
static inline int wrapgetRandomRange(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	const F32 ret = getRandomRange(min(arg0, arg1), max(arg0, arg1));

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap the module.
void wrapModuleMisc(lua_State* l)
{
	LuaBinder::pushLuaCFunc(l, "round", wrapround);
	LuaBinder::pushLuaCFunc(l, "getRandomRange", wrapgetRandomRange);
}

} // end namespace anki
