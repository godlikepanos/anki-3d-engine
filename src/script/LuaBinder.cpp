#include "anki/script/LuaBinder.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include <iostream>

namespace anki {

namespace detail {

//==============================================================================
void checkArgsCount(lua_State* l, I argsCount)
{
	I actualArgsCount = lua_gettop(l);
	if(argsCount != actualArgsCount)
	{
		luaL_error(l, "Expecting %d arguments, got %d", argsCount, 
			actualArgsCount);
	}
}

//==============================================================================
void createClass(lua_State* l, const char* className)
{
	lua_newtable(l);
	lua_setglobal(l, className);
	luaL_newmetatable(l, className);
	lua_pushstring(l, "__index");
	lua_pushvalue(l, -2);  // pushes the metatable
	lua_settable(l, -3);  // metatable.__index = metatable
}

//==============================================================================
void pushCFunctionMethod(lua_State* l, const char* name, lua_CFunction luafunc)
{
	lua_pushstring(l, name);
	lua_pushcfunction(l, luafunc);
	lua_settable(l, -3);
}

//==============================================================================
void pushCFunctionStatic(lua_State* l, const char* className,
	const char* name, lua_CFunction luafunc) 
{
	lua_getglobal(l, className);
	lua_pushcfunction(l, luafunc);
	lua_setfield(l, -2, name);
	lua_pop(l, 1);
}

} // end namespace lua_detail

//==============================================================================
static int luaPanic(lua_State* l)
{
	std::string err(lua_tostring(l, -1));
	ANKI_LOGE("Lua panic attack: %s", err.c_str());
	abort();
}

//==============================================================================
LuaBinder::LuaBinder()
{
	m_alloc = Allocator<U8>(HeapMemoryPool(0));

	m_l = luaL_newstate();
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
void LuaBinder::evalString(const char* str)
{
	int e = luaL_dostring(m_l, str);
	if(e)
	{
		std::string str(lua_tostring(m_l, -1));
		lua_pop(m_l, 1);
		throw ANKI_EXCEPTION("%s", str.c_str());
	}
}

} // end namespace anki
