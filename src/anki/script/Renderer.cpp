// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include <anki/script/LuaBinder.h>
#include <anki/script/ScriptManager.h>
#include <anki/Renderer.h>

namespace anki
{

static MainRenderer* getMainRenderer(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	ScriptManager* scriptManager = reinterpret_cast<ScriptManager*>(binder->getParent());

	return &scriptManager->getMainRenderer();
}

static const char* classnameDbg = "Dbg";

template<>
I64 LuaBinder::getWrappedTypeSignature<Dbg>()
{
	return -2784798555522127122;
}

template<>
const char* LuaBinder::getWrappedTypeName<Dbg>()
{
	return classnameDbg;
}

/// Pre-wrap method Dbg::getEnabled.
static inline int pwrapDbggetEnabled(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameDbg, -2784798555522127122, ud))
	{
		return -1;
	}

	Dbg* self = ud->getData<Dbg>();

	// Call the method
	Bool ret = self->getEnabled();

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

/// Wrap method Dbg::getEnabled.
static int wrapDbggetEnabled(lua_State* l)
{
	int res = pwrapDbggetEnabled(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Dbg::setEnabled.
static inline int pwrapDbgsetEnabled(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameDbg, -2784798555522127122, ud))
	{
		return -1;
	}

	Dbg* self = ud->getData<Dbg>();

	// Pop arguments
	Bool arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	self->setEnabled(arg0);

	return 0;
}

/// Wrap method Dbg::setEnabled.
static int wrapDbgsetEnabled(lua_State* l)
{
	int res = pwrapDbgsetEnabled(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Dbg.
static inline void wrapDbg(lua_State* l)
{
	LuaBinder::createClass(l, classnameDbg);
	LuaBinder::pushLuaCFuncMethod(l, "getEnabled", wrapDbggetEnabled);
	LuaBinder::pushLuaCFuncMethod(l, "setEnabled", wrapDbgsetEnabled);
	lua_settop(l, 0);
}

static const char* classnameMainRenderer = "MainRenderer";

template<>
I64 LuaBinder::getWrappedTypeSignature<MainRenderer>()
{
	return 919289102518575326;
}

template<>
const char* LuaBinder::getWrappedTypeName<MainRenderer>()
{
	return classnameMainRenderer;
}

/// Pre-wrap method MainRenderer::getAspectRatio.
static inline int pwrapMainRenderergetAspectRatio(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMainRenderer, 919289102518575326, ud))
	{
		return -1;
	}

	MainRenderer* self = ud->getData<MainRenderer>();

	// Call the method
	F32 ret = self->getAspectRatio();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method MainRenderer::getAspectRatio.
static int wrapMainRenderergetAspectRatio(lua_State* l)
{
	int res = pwrapMainRenderergetAspectRatio(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class MainRenderer.
static inline void wrapMainRenderer(lua_State* l)
{
	LuaBinder::createClass(l, classnameMainRenderer);
	LuaBinder::pushLuaCFuncMethod(l, "getAspectRatio", wrapMainRenderergetAspectRatio);
	lua_settop(l, 0);
}

/// Pre-wrap function getMainRenderer.
static inline int pwrapgetMainRenderer(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 0);

	// Call the function
	MainRenderer* ret = getMainRenderer(l);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "MainRenderer");
	ud->initPointed(919289102518575326, const_cast<MainRenderer*>(ret));

	return 1;
}

/// Wrap function getMainRenderer.
static int wrapgetMainRenderer(lua_State* l)
{
	int res = pwrapgetMainRenderer(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap the module.
void wrapModuleRenderer(lua_State* l)
{
	wrapDbg(l);
	wrapMainRenderer(l);
	LuaBinder::pushLuaCFunc(l, "getMainRenderer", wrapgetMainRenderer);
}

} // end namespace anki
