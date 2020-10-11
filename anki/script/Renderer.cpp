// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <anki/script/LuaBinder.h>
#include <anki/script/ScriptManager.h>
#include <anki/Renderer.h>

namespace anki
{

static MainRenderer* getMainRenderer(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	MainRenderer* r = binder->getOtherSystems().m_renderer;
	ANKI_ASSERT(r);
	return r;
}

LuaUserDataTypeInfo luaUserDataTypeInfoDbg = {6963341295180544814, "Dbg",
											  LuaUserData::computeSizeForGarbageCollected<Dbg>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Dbg>()
{
	return luaUserDataTypeInfoDbg;
}

/// Pre-wrap method Dbg::getEnabled.
static inline int pwrapDbggetEnabled(lua_State* l)
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

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoDbg, ud))
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
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoDbg, ud))
	{
		return -1;
	}

	Dbg* self = ud->getData<Dbg>();

	// Pop arguments
	Bool arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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
	LuaBinder::createClass(l, &luaUserDataTypeInfoDbg);
	LuaBinder::pushLuaCFuncMethod(l, "getEnabled", wrapDbggetEnabled);
	LuaBinder::pushLuaCFuncMethod(l, "setEnabled", wrapDbgsetEnabled);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoMainRenderer = {-2700850970637484325, "MainRenderer",
													   LuaUserData::computeSizeForGarbageCollected<MainRenderer>(),
													   nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MainRenderer>()
{
	return luaUserDataTypeInfoMainRenderer;
}

/// Pre-wrap method MainRenderer::getAspectRatio.
static inline int pwrapMainRenderergetAspectRatio(lua_State* l)
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

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMainRenderer, ud))
	{
		return -1;
	}

	MainRenderer* self = ud->getData<MainRenderer>();

	// Call the method
	F32 ret = self->getAspectRatio();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

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

/// Pre-wrap method MainRenderer::setCurrentDebugRenderTarget.
static inline int pwrapMainRenderersetCurrentDebugRenderTarget(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMainRenderer, ud))
	{
		return -1;
	}

	MainRenderer* self = ud->getData<MainRenderer>();

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	self->getOffscreenRenderer().setCurrentDebugRenderTarget(arg0);

	return 0;
}

/// Wrap method MainRenderer::setCurrentDebugRenderTarget.
static int wrapMainRenderersetCurrentDebugRenderTarget(lua_State* l)
{
	int res = pwrapMainRenderersetCurrentDebugRenderTarget(l);
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
	LuaBinder::createClass(l, &luaUserDataTypeInfoMainRenderer);
	LuaBinder::pushLuaCFuncMethod(l, "getAspectRatio", wrapMainRenderergetAspectRatio);
	LuaBinder::pushLuaCFuncMethod(l, "setCurrentDebugRenderTarget", wrapMainRenderersetCurrentDebugRenderTarget);
	lua_settop(l, 0);
}

/// Pre-wrap function getMainRenderer.
static inline int pwrapgetMainRenderer(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 0)))
	{
		return -1;
	}

	// Call the function
	MainRenderer* ret = getMainRenderer(l);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MainRenderer");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMainRenderer;
	ud->initPointed(&luaUserDataTypeInfoMainRenderer, const_cast<MainRenderer*>(ret));

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
