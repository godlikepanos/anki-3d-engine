#include "anki/script/LuaBinder.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include <iostream>

namespace anki {

namespace lua_detail {

//==============================================================================
void checkArgsCount(lua_State* l, int argsCount)
{
	int actualArgsCount = lua_gettop(l);
	if(argsCount != actualArgsCount)
	{
		luaL_error(l, "Expecting %d arguments, got %d", argsCount, 
			actualArgsCount);
	}
}

} // end namespace lua_detail

//==============================================================================
static int luaPanic(lua_State* l)
{
	std::string err(lua_tostring(l, -1));
	ANKI_LOGE("Lua panic attack: " << err);
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
		throw ANKI_EXCEPTION(str);
	}
}

} // end namespace anki
