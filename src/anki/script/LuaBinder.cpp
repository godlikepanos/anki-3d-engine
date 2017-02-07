// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/script/LuaBinder.h>
#include <anki/util/Logger.h>
#include <iostream>
#include <cstring>

namespace anki
{

static int luaPanic(lua_State* l)
{
	ANKI_SCRIPT_LOGE("Lua panic attack: %s", lua_tostring(l, -1));
	abort();
}

LuaBinder::LuaBinder()
{
}

LuaBinder::~LuaBinder()
{
	lua_close(m_l);

	ANKI_ASSERT(m_alloc.getMemoryPool().getAllocationsCount() == 0 && "Leaking memory");
}

Error LuaBinder::create(Allocator<U8>& alloc, void* parent)
{
	m_parent = parent;
	m_alloc = alloc;

	m_l = lua_newstate(luaAllocCallback, this);
	luaL_openlibs(m_l);
	lua_atpanic(m_l, &luaPanic);

	return ErrorCode::NONE;
}

void* LuaBinder::luaAllocCallback(void* userData, void* ptr, PtrSize osize, PtrSize nsize)
{
	ANKI_ASSERT(userData);

#if 1
	LuaBinder& binder = *reinterpret_cast<LuaBinder*>(userData);
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
			out = binder.m_alloc.getMemoryPool().allocate(nsize, 16);
		}
		else if(nsize <= osize)
		{
			out = ptr;
		}
		else
		{
			// realloc

			out = binder.m_alloc.getMemoryPool().allocate(nsize, 16);
			std::memcpy(out, ptr, osize);
			binder.m_alloc.getMemoryPool().free(ptr);
		}
	}
#else
	void* out = nullptr;
	if(nsize == 0)
	{
		free(ptr);
	}
	else
	{
		out = realloc(ptr, nsize);
	}
#endif

	return out;
}

Error LuaBinder::evalString(const CString& str)
{
	Error err = ErrorCode::NONE;
	int e = luaL_dostring(m_l, &str[0]);
	if(e)
	{
		ANKI_SCRIPT_LOGE("%s (line:%d)", lua_tostring(m_l, -1));
		lua_pop(m_l, 1);
		err = ErrorCode::USER_DATA;
	}

	lua_gc(m_l, LUA_GCCOLLECT, 0);

	return err;
}

void LuaBinder::checkArgsCount(lua_State* l, I argsCount)
{
	I actualArgsCount = lua_gettop(l);
	if(argsCount != actualArgsCount)
	{
		luaL_error(l, "Expecting %d arguments, got %d", argsCount, actualArgsCount);
	}
}

void LuaBinder::createClass(lua_State* l, const char* className)
{
	lua_newtable(l); // push new table
	lua_setglobal(l, className); // pop and make global
	luaL_newmetatable(l, className); // push
	lua_pushstring(l, "__index"); // push
	lua_pushvalue(l, -2); // pushes copy of the metatable
	lua_settable(l, -3); // pop*2: metatable.__index = metatable

	// After all these the metatable is in the top of tha stack
}

void LuaBinder::pushLuaCFuncMethod(lua_State* l, const char* name, lua_CFunction luafunc)
{
	lua_pushstring(l, name);
	lua_pushcfunction(l, luafunc);
	lua_settable(l, -3);
}

void LuaBinder::pushLuaCFuncStaticMethod(lua_State* l, const char* className, const char* name, lua_CFunction luafunc)
{
	lua_getglobal(l, className); // push
	lua_pushcfunction(l, luafunc); // push
	lua_setfield(l, -2, name); // pop global
	lua_pop(l, 1); // pop cfunc
}

void LuaBinder::pushLuaCFunc(lua_State* l, const char* name, lua_CFunction luafunc)
{
	lua_register(l, name, luafunc);
}

Error LuaBinder::checkNumberInternal(lua_State* l, I32 stackIdx, lua_Number& number)
{
	Error err = ErrorCode::NONE;
	lua_Number lnum;
	int isnum;

	lnum = lua_tonumberx(l, stackIdx, &isnum);
	if(isnum)
	{
		number = lnum;
	}
	else
	{
		err = ErrorCode::USER_DATA;
		lua_pushfstring(l, "Number expected. Got %s", luaL_typename(l, stackIdx));
	}

	return err;
}

Error LuaBinder::checkString(lua_State* l, I32 stackIdx, const char*& out)
{
	Error err = ErrorCode::NONE;
	const char* s = lua_tolstring(l, stackIdx, nullptr);
	if(s != nullptr)
	{
		out = s;
	}
	else
	{
		err = ErrorCode::USER_DATA;
		lua_pushfstring(l, "String expected. Got %s", luaL_typename(l, stackIdx));
	}

	return err;
}

Error LuaBinder::checkUserData(lua_State* l, I32 stackIdx, const char* typeName, I64 typeSignature, UserData*& out)
{
	Error err = ErrorCode::NONE;

	void* p = lua_touserdata(l, stackIdx);
	if(p != nullptr)
	{
		out = reinterpret_cast<UserData*>(p);
		if(out->getSig() == typeSignature)
		{
			// Check using a LUA method again
			ANKI_ASSERT(luaL_testudata(l, stackIdx, typeName) != nullptr
				&& "ANKI type check passes but LUA's type check failed");
		}
		else
		{
			// It's not the correct user data
			err = ErrorCode::USER_DATA;
		}
	}
	else
	{
		// It's not user data
		err = ErrorCode::USER_DATA;
	}

	if(err)
	{
		lua_pushfstring(l, "Userdata of %s expected. Got %s", typeName, luaL_typename(l, stackIdx));
	}

	return err;
}

void* LuaBinder::luaAlloc(lua_State* l, size_t size, U32 alignment)
{
	void* ud;
	lua_getallocf(l, &ud);
	ANKI_ASSERT(ud);
	LuaBinder* binder = static_cast<LuaBinder*>(ud);

	return binder->m_alloc.getMemoryPool().allocate(size, alignment);
}

void LuaBinder::luaFree(lua_State* l, void* ptr)
{
	void* ud;
	lua_getallocf(l, &ud);
	ANKI_ASSERT(ud);
	LuaBinder* binder = static_cast<LuaBinder*>(ud);

	binder->m_alloc.getMemoryPool().free(ptr);
}

} // end namespace anki
