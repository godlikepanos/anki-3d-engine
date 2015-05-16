// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include "anki/script/LuaBinder.h"
#include "anki/script/ScriptManager.h"
#include "anki/scene/SceneGraph.h"
#include "anki/Event.h"

namespace anki {

//==============================================================================
template<typename T, typename... TArgs>
static T* newEvent(EventManager* events, TArgs... args)
{
	T* ptr;
	Error err = events->template newEvent<T>(ptr, args...);

	if(!err)
	{
		return ptr;
	}
	else
	{
		return nullptr;
	}
}

//==============================================================================
static EventManager* getEventManager(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	ScriptManager* scriptManager =
		reinterpret_cast<ScriptManager*>(binder->getParent());

	return &scriptManager->_getSceneGraph().getEventManager();
}

//==============================================================================
// SceneAmbientColorEvent                                                      =
//==============================================================================

//==============================================================================
static const char* classnameSceneAmbientColorEvent = "SceneAmbientColorEvent";

template<>
I64 LuaBinder::getWrappedTypeSignature<SceneAmbientColorEvent>()
{
	return -2736282921550252951;
}

template<>
const char* LuaBinder::getWrappedTypeName<SceneAmbientColorEvent>()
{
	return classnameSceneAmbientColorEvent;
}

//==============================================================================
/// Wrap class SceneAmbientColorEvent.
static inline void wrapSceneAmbientColorEvent(lua_State* l)
{
	LuaBinder::createClass(l, classnameSceneAmbientColorEvent);
	lua_settop(l, 0);
}

//==============================================================================
// EventManager                                                                =
//==============================================================================

//==============================================================================
static const char* classnameEventManager = "EventManager";

template<>
I64 LuaBinder::getWrappedTypeSignature<EventManager>()
{
	return -6959305329499243407;
}

template<>
const char* LuaBinder::getWrappedTypeName<EventManager>()
{
	return classnameEventManager;
}

//==============================================================================
/// Pre-wrap method EventManager::newSceneAmbientColorEvent.
static inline int pwrapEventManagernewSceneAmbientColorEvent(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 4);
	
	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameEventManager, -6959305329499243407, ud)) return -1;
	EventManager* self = static_cast<EventManager*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	if(LuaBinder::checkUserData(l, 4, "Vec4", 6804478823655046386, ud)) return -1;
	Vec4* iarg2 = static_cast<Vec4*>(ud->m_data);
	const Vec4& arg2(*iarg2);
	
	// Call the method
	SceneAmbientColorEvent* ret = newEvent<SceneAmbientColorEvent>(self, arg0, arg1, arg2);
	
	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}
	
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneAmbientColorEvent");
	ud->m_data = static_cast<void*>(ret);
	ud->m_gc = false;
	ud->m_sig = -2736282921550252951;
	
	return 1;
}

//==============================================================================
/// Wrap method EventManager::newSceneAmbientColorEvent.
static int wrapEventManagernewSceneAmbientColorEvent(lua_State* l)
{
	int res = pwrapEventManagernewSceneAmbientColorEvent(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class EventManager.
static inline void wrapEventManager(lua_State* l)
{
	LuaBinder::createClass(l, classnameEventManager);
	LuaBinder::pushLuaCFuncMethod(l, "newSceneAmbientColorEvent", wrapEventManagernewSceneAmbientColorEvent);
	lua_settop(l, 0);
}

//==============================================================================
/// Pre-wrap function getEventManager.
static inline int pwrapgetEventManager(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 0);
	
	// Call the function
	EventManager* ret = getEventManager(l);
	
	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}
	
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "EventManager");
	ud->m_data = static_cast<void*>(ret);
	ud->m_gc = false;
	ud->m_sig = -6959305329499243407;
	
	return 1;
}

//==============================================================================
/// Wrap function getEventManager.
static int wrapgetEventManager(lua_State* l)
{
	int res = pwrapgetEventManager(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap the module.
void wrapModuleEvent(lua_State* l)
{
	wrapSceneAmbientColorEvent(l);
	wrapEventManager(l);
	LuaBinder::pushLuaCFunc(l, "getEventManager", wrapgetEventManager);
}

} // end namespace anki

