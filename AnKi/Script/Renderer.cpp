// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <AnKi/Script/LuaBinder.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Renderer.h>

namespace anki {

static Renderer* getRenderer(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	return &Renderer::getSingleton();
}

LuaUserDataTypeInfo luaUserDataTypeInfoRenderer = {4110901869536678112, "Renderer", LuaUserData::computeSizeForGarbageCollected<Renderer>(), nullptr,
												   nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Renderer>()
{
	return luaUserDataTypeInfoRenderer;
}

/// Pre-wrap method Renderer::getAspectRatio.
static inline int pwrapRenderergetAspectRatio(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoRenderer, ud))
	{
		return -1;
	}

	Renderer* self = ud->getData<Renderer>();

	// Call the method
	F32 ret = self->getAspectRatio();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method Renderer::getAspectRatio.
static int wrapRenderergetAspectRatio(lua_State* l)
{
	int res = pwrapRenderergetAspectRatio(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Renderer::setCurrentDebugRenderTarget.
static inline int pwrapRenderersetCurrentDebugRenderTarget(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoRenderer, ud))
	{
		return -1;
	}

	Renderer* self = ud->getData<Renderer>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setCurrentDebugRenderTarget(arg0);

	return 0;
}

/// Wrap method Renderer::setCurrentDebugRenderTarget.
static int wrapRenderersetCurrentDebugRenderTarget(lua_State* l)
{
	int res = pwrapRenderersetCurrentDebugRenderTarget(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Renderer.
static inline void wrapRenderer(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoRenderer);
	LuaBinder::pushLuaCFuncMethod(l, "getAspectRatio", wrapRenderergetAspectRatio);
	LuaBinder::pushLuaCFuncMethod(l, "setCurrentDebugRenderTarget", wrapRenderersetCurrentDebugRenderTarget);
	lua_settop(l, 0);
}

/// Pre-wrap function getRenderer.
static inline int pwrapgetRenderer(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 0)) [[unlikely]]
	{
		return -1;
	}

	// Call the function
	Renderer* ret = getRenderer(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Renderer");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoRenderer;
	ud->initPointed(&luaUserDataTypeInfoRenderer, ret);

	return 1;
}

/// Wrap function getRenderer.
static int wrapgetRenderer(lua_State* l)
{
	int res = pwrapgetRenderer(l);
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
	wrapRenderer(l);
	LuaBinder::pushLuaCFunc(l, "getRenderer", wrapgetRenderer);
}

} // end namespace anki
