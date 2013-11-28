#include "anki/script/LuaBinder.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include <iostream>

namespace anki {

namespace lua_detail {

//==============================================================================
void checkArgsCount(lua_State* l, I argsCount)
{
	int actualArgsCount = lua_gettop(l);
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
	l = luaL_newstate();
	luaL_openlibs(l);
	lua_atpanic(l, &luaPanic);
}

//==============================================================================
LuaBinder::~LuaBinder()
{
	lua_close(l);
}

//==============================================================================
void LuaBinder::evalString(const char* str)
{
	int e = luaL_dostring(l, str);
	if(e)
	{
		std::string str(lua_tostring(l, -1));
		lua_pop(l, 1);
		throw ANKI_EXCEPTION("%s", str.c_str());
	}
}

} // end namespace anki
