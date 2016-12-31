// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include <anki/script/LuaBinder.h>
#include <anki/script/ScriptManager.h>
#include <anki/scene/SceneGraph.h>
#include <anki/Event.h>

namespace anki
{

static EventManager* getEventManager(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	ScriptManager* scriptManager = reinterpret_cast<ScriptManager*>(binder->getParent());

	return &scriptManager->getSceneGraph().getEventManager();
}

static const char* classnameLightEvent = "LightEvent";

template<>
I64 LuaBinder::getWrappedTypeSignature<LightEvent>()
{
	return 840634010629725278;
}

template<>
const char* LuaBinder::getWrappedTypeName<LightEvent>()
{
	return classnameLightEvent;
}

/// Pre-wrap method LightEvent::setIntensityMultiplier.
static inline int pwrapLightEventsetIntensityMultiplier(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightEvent, 840634010629725278, ud))
	{
		return -1;
	}

	LightEvent* self = ud->getData<LightEvent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setIntensityMultiplier(arg0);

	return 0;
}

/// Wrap method LightEvent::setIntensityMultiplier.
static int wrapLightEventsetIntensityMultiplier(lua_State* l)
{
	int res = pwrapLightEventsetIntensityMultiplier(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightEvent::setFrequency.
static inline int pwrapLightEventsetFrequency(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightEvent, 840634010629725278, ud))
	{
		return -1;
	}

	LightEvent* self = ud->getData<LightEvent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	self->setFrequency(arg0, arg1);

	return 0;
}

/// Wrap method LightEvent::setFrequency.
static int wrapLightEventsetFrequency(lua_State* l)
{
	int res = pwrapLightEventsetFrequency(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class LightEvent.
static inline void wrapLightEvent(lua_State* l)
{
	LuaBinder::createClass(l, classnameLightEvent);
	LuaBinder::pushLuaCFuncMethod(l, "setIntensityMultiplier", wrapLightEventsetIntensityMultiplier);
	LuaBinder::pushLuaCFuncMethod(l, "setFrequency", wrapLightEventsetFrequency);
	lua_settop(l, 0);
}

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

/// Pre-wrap method EventManager::newLightEvent.
static inline int pwrapEventManagernewLightEvent(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameEventManager, -6959305329499243407, ud))
	{
		return -1;
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1))
	{
		return -1;
	}

	if(LuaBinder::checkUserData(l, 4, "SceneNode", -2220074417980276571, ud))
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	LightEvent* ret = self->newEvent<LightEvent>(arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "LightEvent");
	ud->initPointed(840634010629725278, const_cast<LightEvent*>(ret));

	return 1;
}

/// Wrap method EventManager::newLightEvent.
static int wrapEventManagernewLightEvent(lua_State* l)
{
	int res = pwrapEventManagernewLightEvent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class EventManager.
static inline void wrapEventManager(lua_State* l)
{
	LuaBinder::createClass(l, classnameEventManager);
	LuaBinder::pushLuaCFuncMethod(l, "newLightEvent", wrapEventManagernewLightEvent);
	lua_settop(l, 0);
}

/// Pre-wrap function getEventManager.
static inline int pwrapgetEventManager(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

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
	ud->initPointed(-6959305329499243407, const_cast<EventManager*>(ret));

	return 1;
}

/// Wrap function getEventManager.
static int wrapgetEventManager(lua_State* l)
{
	int res = pwrapgetEventManager(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap the module.
void wrapModuleEvent(lua_State* l)
{
	wrapLightEvent(l);
	wrapEventManager(l);
	LuaBinder::pushLuaCFunc(l, "getEventManager", wrapgetEventManager);
}

} // end namespace anki
