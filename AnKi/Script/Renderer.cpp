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

LuaUserDataTypeInfo g_luaUserDataTypeInfoRenderer = {-8680680372486703494, "Renderer", LuaUserData::computeSizeForGarbageCollected<Renderer>(),
													 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Renderer>()
{
	return g_luaUserDataTypeInfoRenderer;
}

// Wrap method Renderer::getAspectRatio.
static inline int wrapRenderergetAspectRatio(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoRenderer, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Renderer* self = ud->getData<Renderer>();

	// Call the method
	F32 ret = self->getAspectRatio();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Renderer::setCurrentDebugRenderTarget.
static inline int wrapRenderersetCurrentDebugRenderTarget(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoRenderer, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Renderer* self = ud->getData<Renderer>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setCurrentDebugRenderTarget(arg0);

	return 0;
}

// Wrap class Renderer.
static inline void wrapRenderer(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoRenderer);
	LuaBinder::pushLuaCFuncMethod(l, "getAspectRatio", wrapRenderergetAspectRatio);
	LuaBinder::pushLuaCFuncMethod(l, "setCurrentDebugRenderTarget", wrapRenderersetCurrentDebugRenderTarget);
	lua_settop(l, 0);
}

// Wrap function getRenderer.
static inline int wrapgetRenderer(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	Renderer* ret = getRenderer(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Renderer");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoRenderer;
	ud->initPointed(&g_luaUserDataTypeInfoRenderer, ret);

	return 1;
}

// Wrap the module.
void wrapModuleRenderer(lua_State* l)
{
	wrapRenderer(l);
	LuaBinder::pushLuaCFunc(l, "getRenderer", wrapgetRenderer);
}

} // end namespace anki
