// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <AnKi/Script/LuaBinder.h>
#include <AnKi/Script/ScriptManager.h>
#include <AnKi/Scene.h>

namespace anki {

template<typename T, typename... TArgs>
static T* newSceneNode(SceneGraph* scene, CString name, TArgs... args)
{
	return scene->template newSceneNode<T>(name, std::forward<TArgs>(args)...);
}

template<typename T, typename... TArgs>
static T* newEvent(EventManager* eventManager, TArgs... args)
{
	return eventManager->template newEvent<T>(std::forward<TArgs>(args)...);
}

static SceneGraph* getSceneGraph(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	SceneGraph* scene = &SceneGraph::getSingleton();
	ANKI_ASSERT(scene);
	return scene;
}

static EventManager* getEventManager(lua_State* l)
{
	return &getSceneGraph(l)->getEventManager();
}

using WeakArraySceneNodePtr = WeakArray<SceneNode*>;

LuaUserDataTypeInfo luaUserDataTypeInfoLightComponentType = {719535561358076220, "LightComponentType", 0, nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightComponentType>()
{
	return luaUserDataTypeInfoLightComponentType;
}

/// Wrap enum LightComponentType.
static inline void wrapLightComponentType(lua_State* l)
{
	lua_newtable(l);
	lua_setglobal(l, luaUserDataTypeInfoLightComponentType.m_typeName);
	lua_getglobal(l, luaUserDataTypeInfoLightComponentType.m_typeName);

	lua_pushstring(l, "kPoint");
	ANKI_ASSERT(LightComponentType(lua_Number(LightComponentType::kPoint)) == LightComponentType::kPoint
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(LightComponentType::kPoint));
	lua_settable(l, -3);

	lua_pushstring(l, "kSpot");
	ANKI_ASSERT(LightComponentType(lua_Number(LightComponentType::kSpot)) == LightComponentType::kSpot && "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(LightComponentType::kSpot));
	lua_settable(l, -3);

	lua_pushstring(l, "kDirectional");
	ANKI_ASSERT(LightComponentType(lua_Number(LightComponentType::kDirectional)) == LightComponentType::kDirectional
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(LightComponentType::kDirectional));
	lua_settable(l, -3);

	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoBodyComponentCollisionShapeType = {-6304619620946394365, "BodyComponentCollisionShapeType", 0, nullptr,
																		  nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<BodyComponentCollisionShapeType>()
{
	return luaUserDataTypeInfoBodyComponentCollisionShapeType;
}

/// Wrap enum BodyComponentCollisionShapeType.
static inline void wrapBodyComponentCollisionShapeType(lua_State* l)
{
	lua_newtable(l);
	lua_setglobal(l, luaUserDataTypeInfoBodyComponentCollisionShapeType.m_typeName);
	lua_getglobal(l, luaUserDataTypeInfoBodyComponentCollisionShapeType.m_typeName);

	lua_pushstring(l, "kFromMeshComponent");
	ANKI_ASSERT(BodyComponentCollisionShapeType(lua_Number(BodyComponentCollisionShapeType::kFromMeshComponent))
					== BodyComponentCollisionShapeType::kFromMeshComponent
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(BodyComponentCollisionShapeType::kFromMeshComponent));
	lua_settable(l, -3);

	lua_pushstring(l, "kAabb");
	ANKI_ASSERT(BodyComponentCollisionShapeType(lua_Number(BodyComponentCollisionShapeType::kAabb)) == BodyComponentCollisionShapeType::kAabb
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(BodyComponentCollisionShapeType::kAabb));
	lua_settable(l, -3);

	lua_pushstring(l, "kSphere");
	ANKI_ASSERT(BodyComponentCollisionShapeType(lua_Number(BodyComponentCollisionShapeType::kSphere)) == BodyComponentCollisionShapeType::kSphere
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(BodyComponentCollisionShapeType::kSphere));
	lua_settable(l, -3);

	lua_pushstring(l, "kCount");
	ANKI_ASSERT(BodyComponentCollisionShapeType(lua_Number(BodyComponentCollisionShapeType::kCount)) == BodyComponentCollisionShapeType::kCount
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(BodyComponentCollisionShapeType::kCount));
	lua_settable(l, -3);

	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr = {
	-5576003085739332971, "WeakArraySceneNodePtr", LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<WeakArraySceneNodePtr>()
{
	return luaUserDataTypeInfoWeakArraySceneNodePtr;
}

/// Pre-wrap method WeakArraySceneNodePtr::getSize.
static inline int pwrapWeakArraySceneNodePtrgetSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoWeakArraySceneNodePtr, ud))
	{
		return -1;
	}

	WeakArraySceneNodePtr* self = ud->getData<WeakArraySceneNodePtr>();

	// Call the method
	U32 ret = self->getSize();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method WeakArraySceneNodePtr::getSize.
static int wrapWeakArraySceneNodePtrgetSize(lua_State* l)
{
	int res = pwrapWeakArraySceneNodePtrgetSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method WeakArraySceneNodePtr::getAt.
static inline int pwrapWeakArraySceneNodePtrgetAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoWeakArraySceneNodePtr, ud))
	{
		return -1;
	}

	WeakArraySceneNodePtr* self = ud->getData<WeakArraySceneNodePtr>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	SceneNode* ret = (*self)[arg0];

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, ret);

	return 1;
}

/// Wrap method WeakArraySceneNodePtr::getAt.
static int wrapWeakArraySceneNodePtrgetAt(lua_State* l)
{
	int res = pwrapWeakArraySceneNodePtrgetAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class WeakArraySceneNodePtr.
static inline void wrapWeakArraySceneNodePtr(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoWeakArraySceneNodePtr);
	LuaBinder::pushLuaCFuncMethod(l, "getSize", wrapWeakArraySceneNodePtrgetSize);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapWeakArraySceneNodePtrgetAt);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoLightComponent = {3873585113943008776, "LightComponent",
														 LuaUserData::computeSizeForGarbageCollected<LightComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightComponent>()
{
	return luaUserDataTypeInfoLightComponent;
}

/// Pre-wrap method LightComponent::setLightComponentType.
static inline int pwrapLightComponentsetLightComponentType(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	lua_Number arg0Tmp;
	if(LuaBinder::checkNumber(l, 2, arg0Tmp)) [[unlikely]]
	{
		return -1;
	}
	const LightComponentType arg0 = LightComponentType(arg0Tmp);

	// Call the method
	self->setLightComponentType(arg0);

	return 0;
}

/// Wrap method LightComponent::setLightComponentType.
static int wrapLightComponentsetLightComponentType(lua_State* l)
{
	int res = pwrapLightComponentsetLightComponentType(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::setDiffuseColor.
static inline int pwrapLightComponentsetDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setDiffuseColor(arg0);

	return 0;
}

/// Wrap method LightComponent::setDiffuseColor.
static int wrapLightComponentsetDiffuseColor(lua_State* l)
{
	int res = pwrapLightComponentsetDiffuseColor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getDiffuseColor.
static inline int pwrapLightComponentgetDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	const Vec4& ret = self->getDiffuseColor();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	ud->initPointed(&luaUserDataTypeInfoVec4, &ret);

	return 1;
}

/// Wrap method LightComponent::getDiffuseColor.
static int wrapLightComponentgetDiffuseColor(lua_State* l)
{
	int res = pwrapLightComponentgetDiffuseColor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::setRadius.
static inline int pwrapLightComponentsetRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setRadius(arg0);

	return 0;
}

/// Wrap method LightComponent::setRadius.
static int wrapLightComponentsetRadius(lua_State* l)
{
	int res = pwrapLightComponentsetRadius(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getRadius.
static inline int pwrapLightComponentgetRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getRadius();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method LightComponent::getRadius.
static int wrapLightComponentgetRadius(lua_State* l)
{
	int res = pwrapLightComponentgetRadius(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::setDistance.
static inline int pwrapLightComponentsetDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setDistance(arg0);

	return 0;
}

/// Wrap method LightComponent::setDistance.
static int wrapLightComponentsetDistance(lua_State* l)
{
	int res = pwrapLightComponentsetDistance(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getDistance.
static inline int pwrapLightComponentgetDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getDistance();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method LightComponent::getDistance.
static int wrapLightComponentgetDistance(lua_State* l)
{
	int res = pwrapLightComponentgetDistance(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::setInnerAngle.
static inline int pwrapLightComponentsetInnerAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setInnerAngle(arg0);

	return 0;
}

/// Wrap method LightComponent::setInnerAngle.
static int wrapLightComponentsetInnerAngle(lua_State* l)
{
	int res = pwrapLightComponentsetInnerAngle(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getInnerAngle.
static inline int pwrapLightComponentgetInnerAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getInnerAngle();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method LightComponent::getInnerAngle.
static int wrapLightComponentgetInnerAngle(lua_State* l)
{
	int res = pwrapLightComponentgetInnerAngle(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::setOuterAngle.
static inline int pwrapLightComponentsetOuterAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setOuterAngle(arg0);

	return 0;
}

/// Wrap method LightComponent::setOuterAngle.
static int wrapLightComponentsetOuterAngle(lua_State* l)
{
	int res = pwrapLightComponentsetOuterAngle(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getOuterAngle.
static inline int pwrapLightComponentgetOuterAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getOuterAngle();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method LightComponent::getOuterAngle.
static int wrapLightComponentgetOuterAngle(lua_State* l)
{
	int res = pwrapLightComponentgetOuterAngle(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::setShadowEnabled.
static inline int pwrapLightComponentsetShadowEnabled(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	Bool arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setShadowEnabled(arg0);

	return 0;
}

/// Wrap method LightComponent::setShadowEnabled.
static int wrapLightComponentsetShadowEnabled(lua_State* l)
{
	int res = pwrapLightComponentsetShadowEnabled(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getShadowEnabled.
static inline int pwrapLightComponentgetShadowEnabled(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	Bool ret = self->getShadowEnabled();

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

/// Wrap method LightComponent::getShadowEnabled.
static int wrapLightComponentgetShadowEnabled(lua_State* l)
{
	int res = pwrapLightComponentgetShadowEnabled(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class LightComponent.
static inline void wrapLightComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoLightComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setLightComponentType", wrapLightComponentsetLightComponentType);
	LuaBinder::pushLuaCFuncMethod(l, "setDiffuseColor", wrapLightComponentsetDiffuseColor);
	LuaBinder::pushLuaCFuncMethod(l, "getDiffuseColor", wrapLightComponentgetDiffuseColor);
	LuaBinder::pushLuaCFuncMethod(l, "setRadius", wrapLightComponentsetRadius);
	LuaBinder::pushLuaCFuncMethod(l, "getRadius", wrapLightComponentgetRadius);
	LuaBinder::pushLuaCFuncMethod(l, "setDistance", wrapLightComponentsetDistance);
	LuaBinder::pushLuaCFuncMethod(l, "getDistance", wrapLightComponentgetDistance);
	LuaBinder::pushLuaCFuncMethod(l, "setInnerAngle", wrapLightComponentsetInnerAngle);
	LuaBinder::pushLuaCFuncMethod(l, "getInnerAngle", wrapLightComponentgetInnerAngle);
	LuaBinder::pushLuaCFuncMethod(l, "setOuterAngle", wrapLightComponentsetOuterAngle);
	LuaBinder::pushLuaCFuncMethod(l, "getOuterAngle", wrapLightComponentgetOuterAngle);
	LuaBinder::pushLuaCFuncMethod(l, "setShadowEnabled", wrapLightComponentsetShadowEnabled);
	LuaBinder::pushLuaCFuncMethod(l, "getShadowEnabled", wrapLightComponentgetShadowEnabled);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoDecalComponent = {6811154422270415588, "DecalComponent",
														 LuaUserData::computeSizeForGarbageCollected<DecalComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DecalComponent>()
{
	return luaUserDataTypeInfoDecalComponent;
}

/// Pre-wrap method DecalComponent::loadDiffuseImageResource.
static inline int pwrapDecalComponentloadDiffuseImageResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 3)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoDecalComponent, ud))
	{
		return -1;
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->loadDiffuseImageResource(arg0, arg1);

	return 0;
}

/// Wrap method DecalComponent::loadDiffuseImageResource.
static int wrapDecalComponentloadDiffuseImageResource(lua_State* l)
{
	int res = pwrapDecalComponentloadDiffuseImageResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method DecalComponent::loadMetalRoughnessImageResource.
static inline int pwrapDecalComponentloadMetalRoughnessImageResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 3)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoDecalComponent, ud))
	{
		return -1;
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->loadMetalRoughnessImageResource(arg0, arg1);

	return 0;
}

/// Wrap method DecalComponent::loadMetalRoughnessImageResource.
static int wrapDecalComponentloadMetalRoughnessImageResource(lua_State* l)
{
	int res = pwrapDecalComponentloadMetalRoughnessImageResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class DecalComponent.
static inline void wrapDecalComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoDecalComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadDiffuseImageResource", wrapDecalComponentloadDiffuseImageResource);
	LuaBinder::pushLuaCFuncMethod(l, "loadMetalRoughnessImageResource", wrapDecalComponentloadMetalRoughnessImageResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoLensFlareComponent = {604038026485804297, "LensFlareComponent",
															 LuaUserData::computeSizeForGarbageCollected<LensFlareComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LensFlareComponent>()
{
	return luaUserDataTypeInfoLensFlareComponent;
}

/// Pre-wrap method LensFlareComponent::loadImageResource.
static inline int pwrapLensFlareComponentloadImageResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLensFlareComponent, ud))
	{
		return -1;
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->loadImageResource(arg0);

	return 0;
}

/// Wrap method LensFlareComponent::loadImageResource.
static int wrapLensFlareComponentloadImageResource(lua_State* l)
{
	int res = pwrapLensFlareComponentloadImageResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LensFlareComponent::setFirstFlareSize.
static inline int pwrapLensFlareComponentsetFirstFlareSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLensFlareComponent, ud))
	{
		return -1;
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	self->setFirstFlareSize(arg0);

	return 0;
}

/// Wrap method LensFlareComponent::setFirstFlareSize.
static int wrapLensFlareComponentsetFirstFlareSize(lua_State* l)
{
	int res = pwrapLensFlareComponentsetFirstFlareSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LensFlareComponent::setColorMultiplier.
static inline int pwrapLensFlareComponentsetColorMultiplier(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLensFlareComponent, ud))
	{
		return -1;
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setColorMultiplier(arg0);

	return 0;
}

/// Wrap method LensFlareComponent::setColorMultiplier.
static int wrapLensFlareComponentsetColorMultiplier(lua_State* l)
{
	int res = pwrapLensFlareComponentsetColorMultiplier(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class LensFlareComponent.
static inline void wrapLensFlareComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadImageResource", wrapLensFlareComponentloadImageResource);
	LuaBinder::pushLuaCFuncMethod(l, "setFirstFlareSize", wrapLensFlareComponentsetFirstFlareSize);
	LuaBinder::pushLuaCFuncMethod(l, "setColorMultiplier", wrapLensFlareComponentsetColorMultiplier);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoBodyComponent = {-7063592649961628038, "BodyComponent",
														LuaUserData::computeSizeForGarbageCollected<BodyComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<BodyComponent>()
{
	return luaUserDataTypeInfoBodyComponent;
}

/// Pre-wrap method BodyComponent::setCollisionShapeType.
static inline int pwrapBodyComponentsetCollisionShapeType(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoBodyComponent, ud))
	{
		return -1;
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	lua_Number arg0Tmp;
	if(LuaBinder::checkNumber(l, 2, arg0Tmp)) [[unlikely]]
	{
		return -1;
	}
	const BodyComponentCollisionShapeType arg0 = BodyComponentCollisionShapeType(arg0Tmp);

	// Call the method
	self->setCollisionShapeType(arg0);

	return 0;
}

/// Wrap method BodyComponent::setCollisionShapeType.
static int wrapBodyComponentsetCollisionShapeType(lua_State* l)
{
	int res = pwrapBodyComponentsetCollisionShapeType(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::setBoxExtend.
static inline int pwrapBodyComponentsetBoxExtend(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoBodyComponent, ud))
	{
		return -1;
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setBoxExtend(arg0);

	return 0;
}

/// Wrap method BodyComponent::setBoxExtend.
static int wrapBodyComponentsetBoxExtend(lua_State* l)
{
	int res = pwrapBodyComponentsetBoxExtend(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::getBoxExtend.
static inline int pwrapBodyComponentgetBoxExtend(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoBodyComponent, ud))
	{
		return -1;
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Call the method
	const Vec3& ret = self->getBoxExtend();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	ud->initPointed(&luaUserDataTypeInfoVec3, &ret);

	return 1;
}

/// Wrap method BodyComponent::getBoxExtend.
static int wrapBodyComponentgetBoxExtend(lua_State* l)
{
	int res = pwrapBodyComponentgetBoxExtend(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::setSphereRadius.
static inline int pwrapBodyComponentsetSphereRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoBodyComponent, ud))
	{
		return -1;
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setSphereRadius(arg0);

	return 0;
}

/// Wrap method BodyComponent::setSphereRadius.
static int wrapBodyComponentsetSphereRadius(lua_State* l)
{
	int res = pwrapBodyComponentsetSphereRadius(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::getSphereRadius.
static inline int pwrapBodyComponentgetSphereRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoBodyComponent, ud))
	{
		return -1;
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Call the method
	F32 ret = self->getSphereRadius();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method BodyComponent::getSphereRadius.
static int wrapBodyComponentgetSphereRadius(lua_State* l)
{
	int res = pwrapBodyComponentgetSphereRadius(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::teleportTo.
static inline int pwrapBodyComponentteleportTo(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 3)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoBodyComponent, ud))
	{
		return -1;
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	extern LuaUserDataTypeInfo luaUserDataTypeInfoMat3;
	if(LuaBinder::checkUserData(l, 3, luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return -1;
	}

	Mat3* iarg1 = ud->getData<Mat3>();
	const Mat3& arg1(*iarg1);

	// Call the method
	self->teleportTo(arg0, arg1);

	return 0;
}

/// Wrap method BodyComponent::teleportTo.
static int wrapBodyComponentteleportTo(lua_State* l)
{
	int res = pwrapBodyComponentteleportTo(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class BodyComponent.
static inline void wrapBodyComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoBodyComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setCollisionShapeType", wrapBodyComponentsetCollisionShapeType);
	LuaBinder::pushLuaCFuncMethod(l, "setBoxExtend", wrapBodyComponentsetBoxExtend);
	LuaBinder::pushLuaCFuncMethod(l, "getBoxExtend", wrapBodyComponentgetBoxExtend);
	LuaBinder::pushLuaCFuncMethod(l, "setSphereRadius", wrapBodyComponentsetSphereRadius);
	LuaBinder::pushLuaCFuncMethod(l, "getSphereRadius", wrapBodyComponentgetSphereRadius);
	LuaBinder::pushLuaCFuncMethod(l, "teleportTo", wrapBodyComponentteleportTo);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoTriggerComponent = {-1123327168149663686, "TriggerComponent",
														   LuaUserData::computeSizeForGarbageCollected<TriggerComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<TriggerComponent>()
{
	return luaUserDataTypeInfoTriggerComponent;
}

/// Pre-wrap method TriggerComponent::getSceneNodesEnter.
static inline int pwrapTriggerComponentgetSceneNodesEnter(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoTriggerComponent, ud))
	{
		return -1;
	}

	TriggerComponent* self = ud->getData<TriggerComponent>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getSceneNodesEnter();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArraySceneNodePtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr;
	ud->initGarbageCollected(&luaUserDataTypeInfoWeakArraySceneNodePtr);
	::new(ud->getData<WeakArraySceneNodePtr>()) WeakArraySceneNodePtr(std::move(ret));

	return 1;
}

/// Wrap method TriggerComponent::getSceneNodesEnter.
static int wrapTriggerComponentgetSceneNodesEnter(lua_State* l)
{
	int res = pwrapTriggerComponentgetSceneNodesEnter(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method TriggerComponent::getSceneNodesExit.
static inline int pwrapTriggerComponentgetSceneNodesExit(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoTriggerComponent, ud))
	{
		return -1;
	}

	TriggerComponent* self = ud->getData<TriggerComponent>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getSceneNodesExit();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArraySceneNodePtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr;
	ud->initGarbageCollected(&luaUserDataTypeInfoWeakArraySceneNodePtr);
	::new(ud->getData<WeakArraySceneNodePtr>()) WeakArraySceneNodePtr(std::move(ret));

	return 1;
}

/// Wrap method TriggerComponent::getSceneNodesExit.
static int wrapTriggerComponentgetSceneNodesExit(lua_State* l)
{
	int res = pwrapTriggerComponentgetSceneNodesExit(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class TriggerComponent.
static inline void wrapTriggerComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoTriggerComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodesEnter", wrapTriggerComponentgetSceneNodesEnter);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodesExit", wrapTriggerComponentgetSceneNodesExit);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityComponent = {346637918581847223, "FogDensityComponent",
															  LuaUserData::computeSizeForGarbageCollected<FogDensityComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FogDensityComponent>()
{
	return luaUserDataTypeInfoFogDensityComponent;
}

/// Pre-wrap method FogDensityComponent::setDensity.
static inline int pwrapFogDensityComponentsetDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFogDensityComponent, ud))
	{
		return -1;
	}

	FogDensityComponent* self = ud->getData<FogDensityComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setDensity(arg0);

	return 0;
}

/// Wrap method FogDensityComponent::setDensity.
static int wrapFogDensityComponentsetDensity(lua_State* l)
{
	int res = pwrapFogDensityComponentsetDensity(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method FogDensityComponent::getDensity.
static inline int pwrapFogDensityComponentgetDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFogDensityComponent, ud))
	{
		return -1;
	}

	FogDensityComponent* self = ud->getData<FogDensityComponent>();

	// Call the method
	F32 ret = self->getDensity();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method FogDensityComponent::getDensity.
static int wrapFogDensityComponentgetDensity(lua_State* l)
{
	int res = pwrapFogDensityComponentgetDensity(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class FogDensityComponent.
static inline void wrapFogDensityComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoFogDensityComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setDensity", wrapFogDensityComponentsetDensity);
	LuaBinder::pushLuaCFuncMethod(l, "getDensity", wrapFogDensityComponentgetDensity);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoCameraComponent = {852899725312716665, "CameraComponent",
														  LuaUserData::computeSizeForGarbageCollected<CameraComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<CameraComponent>()
{
	return luaUserDataTypeInfoCameraComponent;
}

/// Pre-wrap method CameraComponent::setPerspective.
static inline int pwrapCameraComponentsetPerspective(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 5)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoCameraComponent, ud))
	{
		return -1;
	}

	CameraComponent* self = ud->getData<CameraComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2)) [[unlikely]]
	{
		return -1;
	}

	F32 arg3;
	if(LuaBinder::checkNumber(l, 5, arg3)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setPerspective(arg0, arg1, arg2, arg3);

	return 0;
}

/// Wrap method CameraComponent::setPerspective.
static int wrapCameraComponentsetPerspective(lua_State* l)
{
	int res = pwrapCameraComponentsetPerspective(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class CameraComponent.
static inline void wrapCameraComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoCameraComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setPerspective", wrapCameraComponentsetPerspective);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeComponent = {
	7683081894085046403, "GlobalIlluminationProbeComponent", LuaUserData::computeSizeForGarbageCollected<GlobalIlluminationProbeComponent>(), nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GlobalIlluminationProbeComponent>()
{
	return luaUserDataTypeInfoGlobalIlluminationProbeComponent;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::setCellSize.
static inline int pwrapGlobalIlluminationProbeComponentsetCellSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setCellSize(arg0);

	return 0;
}

/// Wrap method GlobalIlluminationProbeComponent::setCellSize.
static int wrapGlobalIlluminationProbeComponentsetCellSize(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentsetCellSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::getCellSize.
static inline int pwrapGlobalIlluminationProbeComponentgetCellSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Call the method
	F32 ret = self->getCellSize();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method GlobalIlluminationProbeComponent::getCellSize.
static int wrapGlobalIlluminationProbeComponentgetCellSize(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentgetCellSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::setFadeDistance.
static inline int pwrapGlobalIlluminationProbeComponentsetFadeDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setFadeDistance(arg0);

	return 0;
}

/// Wrap method GlobalIlluminationProbeComponent::setFadeDistance.
static int wrapGlobalIlluminationProbeComponentsetFadeDistance(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentsetFadeDistance(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::getFadeDistance.
static inline int pwrapGlobalIlluminationProbeComponentgetFadeDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Call the method
	F32 ret = self->getFadeDistance();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method GlobalIlluminationProbeComponent::getFadeDistance.
static int wrapGlobalIlluminationProbeComponentgetFadeDistance(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentgetFadeDistance(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class GlobalIlluminationProbeComponent.
static inline void wrapGlobalIlluminationProbeComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoGlobalIlluminationProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setCellSize", wrapGlobalIlluminationProbeComponentsetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "getCellSize", wrapGlobalIlluminationProbeComponentgetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "setFadeDistance", wrapGlobalIlluminationProbeComponentsetFadeDistance);
	LuaBinder::pushLuaCFuncMethod(l, "getFadeDistance", wrapGlobalIlluminationProbeComponentgetFadeDistance);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeComponent = {
	8310430098927318936, "ReflectionProbeComponent", LuaUserData::computeSizeForGarbageCollected<ReflectionProbeComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ReflectionProbeComponent>()
{
	return luaUserDataTypeInfoReflectionProbeComponent;
}

/// Wrap class ReflectionProbeComponent.
static inline void wrapReflectionProbeComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoReflectionProbeComponent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterComponent = {
	1740612382933529549, "ParticleEmitterComponent", LuaUserData::computeSizeForGarbageCollected<ParticleEmitterComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ParticleEmitterComponent>()
{
	return luaUserDataTypeInfoParticleEmitterComponent;
}

/// Pre-wrap method ParticleEmitterComponent::loadParticleEmitterResource.
static inline int pwrapParticleEmitterComponentloadParticleEmitterResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoParticleEmitterComponent, ud))
	{
		return -1;
	}

	ParticleEmitterComponent* self = ud->getData<ParticleEmitterComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->loadParticleEmitterResource(arg0);

	return 0;
}

/// Wrap method ParticleEmitterComponent::loadParticleEmitterResource.
static int wrapParticleEmitterComponentloadParticleEmitterResource(lua_State* l)
{
	int res = pwrapParticleEmitterComponentloadParticleEmitterResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ParticleEmitterComponent.
static inline void wrapParticleEmitterComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadParticleEmitterResource", wrapParticleEmitterComponentloadParticleEmitterResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoMeshComponent = {3466398713081927742, "MeshComponent",
														LuaUserData::computeSizeForGarbageCollected<MeshComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MeshComponent>()
{
	return luaUserDataTypeInfoMeshComponent;
}

/// Pre-wrap method MeshComponent::setMeshFilename.
static inline int pwrapMeshComponentsetMeshFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMeshComponent, ud))
	{
		return -1;
	}

	MeshComponent* self = ud->getData<MeshComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	MeshComponent& ret = self->setMeshFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MeshComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMeshComponent;
	ud->initPointed(&luaUserDataTypeInfoMeshComponent, &ret);

	return 1;
}

/// Wrap method MeshComponent::setMeshFilename.
static int wrapMeshComponentsetMeshFilename(lua_State* l)
{
	int res = pwrapMeshComponentsetMeshFilename(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class MeshComponent.
static inline void wrapMeshComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoMeshComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setMeshFilename", wrapMeshComponentsetMeshFilename);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoMaterialComponent = {840835603277348057, "MaterialComponent",
															LuaUserData::computeSizeForGarbageCollected<MaterialComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MaterialComponent>()
{
	return luaUserDataTypeInfoMaterialComponent;
}

/// Pre-wrap method MaterialComponent::setMaterialFilename.
static inline int pwrapMaterialComponentsetMaterialFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMaterialComponent, ud))
	{
		return -1;
	}

	MaterialComponent* self = ud->getData<MaterialComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	MaterialComponent& ret = self->setMaterialFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&luaUserDataTypeInfoMaterialComponent, &ret);

	return 1;
}

/// Wrap method MaterialComponent::setMaterialFilename.
static int wrapMaterialComponentsetMaterialFilename(lua_State* l)
{
	int res = pwrapMaterialComponentsetMaterialFilename(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MaterialComponent::setSubmeshIndex.
static inline int pwrapMaterialComponentsetSubmeshIndex(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMaterialComponent, ud))
	{
		return -1;
	}

	MaterialComponent* self = ud->getData<MaterialComponent>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	MaterialComponent& ret = self->setSubmeshIndex(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&luaUserDataTypeInfoMaterialComponent, &ret);

	return 1;
}

/// Wrap method MaterialComponent::setSubmeshIndex.
static int wrapMaterialComponentsetSubmeshIndex(lua_State* l)
{
	int res = pwrapMaterialComponentsetSubmeshIndex(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class MaterialComponent.
static inline void wrapMaterialComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoMaterialComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setMaterialFilename", wrapMaterialComponentsetMaterialFilename);
	LuaBinder::pushLuaCFuncMethod(l, "setSubmeshIndex", wrapMaterialComponentsetSubmeshIndex);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSkinComponent = {-1564186081984009359, "SkinComponent",
														LuaUserData::computeSizeForGarbageCollected<SkinComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SkinComponent>()
{
	return luaUserDataTypeInfoSkinComponent;
}

/// Pre-wrap method SkinComponent::loadSkeletonResource.
static inline int pwrapSkinComponentloadSkeletonResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkinComponent, ud))
	{
		return -1;
	}

	SkinComponent* self = ud->getData<SkinComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->loadSkeletonResource(arg0);

	return 0;
}

/// Wrap method SkinComponent::loadSkeletonResource.
static int wrapSkinComponentloadSkeletonResource(lua_State* l)
{
	int res = pwrapSkinComponentloadSkeletonResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SkinComponent.
static inline void wrapSkinComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSkinComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadSkeletonResource", wrapSkinComponentloadSkeletonResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSkyboxComponent = {2242564588996850216, "SkyboxComponent",
														  LuaUserData::computeSizeForGarbageCollected<SkyboxComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SkyboxComponent>()
{
	return luaUserDataTypeInfoSkyboxComponent;
}

/// Pre-wrap method SkyboxComponent::setSolidColor.
static inline int pwrapSkyboxComponentsetSolidColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setSolidColor(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setSolidColor.
static int wrapSkyboxComponentsetSolidColor(lua_State* l)
{
	int res = pwrapSkyboxComponentsetSolidColor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::loadImageResource.
static inline int pwrapSkyboxComponentloadImageResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->loadImageResource(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::loadImageResource.
static int wrapSkyboxComponentloadImageResource(lua_State* l)
{
	int res = pwrapSkyboxComponentloadImageResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setGeneratedSky.
static inline int pwrapSkyboxComponentsetGeneratedSky(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Call the method
	self->setGeneratedSky();

	return 0;
}

/// Wrap method SkyboxComponent::setGeneratedSky.
static int wrapSkyboxComponentsetGeneratedSky(lua_State* l)
{
	int res = pwrapSkyboxComponentsetGeneratedSky(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setMinFogDensity.
static inline int pwrapSkyboxComponentsetMinFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setMinFogDensity(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setMinFogDensity.
static int wrapSkyboxComponentsetMinFogDensity(lua_State* l)
{
	int res = pwrapSkyboxComponentsetMinFogDensity(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setMaxFogDensity.
static inline int pwrapSkyboxComponentsetMaxFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setMaxFogDensity(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setMaxFogDensity.
static int wrapSkyboxComponentsetMaxFogDensity(lua_State* l)
{
	int res = pwrapSkyboxComponentsetMaxFogDensity(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setHeightOfMinFogDensity.
static inline int pwrapSkyboxComponentsetHeightOfMinFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setHeightOfMinFogDensity(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setHeightOfMinFogDensity.
static int wrapSkyboxComponentsetHeightOfMinFogDensity(lua_State* l)
{
	int res = pwrapSkyboxComponentsetHeightOfMinFogDensity(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setHeightOfMaxFogDensity.
static inline int pwrapSkyboxComponentsetHeightOfMaxFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	self->setHeightOfMaxFogDensity(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setHeightOfMaxFogDensity.
static int wrapSkyboxComponentsetHeightOfMaxFogDensity(lua_State* l)
{
	int res = pwrapSkyboxComponentsetHeightOfMaxFogDensity(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setFogDiffuseColor.
static inline int pwrapSkyboxComponentsetFogDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setFogDiffuseColor(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setFogDiffuseColor.
static int wrapSkyboxComponentsetFogDiffuseColor(lua_State* l)
{
	int res = pwrapSkyboxComponentsetFogDiffuseColor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setImageBias.
static inline int pwrapSkyboxComponentsetImageBias(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setImageBias(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setImageBias.
static int wrapSkyboxComponentsetImageBias(lua_State* l)
{
	int res = pwrapSkyboxComponentsetImageBias(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SkyboxComponent::setImageScale.
static inline int pwrapSkyboxComponentsetImageScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxComponent, ud))
	{
		return -1;
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setImageScale(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setImageScale.
static int wrapSkyboxComponentsetImageScale(lua_State* l)
{
	int res = pwrapSkyboxComponentsetImageScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SkyboxComponent.
static inline void wrapSkyboxComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSkyboxComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setSolidColor", wrapSkyboxComponentsetSolidColor);
	LuaBinder::pushLuaCFuncMethod(l, "loadImageResource", wrapSkyboxComponentloadImageResource);
	LuaBinder::pushLuaCFuncMethod(l, "setGeneratedSky", wrapSkyboxComponentsetGeneratedSky);
	LuaBinder::pushLuaCFuncMethod(l, "setMinFogDensity", wrapSkyboxComponentsetMinFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setMaxFogDensity", wrapSkyboxComponentsetMaxFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setHeightOfMinFogDensity", wrapSkyboxComponentsetHeightOfMinFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setHeightOfMaxFogDensity", wrapSkyboxComponentsetHeightOfMaxFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setFogDiffuseColor", wrapSkyboxComponentsetFogDiffuseColor);
	LuaBinder::pushLuaCFuncMethod(l, "setImageBias", wrapSkyboxComponentsetImageBias);
	LuaBinder::pushLuaCFuncMethod(l, "setImageScale", wrapSkyboxComponentsetImageScale);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode = {2187633709272075521, "SceneNode", LuaUserData::computeSizeForGarbageCollected<SceneNode>(),
													nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneNode>()
{
	return luaUserDataTypeInfoSceneNode;
}

/// Pre-wrap method SceneNode::getName.
static inline int pwrapSceneNodegetName(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	CString ret = self->getName();

	// Push return value
	lua_pushstring(l, &ret[0]);

	return 1;
}

/// Wrap method SceneNode::getName.
static int wrapSceneNodegetName(lua_State* l)
{
	int res = pwrapSceneNodegetName(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::addChild.
static inline int pwrapSceneNodeaddChild(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return -1;
	}

	SceneNode* iarg0 = ud->getData<SceneNode>();
	SceneNode* arg0(iarg0);

	// Call the method
	self->addChild(arg0);

	return 0;
}

/// Wrap method SceneNode::addChild.
static int wrapSceneNodeaddChild(lua_State* l)
{
	int res = pwrapSceneNodeaddChild(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::markForDeletion.
static inline int pwrapSceneNodemarkForDeletion(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	self->markForDeletion();

	return 0;
}

/// Wrap method SceneNode::markForDeletion.
static int wrapSceneNodemarkForDeletion(lua_State* l)
{
	int res = pwrapSceneNodemarkForDeletion(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::setLocalOrigin.
static inline int pwrapSceneNodesetLocalOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setLocalOrigin(arg0);

	return 0;
}

/// Wrap method SceneNode::setLocalOrigin.
static int wrapSceneNodesetLocalOrigin(lua_State* l)
{
	int res = pwrapSceneNodesetLocalOrigin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getLocalOrigin.
static inline int pwrapSceneNodegetLocalOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	Vec3 ret = self->getLocalOrigin();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method SceneNode::getLocalOrigin.
static int wrapSceneNodegetLocalOrigin(lua_State* l)
{
	int res = pwrapSceneNodegetLocalOrigin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::setLocalRotation.
static inline int pwrapSceneNodesetLocalRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMat3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return -1;
	}

	Mat3* iarg0 = ud->getData<Mat3>();
	const Mat3& arg0(*iarg0);

	// Call the method
	self->setLocalRotation(arg0);

	return 0;
}

/// Wrap method SceneNode::setLocalRotation.
static int wrapSceneNodesetLocalRotation(lua_State* l)
{
	int res = pwrapSceneNodesetLocalRotation(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getLocalRotation.
static inline int pwrapSceneNodegetLocalRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	Mat3 ret = self->getLocalRotation();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Mat3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Mat3");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMat3;
	ud->initGarbageCollected(&luaUserDataTypeInfoMat3);
	::new(ud->getData<Mat3>()) Mat3(std::move(ret));

	return 1;
}

/// Wrap method SceneNode::getLocalRotation.
static int wrapSceneNodegetLocalRotation(lua_State* l)
{
	int res = pwrapSceneNodegetLocalRotation(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::setLocalScale.
static inline int pwrapSceneNodesetLocalScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setLocalScale(arg0);

	return 0;
}

/// Wrap method SceneNode::setLocalScale.
static int wrapSceneNodesetLocalScale(lua_State* l)
{
	int res = pwrapSceneNodesetLocalScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getLocalScale.
static inline int pwrapSceneNodegetLocalScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	Vec3 ret = self->getLocalScale();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method SceneNode::getLocalScale.
static int wrapSceneNodegetLocalScale(lua_State* l)
{
	int res = pwrapSceneNodegetLocalScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::setLocalTransform.
static inline int pwrapSceneNodesetLocalTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTransform;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return -1;
	}

	Transform* iarg0 = ud->getData<Transform>();
	const Transform& arg0(*iarg0);

	// Call the method
	self->setLocalTransform(arg0);

	return 0;
}

/// Wrap method SceneNode::setLocalTransform.
static int wrapSceneNodesetLocalTransform(lua_State* l)
{
	int res = pwrapSceneNodesetLocalTransform(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getLocalTransform.
static inline int pwrapSceneNodegetLocalTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	const Transform& ret = self->getLocalTransform();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Transform");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTransform;
	ud->initPointed(&luaUserDataTypeInfoTransform, &ret);

	return 1;
}

/// Wrap method SceneNode::getLocalTransform.
static int wrapSceneNodegetLocalTransform(lua_State* l)
{
	int res = pwrapSceneNodegetLocalTransform(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<LightComponent>.
static inline int pwrapSceneNodenewLightComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LightComponent* ret = self->newComponent<LightComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LightComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoLightComponent;
	ud->initPointed(&luaUserDataTypeInfoLightComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<LightComponent>.
static int wrapSceneNodenewLightComponent(lua_State* l)
{
	int res = pwrapSceneNodenewLightComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<LensFlareComponent>.
static inline int pwrapSceneNodenewLensFlareComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LensFlareComponent* ret = self->newComponent<LensFlareComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LensFlareComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoLensFlareComponent;
	ud->initPointed(&luaUserDataTypeInfoLensFlareComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<LensFlareComponent>.
static int wrapSceneNodenewLensFlareComponent(lua_State* l)
{
	int res = pwrapSceneNodenewLensFlareComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<DecalComponent>.
static inline int pwrapSceneNodenewDecalComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	DecalComponent* ret = self->newComponent<DecalComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&luaUserDataTypeInfoDecalComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<DecalComponent>.
static int wrapSceneNodenewDecalComponent(lua_State* l)
{
	int res = pwrapSceneNodenewDecalComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<TriggerComponent>.
static inline int pwrapSceneNodenewTriggerComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	TriggerComponent* ret = self->newComponent<TriggerComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "TriggerComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTriggerComponent;
	ud->initPointed(&luaUserDataTypeInfoTriggerComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<TriggerComponent>.
static int wrapSceneNodenewTriggerComponent(lua_State* l)
{
	int res = pwrapSceneNodenewTriggerComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<FogDensityComponent>.
static inline int pwrapSceneNodenewFogDensityComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	FogDensityComponent* ret = self->newComponent<FogDensityComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "FogDensityComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityComponent;
	ud->initPointed(&luaUserDataTypeInfoFogDensityComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<FogDensityComponent>.
static int wrapSceneNodenewFogDensityComponent(lua_State* l)
{
	int res = pwrapSceneNodenewFogDensityComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<CameraComponent>.
static inline int pwrapSceneNodenewCameraComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	CameraComponent* ret = self->newComponent<CameraComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "CameraComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoCameraComponent;
	ud->initPointed(&luaUserDataTypeInfoCameraComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<CameraComponent>.
static int wrapSceneNodenewCameraComponent(lua_State* l)
{
	int res = pwrapSceneNodenewCameraComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<GlobalIlluminationProbeComponent>.
static inline int pwrapSceneNodenewGlobalIlluminationProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	GlobalIlluminationProbeComponent* ret = self->newComponent<GlobalIlluminationProbeComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GlobalIlluminationProbeComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeComponent;
	ud->initPointed(&luaUserDataTypeInfoGlobalIlluminationProbeComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<GlobalIlluminationProbeComponent>.
static int wrapSceneNodenewGlobalIlluminationProbeComponent(lua_State* l)
{
	int res = pwrapSceneNodenewGlobalIlluminationProbeComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<ReflectionProbeComponent>.
static inline int pwrapSceneNodenewReflectionProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ReflectionProbeComponent* ret = self->newComponent<ReflectionProbeComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbeComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeComponent;
	ud->initPointed(&luaUserDataTypeInfoReflectionProbeComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<ReflectionProbeComponent>.
static int wrapSceneNodenewReflectionProbeComponent(lua_State* l)
{
	int res = pwrapSceneNodenewReflectionProbeComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<BodyComponent>.
static inline int pwrapSceneNodenewBodyComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	BodyComponent* ret = self->newComponent<BodyComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "BodyComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoBodyComponent;
	ud->initPointed(&luaUserDataTypeInfoBodyComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<BodyComponent>.
static int wrapSceneNodenewBodyComponent(lua_State* l)
{
	int res = pwrapSceneNodenewBodyComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<ParticleEmitterComponent>.
static inline int pwrapSceneNodenewParticleEmitterComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ParticleEmitterComponent* ret = self->newComponent<ParticleEmitterComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitterComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterComponent;
	ud->initPointed(&luaUserDataTypeInfoParticleEmitterComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<ParticleEmitterComponent>.
static int wrapSceneNodenewParticleEmitterComponent(lua_State* l)
{
	int res = pwrapSceneNodenewParticleEmitterComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<MeshComponent>.
static inline int pwrapSceneNodenewMeshComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MeshComponent* ret = self->newComponent<MeshComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MeshComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMeshComponent;
	ud->initPointed(&luaUserDataTypeInfoMeshComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<MeshComponent>.
static int wrapSceneNodenewMeshComponent(lua_State* l)
{
	int res = pwrapSceneNodenewMeshComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<MaterialComponent>.
static inline int pwrapSceneNodenewMaterialComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MaterialComponent* ret = self->newComponent<MaterialComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&luaUserDataTypeInfoMaterialComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<MaterialComponent>.
static int wrapSceneNodenewMaterialComponent(lua_State* l)
{
	int res = pwrapSceneNodenewMaterialComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<SkinComponent>.
static inline int pwrapSceneNodenewSkinComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkinComponent* ret = self->newComponent<SkinComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkinComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSkinComponent;
	ud->initPointed(&luaUserDataTypeInfoSkinComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<SkinComponent>.
static int wrapSceneNodenewSkinComponent(lua_State* l)
{
	int res = pwrapSceneNodenewSkinComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::newComponent<SkyboxComponent>.
static inline int pwrapSceneNodenewSkyboxComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkyboxComponent* ret = self->newComponent<SkyboxComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkyboxComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSkyboxComponent;
	ud->initPointed(&luaUserDataTypeInfoSkyboxComponent, ret);

	return 1;
}

/// Wrap method SceneNode::newComponent<SkyboxComponent>.
static int wrapSceneNodenewSkyboxComponent(lua_State* l)
{
	int res = pwrapSceneNodenewSkyboxComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<LightComponent>.
static inline int pwrapSceneNodegetFirstLightComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LightComponent& ret = self->getFirstComponentOfType<LightComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LightComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoLightComponent;
	ud->initPointed(&luaUserDataTypeInfoLightComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<LightComponent>.
static int wrapSceneNodegetFirstLightComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstLightComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<LensFlareComponent>.
static inline int pwrapSceneNodegetFirstLensFlareComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LensFlareComponent& ret = self->getFirstComponentOfType<LensFlareComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LensFlareComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoLensFlareComponent;
	ud->initPointed(&luaUserDataTypeInfoLensFlareComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<LensFlareComponent>.
static int wrapSceneNodegetFirstLensFlareComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstLensFlareComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<DecalComponent>.
static inline int pwrapSceneNodegetFirstDecalComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	DecalComponent& ret = self->getFirstComponentOfType<DecalComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&luaUserDataTypeInfoDecalComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<DecalComponent>.
static int wrapSceneNodegetFirstDecalComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstDecalComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<TriggerComponent>.
static inline int pwrapSceneNodegetFirstTriggerComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	TriggerComponent& ret = self->getFirstComponentOfType<TriggerComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "TriggerComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTriggerComponent;
	ud->initPointed(&luaUserDataTypeInfoTriggerComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<TriggerComponent>.
static int wrapSceneNodegetFirstTriggerComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstTriggerComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<FogDensityComponent>.
static inline int pwrapSceneNodegetFirstFogDensityComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	FogDensityComponent& ret = self->getFirstComponentOfType<FogDensityComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "FogDensityComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityComponent;
	ud->initPointed(&luaUserDataTypeInfoFogDensityComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<FogDensityComponent>.
static int wrapSceneNodegetFirstFogDensityComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstFogDensityComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<CameraComponent>.
static inline int pwrapSceneNodegetFirstCameraComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	CameraComponent& ret = self->getFirstComponentOfType<CameraComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "CameraComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoCameraComponent;
	ud->initPointed(&luaUserDataTypeInfoCameraComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<CameraComponent>.
static int wrapSceneNodegetFirstCameraComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstCameraComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<GlobalIlluminationProbeComponent>.
static inline int pwrapSceneNodegetFirstGlobalIlluminationProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	GlobalIlluminationProbeComponent& ret = self->getFirstComponentOfType<GlobalIlluminationProbeComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GlobalIlluminationProbeComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeComponent;
	ud->initPointed(&luaUserDataTypeInfoGlobalIlluminationProbeComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<GlobalIlluminationProbeComponent>.
static int wrapSceneNodegetFirstGlobalIlluminationProbeComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstGlobalIlluminationProbeComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<ReflectionProbeComponent>.
static inline int pwrapSceneNodegetFirstReflectionProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ReflectionProbeComponent& ret = self->getFirstComponentOfType<ReflectionProbeComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbeComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeComponent;
	ud->initPointed(&luaUserDataTypeInfoReflectionProbeComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<ReflectionProbeComponent>.
static int wrapSceneNodegetFirstReflectionProbeComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstReflectionProbeComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<BodyComponent>.
static inline int pwrapSceneNodegetFirstBodyComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	BodyComponent& ret = self->getFirstComponentOfType<BodyComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "BodyComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoBodyComponent;
	ud->initPointed(&luaUserDataTypeInfoBodyComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<BodyComponent>.
static int wrapSceneNodegetFirstBodyComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstBodyComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<ParticleEmitterComponent>.
static inline int pwrapSceneNodegetFirstParticleEmitterComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ParticleEmitterComponent& ret = self->getFirstComponentOfType<ParticleEmitterComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitterComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterComponent;
	ud->initPointed(&luaUserDataTypeInfoParticleEmitterComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<ParticleEmitterComponent>.
static int wrapSceneNodegetFirstParticleEmitterComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstParticleEmitterComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<MeshComponent>.
static inline int pwrapSceneNodegetFirstMeshComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MeshComponent& ret = self->getFirstComponentOfType<MeshComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MeshComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMeshComponent;
	ud->initPointed(&luaUserDataTypeInfoMeshComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<MeshComponent>.
static int wrapSceneNodegetFirstMeshComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstMeshComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<MaterialComponent>.
static inline int pwrapSceneNodegetFirstMaterialComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MaterialComponent& ret = self->getFirstComponentOfType<MaterialComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&luaUserDataTypeInfoMaterialComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<MaterialComponent>.
static int wrapSceneNodegetFirstMaterialComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstMaterialComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<SkinComponent>.
static inline int pwrapSceneNodegetFirstSkinComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkinComponent& ret = self->getFirstComponentOfType<SkinComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkinComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSkinComponent;
	ud->initPointed(&luaUserDataTypeInfoSkinComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<SkinComponent>.
static int wrapSceneNodegetFirstSkinComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstSkinComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::getFirstComponentOfType<SkyboxComponent>.
static inline int pwrapSceneNodegetFirstSkyboxComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkyboxComponent& ret = self->getFirstComponentOfType<SkyboxComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkyboxComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSkyboxComponent;
	ud->initPointed(&luaUserDataTypeInfoSkyboxComponent, &ret);

	return 1;
}

/// Wrap method SceneNode::getFirstComponentOfType<SkyboxComponent>.
static int wrapSceneNodegetFirstSkyboxComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFirstSkyboxComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SceneNode.
static inline void wrapSceneNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSceneNode);
	LuaBinder::pushLuaCFuncMethod(l, "getName", wrapSceneNodegetName);
	LuaBinder::pushLuaCFuncMethod(l, "addChild", wrapSceneNodeaddChild);
	LuaBinder::pushLuaCFuncMethod(l, "markForDeletion", wrapSceneNodemarkForDeletion);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalOrigin", wrapSceneNodesetLocalOrigin);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalOrigin", wrapSceneNodegetLocalOrigin);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalRotation", wrapSceneNodesetLocalRotation);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalRotation", wrapSceneNodegetLocalRotation);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalScale", wrapSceneNodesetLocalScale);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalScale", wrapSceneNodegetLocalScale);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalTransform", wrapSceneNodesetLocalTransform);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalTransform", wrapSceneNodegetLocalTransform);
	LuaBinder::pushLuaCFuncMethod(l, "newLightComponent", wrapSceneNodenewLightComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newLensFlareComponent", wrapSceneNodenewLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newDecalComponent", wrapSceneNodenewDecalComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newTriggerComponent", wrapSceneNodenewTriggerComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newFogDensityComponent", wrapSceneNodenewFogDensityComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newCameraComponent", wrapSceneNodenewCameraComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newGlobalIlluminationProbeComponent", wrapSceneNodenewGlobalIlluminationProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProbeComponent", wrapSceneNodenewReflectionProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newBodyComponent", wrapSceneNodenewBodyComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitterComponent", wrapSceneNodenewParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newMeshComponent", wrapSceneNodenewMeshComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newMaterialComponent", wrapSceneNodenewMaterialComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newSkinComponent", wrapSceneNodenewSkinComponent);
	LuaBinder::pushLuaCFuncMethod(l, "newSkyboxComponent", wrapSceneNodenewSkyboxComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstLightComponent", wrapSceneNodegetFirstLightComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstLensFlareComponent", wrapSceneNodegetFirstLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstDecalComponent", wrapSceneNodegetFirstDecalComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstTriggerComponent", wrapSceneNodegetFirstTriggerComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstFogDensityComponent", wrapSceneNodegetFirstFogDensityComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstCameraComponent", wrapSceneNodegetFirstCameraComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstGlobalIlluminationProbeComponent", wrapSceneNodegetFirstGlobalIlluminationProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstReflectionProbeComponent", wrapSceneNodegetFirstReflectionProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstBodyComponent", wrapSceneNodegetFirstBodyComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstParticleEmitterComponent", wrapSceneNodegetFirstParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstMeshComponent", wrapSceneNodegetFirstMeshComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstMaterialComponent", wrapSceneNodegetFirstMaterialComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstSkinComponent", wrapSceneNodegetFirstSkinComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFirstSkyboxComponent", wrapSceneNodegetFirstSkyboxComponent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSceneGraph = {-4504117787585012859, "SceneGraph", LuaUserData::computeSizeForGarbageCollected<SceneGraph>(),
													 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneGraph>()
{
	return luaUserDataTypeInfoSceneGraph;
}

/// Pre-wrap method SceneGraph::newSceneNode.
static inline int pwrapSceneGraphnewSceneNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneGraph, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	SceneNode* ret = newSceneNode<SceneNode>(self, arg0);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newSceneNode.
static int wrapSceneGraphnewSceneNode(lua_State* l)
{
	int res = pwrapSceneGraphnewSceneNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::setActiveCameraNode.
static inline int pwrapSceneGraphsetActiveCameraNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneGraph, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return -1;
	}

	SceneNode* iarg0 = ud->getData<SceneNode>();
	SceneNode* arg0(iarg0);

	// Call the method
	self->setActiveCameraNode(arg0);

	return 0;
}

/// Wrap method SceneGraph::setActiveCameraNode.
static int wrapSceneGraphsetActiveCameraNode(lua_State* l)
{
	int res = pwrapSceneGraphsetActiveCameraNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::tryFindSceneNode.
static inline int pwrapSceneGraphtryFindSceneNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneGraph, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	SceneNode* ret = self->tryFindSceneNode(arg0);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, ret);

	return 1;
}

/// Wrap method SceneGraph::tryFindSceneNode.
static int wrapSceneGraphtryFindSceneNode(lua_State* l)
{
	int res = pwrapSceneGraphtryFindSceneNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SceneGraph.
static inline void wrapSceneGraph(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSceneGraph);
	LuaBinder::pushLuaCFuncMethod(l, "newSceneNode", wrapSceneGraphnewSceneNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCameraNode", wrapSceneGraphsetActiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "tryFindSceneNode", wrapSceneGraphtryFindSceneNode);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoEvent = {-8037780990830956566, "Event", LuaUserData::computeSizeForGarbageCollected<Event>(), nullptr,
												nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Event>()
{
	return luaUserDataTypeInfoEvent;
}

/// Pre-wrap method Event::getAssociatedSceneNodes.
static inline int pwrapEventgetAssociatedSceneNodes(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 1)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoEvent, ud))
	{
		return -1;
	}

	Event* self = ud->getData<Event>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getAssociatedSceneNodes();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArraySceneNodePtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr;
	ud->initGarbageCollected(&luaUserDataTypeInfoWeakArraySceneNodePtr);
	::new(ud->getData<WeakArraySceneNodePtr>()) WeakArraySceneNodePtr(std::move(ret));

	return 1;
}

/// Wrap method Event::getAssociatedSceneNodes.
static int wrapEventgetAssociatedSceneNodes(lua_State* l)
{
	int res = pwrapEventgetAssociatedSceneNodes(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Event.
static inline void wrapEvent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoEvent);
	LuaBinder::pushLuaCFuncMethod(l, "getAssociatedSceneNodes", wrapEventgetAssociatedSceneNodes);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoLightEvent = {-887179890638014087, "LightEvent", LuaUserData::computeSizeForGarbageCollected<LightEvent>(),
													 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightEvent>()
{
	return luaUserDataTypeInfoLightEvent;
}

/// Pre-wrap method LightEvent::setIntensityMultiplier.
static inline int pwrapLightEventsetIntensityMultiplier(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 2)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightEvent, ud))
	{
		return -1;
	}

	LightEvent* self = ud->getData<LightEvent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)) [[unlikely]]
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
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 3)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightEvent, ud))
	{
		return -1;
	}

	LightEvent* self = ud->getData<LightEvent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
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
	LuaBinder::createClass(l, &luaUserDataTypeInfoLightEvent);
	LuaBinder::pushLuaCFuncMethod(l, "setIntensityMultiplier", wrapLightEventsetIntensityMultiplier);
	LuaBinder::pushLuaCFuncMethod(l, "setFrequency", wrapLightEventsetFrequency);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoScriptEvent = {-3599694744897696875, "ScriptEvent", LuaUserData::computeSizeForGarbageCollected<ScriptEvent>(),
													  nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ScriptEvent>()
{
	return luaUserDataTypeInfoScriptEvent;
}

/// Wrap class ScriptEvent.
static inline void wrapScriptEvent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoScriptEvent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoJitterMoveEvent = {216767496191450146, "JitterMoveEvent",
														  LuaUserData::computeSizeForGarbageCollected<JitterMoveEvent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<JitterMoveEvent>()
{
	return luaUserDataTypeInfoJitterMoveEvent;
}

/// Pre-wrap method JitterMoveEvent::setPositionLimits.
static inline int pwrapJitterMoveEventsetPositionLimits(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 3)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoJitterMoveEvent, ud))
	{
		return -1;
	}

	JitterMoveEvent* self = ud->getData<JitterMoveEvent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, 3, luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return -1;
	}

	Vec3* iarg1 = ud->getData<Vec3>();
	Vec3 arg1(*iarg1);

	// Call the method
	self->setPositionLimits(arg0, arg1);

	return 0;
}

/// Wrap method JitterMoveEvent::setPositionLimits.
static int wrapJitterMoveEventsetPositionLimits(lua_State* l)
{
	int res = pwrapJitterMoveEventsetPositionLimits(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class JitterMoveEvent.
static inline void wrapJitterMoveEvent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoJitterMoveEvent);
	LuaBinder::pushLuaCFuncMethod(l, "setPositionLimits", wrapJitterMoveEventsetPositionLimits);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoAnimationEvent = {7826530453645847513, "AnimationEvent",
														 LuaUserData::computeSizeForGarbageCollected<AnimationEvent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<AnimationEvent>()
{
	return luaUserDataTypeInfoAnimationEvent;
}

/// Wrap class AnimationEvent.
static inline void wrapAnimationEvent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoAnimationEvent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoEventManager = {8340995330108727532, "EventManager",
													   LuaUserData::computeSizeForGarbageCollected<EventManager>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<EventManager>()
{
	return luaUserDataTypeInfoEventManager;
}

/// Pre-wrap method EventManager::newLightEvent.
static inline int pwrapEventManagernewLightEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 4)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoEventManager, ud))
	{
		return -1;
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	LightEvent* ret = newEvent<LightEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LightEvent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoLightEvent;
	ud->initPointed(&luaUserDataTypeInfoLightEvent, ret);

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

/// Pre-wrap method EventManager::newScriptEvent.
static inline int pwrapEventManagernewScriptEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 4)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoEventManager, ud))
	{
		return -1;
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	const char* arg2;
	if(LuaBinder::checkString(l, 4, arg2)) [[unlikely]]
	{
		return -1;
	}

	// Call the method
	ScriptEvent* ret = newEvent<ScriptEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ScriptEvent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoScriptEvent;
	ud->initPointed(&luaUserDataTypeInfoScriptEvent, ret);

	return 1;
}

/// Wrap method EventManager::newScriptEvent.
static int wrapEventManagernewScriptEvent(lua_State* l)
{
	int res = pwrapEventManagernewScriptEvent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method EventManager::newJitterMoveEvent.
static inline int pwrapEventManagernewJitterMoveEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 4)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoEventManager, ud))
	{
		return -1;
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	JitterMoveEvent* ret = newEvent<JitterMoveEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "JitterMoveEvent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoJitterMoveEvent;
	ud->initPointed(&luaUserDataTypeInfoJitterMoveEvent, ret);

	return 1;
}

/// Wrap method EventManager::newJitterMoveEvent.
static int wrapEventManagernewJitterMoveEvent(lua_State* l)
{
	int res = pwrapEventManagernewJitterMoveEvent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method EventManager::newAnimationEvent.
static inline int pwrapEventManagernewAnimationEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 4)) [[unlikely]]
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoEventManager, ud))
	{
		return -1;
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0)) [[unlikely]]
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1)) [[unlikely]]
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	AnimationEvent* ret = newEvent<AnimationEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "AnimationEvent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoAnimationEvent;
	ud->initPointed(&luaUserDataTypeInfoAnimationEvent, ret);

	return 1;
}

/// Wrap method EventManager::newAnimationEvent.
static int wrapEventManagernewAnimationEvent(lua_State* l)
{
	int res = pwrapEventManagernewAnimationEvent(l);
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
	LuaBinder::createClass(l, &luaUserDataTypeInfoEventManager);
	LuaBinder::pushLuaCFuncMethod(l, "newLightEvent", wrapEventManagernewLightEvent);
	LuaBinder::pushLuaCFuncMethod(l, "newScriptEvent", wrapEventManagernewScriptEvent);
	LuaBinder::pushLuaCFuncMethod(l, "newJitterMoveEvent", wrapEventManagernewJitterMoveEvent);
	LuaBinder::pushLuaCFuncMethod(l, "newAnimationEvent", wrapEventManagernewAnimationEvent);
	lua_settop(l, 0);
}

/// Pre-wrap function getSceneGraph.
static inline int pwrapgetSceneGraph(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 0)) [[unlikely]]
	{
		return -1;
	}

	// Call the function
	SceneGraph* ret = getSceneGraph(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneGraph");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneGraph;
	ud->initPointed(&luaUserDataTypeInfoSceneGraph, ret);

	return 1;
}

/// Wrap function getSceneGraph.
static int wrapgetSceneGraph(lua_State* l)
{
	int res = pwrapgetSceneGraph(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap function getEventManager.
static inline int pwrapgetEventManager(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, 0)) [[unlikely]]
	{
		return -1;
	}

	// Call the function
	EventManager* ret = getEventManager(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "EventManager");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoEventManager;
	ud->initPointed(&luaUserDataTypeInfoEventManager, ret);

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
void wrapModuleScene(lua_State* l)
{
	wrapWeakArraySceneNodePtr(l);
	wrapLightComponent(l);
	wrapDecalComponent(l);
	wrapLensFlareComponent(l);
	wrapBodyComponent(l);
	wrapTriggerComponent(l);
	wrapFogDensityComponent(l);
	wrapCameraComponent(l);
	wrapGlobalIlluminationProbeComponent(l);
	wrapReflectionProbeComponent(l);
	wrapParticleEmitterComponent(l);
	wrapMeshComponent(l);
	wrapMaterialComponent(l);
	wrapSkinComponent(l);
	wrapSkyboxComponent(l);
	wrapSceneNode(l);
	wrapSceneGraph(l);
	wrapEvent(l);
	wrapLightEvent(l);
	wrapScriptEvent(l);
	wrapJitterMoveEvent(l);
	wrapAnimationEvent(l);
	wrapEventManager(l);
	LuaBinder::pushLuaCFunc(l, "getSceneGraph", wrapgetSceneGraph);
	LuaBinder::pushLuaCFunc(l, "getEventManager", wrapgetEventManager);
	wrapLightComponentType(l);
	wrapBodyComponentCollisionShapeType(l);
}

} // end namespace anki
