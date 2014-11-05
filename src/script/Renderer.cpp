// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include "anki/script/LuaBinder.h"
#include "anki/Renderer.h"

namespace anki {

//==============================================================================
// Dbg                                                                         =
//==============================================================================

//==============================================================================
static const char* classnameDbg = "Dbg";

//==============================================================================
/// Pre-wrap method Dbg::getEnabled.
static inline int pwrapDbggetEnabled(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameDbg);
	ud = reinterpret_cast<UserData*>(voidp);
	Dbg* self = reinterpret_cast<Dbg*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	Bool ret = self->getEnabled();
	
	// Push return value
	lua_pushboolean(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Dbg::getEnabled.
static int wrapDbggetEnabled(lua_State* l)
{
	int res = pwrapDbggetEnabled(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Dbg::setEnabled.
static inline int pwrapDbgsetEnabled(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameDbg);
	ud = reinterpret_cast<UserData*>(voidp);
	Dbg* self = reinterpret_cast<Dbg*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	Bool arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	self->setEnabled(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method Dbg::setEnabled.
static int wrapDbgsetEnabled(lua_State* l)
{
	int res = pwrapDbgsetEnabled(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class Dbg.
static inline void wrapDbg(lua_State* l)
{
	LuaBinder::createClass(l, classnameDbg);
	LuaBinder::pushLuaCFuncMethod(l, "getEnabled", wrapDbggetEnabled);
	LuaBinder::pushLuaCFuncMethod(l, "setEnabled", wrapDbgsetEnabled);
	lua_settop(l, 0);
}

//==============================================================================
/// Wrap the module.
void wrapModuleRenderer(lua_State* l)
{
	wrapDbg(l);
}

} // end namespace anki

