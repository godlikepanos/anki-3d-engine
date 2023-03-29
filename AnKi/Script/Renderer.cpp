// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <AnKi/Script/LuaBinder.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Renderer.h>

namespace anki {

static MainRenderer* getMainRenderer(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	MainRenderer* r = nullptr; // TODO glob: fix it
	ANKI_ASSERT(r);
	return r;
}

LuaUserDataTypeInfo luaUserDataTypeInfoMainRenderer = {-6365712250974230727, "MainRenderer",
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
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
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
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
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
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
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
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 0)) [[unlikely]]
	{
		return -1;
	}

	// Call the function
	MainRenderer* ret = getMainRenderer(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MainRenderer");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMainRenderer;
	ud->initPointed(&luaUserDataTypeInfoMainRenderer, ret);

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
	wrapMainRenderer(l);
	LuaBinder::pushLuaCFunc(l, "getMainRenderer", wrapgetMainRenderer);
}

} // end namespace anki
