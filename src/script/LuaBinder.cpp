// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/script/LuaBinder.h"
#include "anki/util/Exception.h"
#include "anki/util/Logger.h"
#include <iostream>
#include <cstring>

namespace anki {

//==============================================================================
static int luaPanic(lua_State* l)
{
	std::string err(lua_tostring(l, -1));
	ANKI_LOGE("Lua panic attack: %s", err.c_str());
	abort();
}

//==============================================================================
LuaBinder::LuaBinder(Allocator<U8>& alloc, void* parent)
:	m_parent(parent)
{
	m_alloc = alloc;

	m_l = lua_newstate(luaAllocCallback, this);
	//m_l = luaL_newstate();
	luaL_openlibs(m_l);
	lua_atpanic(m_l, &luaPanic);
	lua_setuserdata(m_l, this);
}

//==============================================================================
LuaBinder::~LuaBinder()
{
	lua_close(m_l);

	ANKI_ASSERT(m_alloc.getMemoryPool().getAllocationsCount() == 0 
		&& "Leaking memory");
}

//==============================================================================
void* LuaBinder::luaAllocCallback(
	void* userData, void* ptr, PtrSize osize, PtrSize nsize)
{
	ANKI_ASSERT(userData);
	
	LuaBinder& binder = *(LuaBinder*)userData;
	void* out = nullptr;

	if(nsize == 0) 
	{
		if(ptr != nullptr)
		{
			binder.m_alloc.getMemoryPool().free(ptr);
		}
	}
	else
	{
		// Should be doing something like realloc

		if(ptr == nullptr)
		{
			out = binder.m_alloc.allocate(nsize);
		}
		else if(nsize <= osize)
		{
			out = ptr;
		}
		else
		{
			// realloc

			out = binder.m_alloc.allocate(nsize);
			std::memcpy(out, ptr, osize);
			binder.m_alloc.getMemoryPool().free(ptr);
		}
	}

	return out;
}

//==============================================================================
void LuaBinder::evalString(const CString& str)
{
	int e = luaL_dostring(m_l, &str[0]);
	if(e)
	{
		ANKI_LOGE("%s", lua_tostring(m_l, -1));
		lua_pop(m_l, 1);
	}
}

//==============================================================================
void LuaBinder::checkArgsCount(lua_State* l, I argsCount)
{
	I actualArgsCount = lua_gettop(l);
	if(argsCount != actualArgsCount)
	{
		luaL_error(l, "Expecting %d arguments, got %d", argsCount, 
			actualArgsCount);
	}
}

//==============================================================================
void LuaBinder::createClass(lua_State* l, const char* className)
{
	lua_newtable(l); // push new table
	lua_setglobal(l, className); // pop and make global
	luaL_newmetatable(l, className); // push
	lua_pushstring(l, "__index"); // push
	lua_pushvalue(l, -2);  // pushes copy of the metatable
	lua_settable(l, -3);  // pop*2: metatable.__index = metatable

	// After all these the metatable is in the top of tha stack
}

//==============================================================================
void LuaBinder::pushLuaCFuncMethod(
	lua_State* l, const char* name, lua_CFunction luafunc)
{
	lua_pushstring(l, name);
	lua_pushcfunction(l, luafunc);
	lua_settable(l, -3);
}

//==============================================================================
void LuaBinder::pushLuaCFuncStaticMethod(lua_State* l, const char* className,
	const char* name, lua_CFunction luafunc) 
{
	lua_getglobal(l, className); // push
	lua_pushcfunction(l, luafunc); // push
	lua_setfield(l, -2, name); // pop global
	lua_pop(l, 1); // pop cfunc
}

//==============================================================================
void* LuaBinder::luaAlloc(lua_State* l, size_t size)
{
	void* ud;
	lua_Alloc alloc = lua_getallocf(l, &ud);

	return alloc(ud, nullptr, 0, size);
}

//==============================================================================
void LuaBinder::luaFree(lua_State* l, void* ptr)
{
	void* ud;
	lua_Alloc alloc = lua_getallocf(l, &ud);

	alloc(ud, ptr, 0, 0);
}

} // end namespace anki
