// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	T* ptr;
	Error err = scene->template newSceneNode<T>(name, ptr, std::forward<TArgs>(args)...);

	if(!err)
	{
		return ptr;
	}
	else
	{
		return nullptr;
	}
}

template<typename T, typename... TArgs>
static T* newEvent(EventManager* eventManager, TArgs... args)
{
	T* ptr;
	Error err = eventManager->template newEvent<T>(ptr, std::forward<TArgs>(args)...);

	if(!err)
	{
		return ptr;
	}
	else
	{
		return nullptr;
	}
}

static SceneGraph* getSceneGraph(lua_State* l)
{
	LuaBinder* binder = nullptr;
	lua_getallocf(l, reinterpret_cast<void**>(&binder));

	SceneGraph* scene = binder->getOtherSystems().m_sceneGraph;
	ANKI_ASSERT(scene);
	return scene;
}

static EventManager* getEventManager(lua_State* l)
{
	return &getSceneGraph(l)->getEventManager();
}

using WeakArraySceneNodePtr = WeakArray<SceneNode*>;
using WeakArrayBodyComponentPtr = WeakArray<BodyComponent*>;

LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr = {
	-8739932090814685424, "WeakArraySceneNodePtr", LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>(),
	nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	SceneNode* ret = (*self)[arg0];

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

LuaUserDataTypeInfo luaUserDataTypeInfoWeakArrayBodyComponentPtr = {
	-7706209051073311304, "WeakArrayBodyComponentPtr",
	LuaUserData::computeSizeForGarbageCollected<WeakArrayBodyComponentPtr>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<WeakArrayBodyComponentPtr>()
{
	return luaUserDataTypeInfoWeakArrayBodyComponentPtr;
}

/// Pre-wrap method WeakArrayBodyComponentPtr::getSize.
static inline int pwrapWeakArrayBodyComponentPtrgetSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoWeakArrayBodyComponentPtr, ud))
	{
		return -1;
	}

	WeakArrayBodyComponentPtr* self = ud->getData<WeakArrayBodyComponentPtr>();

	// Call the method
	U32 ret = self->getSize();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method WeakArrayBodyComponentPtr::getSize.
static int wrapWeakArrayBodyComponentPtrgetSize(lua_State* l)
{
	int res = pwrapWeakArrayBodyComponentPtrgetSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method WeakArrayBodyComponentPtr::getAt.
static inline int pwrapWeakArrayBodyComponentPtrgetAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoWeakArrayBodyComponentPtr, ud))
	{
		return -1;
	}

	WeakArrayBodyComponentPtr* self = ud->getData<WeakArrayBodyComponentPtr>();

	// Pop arguments
	U32 arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	BodyComponent* ret = (*self)[arg0];

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method WeakArrayBodyComponentPtr::getAt.
static int wrapWeakArrayBodyComponentPtrgetAt(lua_State* l)
{
	int res = pwrapWeakArrayBodyComponentPtrgetAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class WeakArrayBodyComponentPtr.
static inline void wrapWeakArrayBodyComponentPtr(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoWeakArrayBodyComponentPtr);
	LuaBinder::pushLuaCFuncMethod(l, "getSize", wrapWeakArrayBodyComponentPtrgetSize);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapWeakArrayBodyComponentPtrgetAt);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoMoveComponent = {-8687627984466261764, "MoveComponent",
														LuaUserData::computeSizeForGarbageCollected<MoveComponent>(),
														nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MoveComponent>()
{
	return luaUserDataTypeInfoMoveComponent;
}

/// Pre-wrap method MoveComponent::setLocalOrigin.
static inline int pwrapMoveComponentsetLocalOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setLocalOrigin(arg0);

	return 0;
}

/// Wrap method MoveComponent::setLocalOrigin.
static int wrapMoveComponentsetLocalOrigin(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalOrigin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::getLocalOrigin.
static inline int pwrapMoveComponentgetLocalOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Call the method
	const Vec4& ret = self->getLocalOrigin();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	ud->initPointed(&luaUserDataTypeInfoVec4, &ret);

	return 1;
}

/// Wrap method MoveComponent::getLocalOrigin.
static int wrapMoveComponentgetLocalOrigin(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalOrigin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::setLocalRotation.
static inline int pwrapMoveComponentsetLocalRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMat3x4;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoMat3x4, ud)))
	{
		return -1;
	}

	Mat3x4* iarg0 = ud->getData<Mat3x4>();
	const Mat3x4& arg0(*iarg0);

	// Call the method
	self->setLocalRotation(arg0);

	return 0;
}

/// Wrap method MoveComponent::setLocalRotation.
static int wrapMoveComponentsetLocalRotation(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalRotation(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::getLocalRotation.
static inline int pwrapMoveComponentgetLocalRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Call the method
	const Mat3x4& ret = self->getLocalRotation();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Mat3x4");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMat3x4;
	ud->initPointed(&luaUserDataTypeInfoMat3x4, &ret);

	return 1;
}

/// Wrap method MoveComponent::getLocalRotation.
static int wrapMoveComponentgetLocalRotation(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalRotation(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::setLocalScale.
static inline int pwrapMoveComponentsetLocalScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	F32 arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	self->setLocalScale(arg0);

	return 0;
}

/// Wrap method MoveComponent::setLocalScale.
static int wrapMoveComponentsetLocalScale(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::getLocalScale.
static inline int pwrapMoveComponentgetLocalScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Call the method
	F32 ret = self->getLocalScale();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

/// Wrap method MoveComponent::getLocalScale.
static int wrapMoveComponentgetLocalScale(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::setLocalTransform.
static inline int pwrapMoveComponentsetLocalTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTransform;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoTransform, ud)))
	{
		return -1;
	}

	Transform* iarg0 = ud->getData<Transform>();
	const Transform& arg0(*iarg0);

	// Call the method
	self->setLocalTransform(arg0);

	return 0;
}

/// Wrap method MoveComponent::setLocalTransform.
static int wrapMoveComponentsetLocalTransform(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalTransform(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method MoveComponent::getLocalTransform.
static inline int pwrapMoveComponentgetLocalTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

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

/// Wrap method MoveComponent::getLocalTransform.
static int wrapMoveComponentgetLocalTransform(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalTransform(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class MoveComponent.
static inline void wrapMoveComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoMoveComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalOrigin", wrapMoveComponentsetLocalOrigin);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalOrigin", wrapMoveComponentgetLocalOrigin);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalRotation", wrapMoveComponentsetLocalRotation);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalRotation", wrapMoveComponentgetLocalRotation);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalScale", wrapMoveComponentsetLocalScale);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalScale", wrapMoveComponentgetLocalScale);
	LuaBinder::pushLuaCFuncMethod(l, "setLocalTransform", wrapMoveComponentsetLocalTransform);
	LuaBinder::pushLuaCFuncMethod(l, "getLocalTransform", wrapMoveComponentgetLocalTransform);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoLightComponent = {7365641557642621447, "LightComponent",
														 LuaUserData::computeSizeForGarbageCollected<LightComponent>(),
														 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightComponent>()
{
	return luaUserDataTypeInfoLightComponent;
}

/// Pre-wrap method LightComponent::setDiffuseColor.
static inline int pwrapLightComponentsetDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

LuaUserDataTypeInfo luaUserDataTypeInfoDecalComponent = {5204471888737717555, "DecalComponent",
														 LuaUserData::computeSizeForGarbageCollected<DecalComponent>(),
														 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DecalComponent>()
{
	return luaUserDataTypeInfoDecalComponent;
}

/// Pre-wrap method DecalComponent::setDiffuseDecal.
static inline int pwrapDecalComponentsetDiffuseDecal(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	F32 arg2;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 4, arg2)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->setDiffuseDecal(arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
}

/// Wrap method DecalComponent::setDiffuseDecal.
static int wrapDecalComponentsetDiffuseDecal(lua_State* l)
{
	int res = pwrapDecalComponentsetDiffuseDecal(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method DecalComponent::setSpecularRoughnessDecal.
static inline int pwrapDecalComponentsetSpecularRoughnessDecal(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	F32 arg2;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 4, arg2)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->setSpecularRoughnessDecal(arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
}

/// Wrap method DecalComponent::setSpecularRoughnessDecal.
static int wrapDecalComponentsetSpecularRoughnessDecal(lua_State* l)
{
	int res = pwrapDecalComponentsetSpecularRoughnessDecal(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method DecalComponent::setBoxVolumeSize.
static inline int pwrapDecalComponentsetBoxVolumeSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setBoxVolumeSize(arg0);

	return 0;
}

/// Wrap method DecalComponent::setBoxVolumeSize.
static int wrapDecalComponentsetBoxVolumeSize(lua_State* l)
{
	int res = pwrapDecalComponentsetBoxVolumeSize(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "setDiffuseDecal", wrapDecalComponentsetDiffuseDecal);
	LuaBinder::pushLuaCFuncMethod(l, "setSpecularRoughnessDecal", wrapDecalComponentsetSpecularRoughnessDecal);
	LuaBinder::pushLuaCFuncMethod(l, "setBoxVolumeSize", wrapDecalComponentsetBoxVolumeSize);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoLensFlareComponent = {
	3129239218800905411, "LensFlareComponent", LuaUserData::computeSizeForGarbageCollected<LensFlareComponent>(),
	nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadImageResource(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec2, ud)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)))
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

LuaUserDataTypeInfo luaUserDataTypeInfoBodyComponent = {-2321292920370861788, "BodyComponent",
														LuaUserData::computeSizeForGarbageCollected<BodyComponent>(),
														nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<BodyComponent>()
{
	return luaUserDataTypeInfoBodyComponent;
}

/// Pre-wrap method BodyComponent::loadMeshResource.
static inline int pwrapBodyComponentloadMeshResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadMeshResource(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
}

/// Wrap method BodyComponent::loadMeshResource.
static int wrapBodyComponentloadMeshResource(lua_State* l)
{
	int res = pwrapBodyComponentloadMeshResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::setWorldTransform.
static inline int pwrapBodyComponentsetWorldTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTransform;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoTransform, ud)))
	{
		return -1;
	}

	Transform* iarg0 = ud->getData<Transform>();
	const Transform& arg0(*iarg0);

	// Call the method
	self->setWorldTransform(arg0);

	return 0;
}

/// Wrap method BodyComponent::setWorldTransform.
static int wrapBodyComponentsetWorldTransform(lua_State* l)
{
	int res = pwrapBodyComponentsetWorldTransform(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method BodyComponent::getWorldTransform.
static inline int pwrapBodyComponentgetWorldTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	Transform ret = self->getWorldTransform();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Transform>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Transform");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTransform;
	ud->initGarbageCollected(&luaUserDataTypeInfoTransform);
	::new(ud->getData<Transform>()) Transform(std::move(ret));

	return 1;
}

/// Wrap method BodyComponent::getWorldTransform.
static int wrapBodyComponentgetWorldTransform(lua_State* l)
{
	int res = pwrapBodyComponentgetWorldTransform(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "loadMeshResource", wrapBodyComponentloadMeshResource);
	LuaBinder::pushLuaCFuncMethod(l, "setWorldTransform", wrapBodyComponentsetWorldTransform);
	LuaBinder::pushLuaCFuncMethod(l, "getWorldTransform", wrapBodyComponentgetWorldTransform);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoTriggerComponent = {
	8727234490176981006, "TriggerComponent", LuaUserData::computeSizeForGarbageCollected<TriggerComponent>(), nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<TriggerComponent>()
{
	return luaUserDataTypeInfoTriggerComponent;
}

/// Pre-wrap method TriggerComponent::getBodyComponentsEnter.
static inline int pwrapTriggerComponentgetBodyComponentsEnter(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	WeakArrayBodyComponentPtr ret = self->getBodyComponentsEnter();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArrayBodyComponentPtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArrayBodyComponentPtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoWeakArrayBodyComponentPtr;
	ud->initGarbageCollected(&luaUserDataTypeInfoWeakArrayBodyComponentPtr);
	::new(ud->getData<WeakArrayBodyComponentPtr>()) WeakArrayBodyComponentPtr(std::move(ret));

	return 1;
}

/// Wrap method TriggerComponent::getBodyComponentsEnter.
static int wrapTriggerComponentgetBodyComponentsEnter(lua_State* l)
{
	int res = pwrapTriggerComponentgetBodyComponentsEnter(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method TriggerComponent::getBodyComponentsInside.
static inline int pwrapTriggerComponentgetBodyComponentsInside(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	WeakArrayBodyComponentPtr ret = self->getBodyComponentsInside();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArrayBodyComponentPtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArrayBodyComponentPtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoWeakArrayBodyComponentPtr;
	ud->initGarbageCollected(&luaUserDataTypeInfoWeakArrayBodyComponentPtr);
	::new(ud->getData<WeakArrayBodyComponentPtr>()) WeakArrayBodyComponentPtr(std::move(ret));

	return 1;
}

/// Wrap method TriggerComponent::getBodyComponentsInside.
static int wrapTriggerComponentgetBodyComponentsInside(lua_State* l)
{
	int res = pwrapTriggerComponentgetBodyComponentsInside(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method TriggerComponent::getBodyComponentsExit.
static inline int pwrapTriggerComponentgetBodyComponentsExit(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	WeakArrayBodyComponentPtr ret = self->getBodyComponentsExit();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArrayBodyComponentPtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArrayBodyComponentPtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoWeakArrayBodyComponentPtr;
	ud->initGarbageCollected(&luaUserDataTypeInfoWeakArrayBodyComponentPtr);
	::new(ud->getData<WeakArrayBodyComponentPtr>()) WeakArrayBodyComponentPtr(std::move(ret));

	return 1;
}

/// Wrap method TriggerComponent::getBodyComponentsExit.
static int wrapTriggerComponentgetBodyComponentsExit(lua_State* l)
{
	int res = pwrapTriggerComponentgetBodyComponentsExit(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "getBodyComponentsEnter", wrapTriggerComponentgetBodyComponentsEnter);
	LuaBinder::pushLuaCFuncMethod(l, "getBodyComponentsInside", wrapTriggerComponentgetBodyComponentsInside);
	LuaBinder::pushLuaCFuncMethod(l, "getBodyComponentsExit", wrapTriggerComponentgetBodyComponentsExit);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityComponent = {
	-1966916321067375525, "FogDensityComponent", LuaUserData::computeSizeForGarbageCollected<FogDensityComponent>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FogDensityComponent>()
{
	return luaUserDataTypeInfoFogDensityComponent;
}

/// Pre-wrap method FogDensityComponent::setBoxVolumeSize.
static inline int pwrapFogDensityComponentsetBoxVolumeSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setBoxVolumeSize(arg0);

	return 0;
}

/// Wrap method FogDensityComponent::setBoxVolumeSize.
static int wrapFogDensityComponentsetBoxVolumeSize(lua_State* l)
{
	int res = pwrapFogDensityComponentsetBoxVolumeSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method FogDensityComponent::setSphereVolumeRadius.
static inline int pwrapFogDensityComponentsetSphereVolumeRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	self->setSphereVolumeRadius(arg0);

	return 0;
}

/// Wrap method FogDensityComponent::setSphereVolumeRadius.
static int wrapFogDensityComponentsetSphereVolumeRadius(lua_State* l)
{
	int res = pwrapFogDensityComponentsetSphereVolumeRadius(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method FogDensityComponent::setDensity.
static inline int pwrapFogDensityComponentsetDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	LuaBinder::pushLuaCFuncMethod(l, "setBoxVolumeSize", wrapFogDensityComponentsetBoxVolumeSize);
	LuaBinder::pushLuaCFuncMethod(l, "setSphereVolumeRadius", wrapFogDensityComponentsetSphereVolumeRadius);
	LuaBinder::pushLuaCFuncMethod(l, "setDensity", wrapFogDensityComponentsetDensity);
	LuaBinder::pushLuaCFuncMethod(l, "getDensity", wrapFogDensityComponentgetDensity);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoFrustumComponent = {
	-1191485642898408008, "FrustumComponent", LuaUserData::computeSizeForGarbageCollected<FrustumComponent>(), nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FrustumComponent>()
{
	return luaUserDataTypeInfoFrustumComponent;
}

/// Pre-wrap method FrustumComponent::setPerspective.
static inline int pwrapFrustumComponentsetPerspective(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 5)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFrustumComponent, ud))
	{
		return -1;
	}

	FrustumComponent* self = ud->getData<FrustumComponent>();

	// Pop arguments
	F32 arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	F32 arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 3, arg1)))
	{
		return -1;
	}

	F32 arg2;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 4, arg2)))
	{
		return -1;
	}

	F32 arg3;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 5, arg3)))
	{
		return -1;
	}

	// Call the method
	self->setPerspective(arg0, arg1, arg2, arg3);

	return 0;
}

/// Wrap method FrustumComponent::setPerspective.
static int wrapFrustumComponentsetPerspective(lua_State* l)
{
	int res = pwrapFrustumComponentsetPerspective(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method FrustumComponent::setShadowCascadesDistancePower.
static inline int pwrapFrustumComponentsetShadowCascadesDistancePower(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFrustumComponent, ud))
	{
		return -1;
	}

	FrustumComponent* self = ud->getData<FrustumComponent>();

	// Pop arguments
	F32 arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	self->setShadowCascadesDistancePower(arg0);

	return 0;
}

/// Wrap method FrustumComponent::setShadowCascadesDistancePower.
static int wrapFrustumComponentsetShadowCascadesDistancePower(lua_State* l)
{
	int res = pwrapFrustumComponentsetShadowCascadesDistancePower(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method FrustumComponent::setEffectiveShadowDistance.
static inline int pwrapFrustumComponentsetEffectiveShadowDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFrustumComponent, ud))
	{
		return -1;
	}

	FrustumComponent* self = ud->getData<FrustumComponent>();

	// Pop arguments
	F32 arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	self->setEffectiveShadowDistance(arg0);

	return 0;
}

/// Wrap method FrustumComponent::setEffectiveShadowDistance.
static int wrapFrustumComponentsetEffectiveShadowDistance(lua_State* l)
{
	int res = pwrapFrustumComponentsetEffectiveShadowDistance(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class FrustumComponent.
static inline void wrapFrustumComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoFrustumComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setPerspective", wrapFrustumComponentsetPerspective);
	LuaBinder::pushLuaCFuncMethod(l, "setShadowCascadesDistancePower",
								  wrapFrustumComponentsetShadowCascadesDistancePower);
	LuaBinder::pushLuaCFuncMethod(l, "setEffectiveShadowDistance", wrapFrustumComponentsetEffectiveShadowDistance);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeComponent = {
	-1018126859883492712, "GlobalIlluminationProbeComponent",
	LuaUserData::computeSizeForGarbageCollected<GlobalIlluminationProbeComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GlobalIlluminationProbeComponent>()
{
	return luaUserDataTypeInfoGlobalIlluminationProbeComponent;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::setBoxVolumeSize.
static inline int pwrapGlobalIlluminationProbeComponentsetBoxVolumeSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setBoxVolumeSize(arg0);

	return 0;
}

/// Wrap method GlobalIlluminationProbeComponent::setBoxVolumeSize.
static int wrapGlobalIlluminationProbeComponentsetBoxVolumeSize(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentsetBoxVolumeSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::setCellSize.
static inline int pwrapGlobalIlluminationProbeComponentsetCellSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	LuaBinder::pushLuaCFuncMethod(l, "setBoxVolumeSize", wrapGlobalIlluminationProbeComponentsetBoxVolumeSize);
	LuaBinder::pushLuaCFuncMethod(l, "setCellSize", wrapGlobalIlluminationProbeComponentsetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "getCellSize", wrapGlobalIlluminationProbeComponentgetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "setFadeDistance", wrapGlobalIlluminationProbeComponentsetFadeDistance);
	LuaBinder::pushLuaCFuncMethod(l, "getFadeDistance", wrapGlobalIlluminationProbeComponentgetFadeDistance);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeComponent = {
	-927979292880454957, "ReflectionProbeComponent",
	LuaUserData::computeSizeForGarbageCollected<ReflectionProbeComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ReflectionProbeComponent>()
{
	return luaUserDataTypeInfoReflectionProbeComponent;
}

/// Pre-wrap method ReflectionProbeComponent::setBoxVolumeSize.
static inline int pwrapReflectionProbeComponentsetBoxVolumeSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoReflectionProbeComponent, ud))
	{
		return -1;
	}

	ReflectionProbeComponent* self = ud->getData<ReflectionProbeComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec3;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setBoxVolumeSize(arg0);

	return 0;
}

/// Wrap method ReflectionProbeComponent::setBoxVolumeSize.
static int wrapReflectionProbeComponentsetBoxVolumeSize(lua_State* l)
{
	int res = pwrapReflectionProbeComponentsetBoxVolumeSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method ReflectionProbeComponent::getBoxVolumeSize.
static inline int pwrapReflectionProbeComponentgetBoxVolumeSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoReflectionProbeComponent, ud))
	{
		return -1;
	}

	ReflectionProbeComponent* self = ud->getData<ReflectionProbeComponent>();

	// Call the method
	Vec3 ret = self->getBoxVolumeSize();

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

/// Wrap method ReflectionProbeComponent::getBoxVolumeSize.
static int wrapReflectionProbeComponentgetBoxVolumeSize(lua_State* l)
{
	int res = pwrapReflectionProbeComponentgetBoxVolumeSize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ReflectionProbeComponent.
static inline void wrapReflectionProbeComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoReflectionProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setBoxVolumeSize", wrapReflectionProbeComponentsetBoxVolumeSize);
	LuaBinder::pushLuaCFuncMethod(l, "getBoxVolumeSize", wrapReflectionProbeComponentgetBoxVolumeSize);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterComponent = {
	-6511516637176772129, "ParticleEmitterComponent",
	LuaUserData::computeSizeForGarbageCollected<ParticleEmitterComponent>(), nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadParticleEmitterResource(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
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
	LuaBinder::pushLuaCFuncMethod(l, "loadParticleEmitterResource",
								  wrapParticleEmitterComponentloadParticleEmitterResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoGpuParticleEmitterComponent = {
	7059837890409742045, "GpuParticleEmitterComponent",
	LuaUserData::computeSizeForGarbageCollected<GpuParticleEmitterComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GpuParticleEmitterComponent>()
{
	return luaUserDataTypeInfoGpuParticleEmitterComponent;
}

/// Pre-wrap method GpuParticleEmitterComponent::loadParticleEmitterResource.
static inline int pwrapGpuParticleEmitterComponentloadParticleEmitterResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGpuParticleEmitterComponent, ud))
	{
		return -1;
	}

	GpuParticleEmitterComponent* self = ud->getData<GpuParticleEmitterComponent>();

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadParticleEmitterResource(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
}

/// Wrap method GpuParticleEmitterComponent::loadParticleEmitterResource.
static int wrapGpuParticleEmitterComponentloadParticleEmitterResource(lua_State* l)
{
	int res = pwrapGpuParticleEmitterComponentloadParticleEmitterResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class GpuParticleEmitterComponent.
static inline void wrapGpuParticleEmitterComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoGpuParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadParticleEmitterResource",
								  wrapGpuParticleEmitterComponentloadParticleEmitterResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoModelComponent = {-3624966836559375132, "ModelComponent",
														 LuaUserData::computeSizeForGarbageCollected<ModelComponent>(),
														 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ModelComponent>()
{
	return luaUserDataTypeInfoModelComponent;
}

/// Pre-wrap method ModelComponent::loadModelResource.
static inline int pwrapModelComponentloadModelResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoModelComponent, ud))
	{
		return -1;
	}

	ModelComponent* self = ud->getData<ModelComponent>();

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadModelResource(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
}

/// Wrap method ModelComponent::loadModelResource.
static int wrapModelComponentloadModelResource(lua_State* l)
{
	int res = pwrapModelComponentloadModelResource(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ModelComponent.
static inline void wrapModelComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoModelComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadModelResource", wrapModelComponentloadModelResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSkinComponent = {-8546506900929446763, "SkinComponent",
														LuaUserData::computeSizeForGarbageCollected<SkinComponent>(),
														nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadSkeletonResource(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, lua_Number(!!ret));

	return 1;
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

LuaUserDataTypeInfo luaUserDataTypeInfoSkyboxComponent = {
	-5875722332241774365, "SkyboxComponent", LuaUserData::computeSizeForGarbageCollected<SkyboxComponent>(), nullptr,
	nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec3, ud)))
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

/// Pre-wrap method SkyboxComponent::setImage.
static inline int pwrapSkyboxComponentsetImage(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	self->setImage(arg0);

	return 0;
}

/// Wrap method SkyboxComponent::setImage.
static int wrapSkyboxComponentsetImage(lua_State* l)
{
	int res = pwrapSkyboxComponentsetImage(l);
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
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

/// Wrap class SkyboxComponent.
static inline void wrapSkyboxComponent(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSkyboxComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setSolidColor", wrapSkyboxComponentsetSolidColor);
	LuaBinder::pushLuaCFuncMethod(l, "setImage", wrapSkyboxComponentsetImage);
	LuaBinder::pushLuaCFuncMethod(l, "setMinFogDensity", wrapSkyboxComponentsetMinFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setMaxFogDensity", wrapSkyboxComponentsetMaxFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setHeightOfMinFogDensity", wrapSkyboxComponentsetHeightOfMinFogDensity);
	LuaBinder::pushLuaCFuncMethod(l, "setHeightOfMaxFogDensity", wrapSkyboxComponentsetHeightOfMaxFogDensity);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode = {
	-5266826471794230711, "SceneNode", LuaUserData::computeSizeForGarbageCollected<SceneNode>(), nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoSceneNode, ud)))
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

/// Pre-wrap method SceneNode::setMarkedForDeletion.
static inline int pwrapSceneNodesetMarkedForDeletion(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	self->setMarkedForDeletion();

	return 0;
}

/// Wrap method SceneNode::setMarkedForDeletion.
static int wrapSceneNodesetMarkedForDeletion(lua_State* l)
{
	int res = pwrapSceneNodesetMarkedForDeletion(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<MoveComponent>.
static inline int pwrapSceneNodegetMoveComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	MoveComponent* ret = self->tryGetFirstComponentOfType<MoveComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MoveComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoMoveComponent;
	ud->initPointed(&luaUserDataTypeInfoMoveComponent, ret);

	return 1;
}

/// Wrap method SceneNode::tryGetFirstComponentOfType<MoveComponent>.
static int wrapSceneNodegetMoveComponent(lua_State* l)
{
	int res = pwrapSceneNodegetMoveComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<LightComponent>.
static inline int pwrapSceneNodegetLightComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	LightComponent* ret = self->tryGetFirstComponentOfType<LightComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<LightComponent>.
static int wrapSceneNodegetLightComponent(lua_State* l)
{
	int res = pwrapSceneNodegetLightComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<LensFlareComponent>.
static inline int pwrapSceneNodegetLensFlareComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	LensFlareComponent* ret = self->tryGetFirstComponentOfType<LensFlareComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<LensFlareComponent>.
static int wrapSceneNodegetLensFlareComponent(lua_State* l)
{
	int res = pwrapSceneNodegetLensFlareComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<DecalComponent>.
static inline int pwrapSceneNodegetDecalComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	DecalComponent* ret = self->tryGetFirstComponentOfType<DecalComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<DecalComponent>.
static int wrapSceneNodegetDecalComponent(lua_State* l)
{
	int res = pwrapSceneNodegetDecalComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<TriggerComponent>.
static inline int pwrapSceneNodegetTriggerComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	TriggerComponent* ret = self->tryGetFirstComponentOfType<TriggerComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<TriggerComponent>.
static int wrapSceneNodegetTriggerComponent(lua_State* l)
{
	int res = pwrapSceneNodegetTriggerComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<FogDensityComponent>.
static inline int pwrapSceneNodegetFogDensityComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	FogDensityComponent* ret = self->tryGetFirstComponentOfType<FogDensityComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<FogDensityComponent>.
static int wrapSceneNodegetFogDensityComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFogDensityComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<FrustumComponent>.
static inline int pwrapSceneNodegetFrustumComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	FrustumComponent* ret = self->tryGetFirstComponentOfType<FrustumComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "FrustumComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoFrustumComponent;
	ud->initPointed(&luaUserDataTypeInfoFrustumComponent, ret);

	return 1;
}

/// Wrap method SceneNode::tryGetFirstComponentOfType<FrustumComponent>.
static int wrapSceneNodegetFrustumComponent(lua_State* l)
{
	int res = pwrapSceneNodegetFrustumComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<GlobalIlluminationProbeComponent>.
static inline int pwrapSceneNodegetGlobalIlluminationProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	GlobalIlluminationProbeComponent* ret = self->tryGetFirstComponentOfType<GlobalIlluminationProbeComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<GlobalIlluminationProbeComponent>.
static int wrapSceneNodegetGlobalIlluminationProbeComponent(lua_State* l)
{
	int res = pwrapSceneNodegetGlobalIlluminationProbeComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<ReflectionProbeComponent>.
static inline int pwrapSceneNodegetReflectionProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	ReflectionProbeComponent* ret = self->tryGetFirstComponentOfType<ReflectionProbeComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<ReflectionProbeComponent>.
static int wrapSceneNodegetReflectionProbeComponent(lua_State* l)
{
	int res = pwrapSceneNodegetReflectionProbeComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<BodyComponent>.
static inline int pwrapSceneNodegetBodyComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	BodyComponent* ret = self->tryGetFirstComponentOfType<BodyComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<BodyComponent>.
static int wrapSceneNodegetBodyComponent(lua_State* l)
{
	int res = pwrapSceneNodegetBodyComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<ParticleEmitterComponent>.
static inline int pwrapSceneNodegetParticleEmitterComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	ParticleEmitterComponent* ret = self->tryGetFirstComponentOfType<ParticleEmitterComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<ParticleEmitterComponent>.
static int wrapSceneNodegetParticleEmitterComponent(lua_State* l)
{
	int res = pwrapSceneNodegetParticleEmitterComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<GpuParticleEmitterComponent>.
static inline int pwrapSceneNodegetGpuParticleEmitterComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	GpuParticleEmitterComponent* ret = self->tryGetFirstComponentOfType<GpuParticleEmitterComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GpuParticleEmitterComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoGpuParticleEmitterComponent;
	ud->initPointed(&luaUserDataTypeInfoGpuParticleEmitterComponent, ret);

	return 1;
}

/// Wrap method SceneNode::tryGetFirstComponentOfType<GpuParticleEmitterComponent>.
static int wrapSceneNodegetGpuParticleEmitterComponent(lua_State* l)
{
	int res = pwrapSceneNodegetGpuParticleEmitterComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<ModelComponent>.
static inline int pwrapSceneNodegetModelComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	ModelComponent* ret = self->tryGetFirstComponentOfType<ModelComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ModelComponent");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoModelComponent;
	ud->initPointed(&luaUserDataTypeInfoModelComponent, ret);

	return 1;
}

/// Wrap method SceneNode::tryGetFirstComponentOfType<ModelComponent>.
static int wrapSceneNodegetModelComponent(lua_State* l)
{
	int res = pwrapSceneNodegetModelComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<SkinComponent>.
static inline int pwrapSceneNodegetSkinComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	SkinComponent* ret = self->tryGetFirstComponentOfType<SkinComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<SkinComponent>.
static int wrapSceneNodegetSkinComponent(lua_State* l)
{
	int res = pwrapSceneNodegetSkinComponent(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<SkyboxComponent>.
static inline int pwrapSceneNodegetSkyboxComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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
	SkyboxComponent* ret = self->tryGetFirstComponentOfType<SkyboxComponent>();

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

/// Wrap method SceneNode::tryGetFirstComponentOfType<SkyboxComponent>.
static int wrapSceneNodegetSkyboxComponent(lua_State* l)
{
	int res = pwrapSceneNodegetSkyboxComponent(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "setMarkedForDeletion", wrapSceneNodesetMarkedForDeletion);
	LuaBinder::pushLuaCFuncMethod(l, "getMoveComponent", wrapSceneNodegetMoveComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getLightComponent", wrapSceneNodegetLightComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getLensFlareComponent", wrapSceneNodegetLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getDecalComponent", wrapSceneNodegetDecalComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getTriggerComponent", wrapSceneNodegetTriggerComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFogDensityComponent", wrapSceneNodegetFogDensityComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getFrustumComponent", wrapSceneNodegetFrustumComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getGlobalIlluminationProbeComponent",
								  wrapSceneNodegetGlobalIlluminationProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getReflectionProbeComponent", wrapSceneNodegetReflectionProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getBodyComponent", wrapSceneNodegetBodyComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getParticleEmitterComponent", wrapSceneNodegetParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getGpuParticleEmitterComponent", wrapSceneNodegetGpuParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getModelComponent", wrapSceneNodegetModelComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getSkinComponent", wrapSceneNodegetSkinComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getSkyboxComponent", wrapSceneNodegetSkyboxComponent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoModelNode = {
	-8050065867798521056, "ModelNode", LuaUserData::computeSizeForGarbageCollected<ModelNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ModelNode>()
{
	return luaUserDataTypeInfoModelNode;
}

/// Pre-wrap method ModelNode::getSceneNodeBase.
static inline int pwrapModelNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoModelNode, ud))
	{
		return -1;
	}

	ModelNode* self = ud->getData<ModelNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method ModelNode::getSceneNodeBase.
static int wrapModelNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapModelNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ModelNode.
static inline void wrapModelNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapModelNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoPerspectiveCameraNode = {
	4225474333355308316, "PerspectiveCameraNode", LuaUserData::computeSizeForGarbageCollected<PerspectiveCameraNode>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<PerspectiveCameraNode>()
{
	return luaUserDataTypeInfoPerspectiveCameraNode;
}

/// Pre-wrap method PerspectiveCameraNode::getSceneNodeBase.
static inline int pwrapPerspectiveCameraNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoPerspectiveCameraNode, ud))
	{
		return -1;
	}

	PerspectiveCameraNode* self = ud->getData<PerspectiveCameraNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method PerspectiveCameraNode::getSceneNodeBase.
static int wrapPerspectiveCameraNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapPerspectiveCameraNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class PerspectiveCameraNode.
static inline void wrapPerspectiveCameraNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoPerspectiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPerspectiveCameraNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoPointLightNode = {-476103666317335701, "PointLightNode",
														 LuaUserData::computeSizeForGarbageCollected<PointLightNode>(),
														 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<PointLightNode>()
{
	return luaUserDataTypeInfoPointLightNode;
}

/// Pre-wrap method PointLightNode::getSceneNodeBase.
static inline int pwrapPointLightNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoPointLightNode, ud))
	{
		return -1;
	}

	PointLightNode* self = ud->getData<PointLightNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method PointLightNode::getSceneNodeBase.
static int wrapPointLightNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapPointLightNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class PointLightNode.
static inline void wrapPointLightNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoPointLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPointLightNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSpotLightNode = {-9028941236445099739, "SpotLightNode",
														LuaUserData::computeSizeForGarbageCollected<SpotLightNode>(),
														nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SpotLightNode>()
{
	return luaUserDataTypeInfoSpotLightNode;
}

/// Pre-wrap method SpotLightNode::getSceneNodeBase.
static inline int pwrapSpotLightNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSpotLightNode, ud))
	{
		return -1;
	}

	SpotLightNode* self = ud->getData<SpotLightNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method SpotLightNode::getSceneNodeBase.
static int wrapSpotLightNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapSpotLightNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SpotLightNode.
static inline void wrapSpotLightNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSpotLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapSpotLightNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoDirectionalLightNode = {
	-5885044536233253731, "DirectionalLightNode", LuaUserData::computeSizeForGarbageCollected<DirectionalLightNode>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DirectionalLightNode>()
{
	return luaUserDataTypeInfoDirectionalLightNode;
}

/// Pre-wrap method DirectionalLightNode::getSceneNodeBase.
static inline int pwrapDirectionalLightNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoDirectionalLightNode, ud))
	{
		return -1;
	}

	DirectionalLightNode* self = ud->getData<DirectionalLightNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method DirectionalLightNode::getSceneNodeBase.
static int wrapDirectionalLightNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapDirectionalLightNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class DirectionalLightNode.
static inline void wrapDirectionalLightNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoDirectionalLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapDirectionalLightNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoStaticCollisionNode = {
	-3321869028945608148, "StaticCollisionNode", LuaUserData::computeSizeForGarbageCollected<StaticCollisionNode>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<StaticCollisionNode>()
{
	return luaUserDataTypeInfoStaticCollisionNode;
}

/// Pre-wrap method StaticCollisionNode::getSceneNodeBase.
static inline int pwrapStaticCollisionNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoStaticCollisionNode, ud))
	{
		return -1;
	}

	StaticCollisionNode* self = ud->getData<StaticCollisionNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method StaticCollisionNode::getSceneNodeBase.
static int wrapStaticCollisionNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapStaticCollisionNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class StaticCollisionNode.
static inline void wrapStaticCollisionNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapStaticCollisionNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterNode = {
	7361250518817995771, "ParticleEmitterNode", LuaUserData::computeSizeForGarbageCollected<ParticleEmitterNode>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ParticleEmitterNode>()
{
	return luaUserDataTypeInfoParticleEmitterNode;
}

/// Pre-wrap method ParticleEmitterNode::getSceneNodeBase.
static inline int pwrapParticleEmitterNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoParticleEmitterNode, ud))
	{
		return -1;
	}

	ParticleEmitterNode* self = ud->getData<ParticleEmitterNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method ParticleEmitterNode::getSceneNodeBase.
static int wrapParticleEmitterNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapParticleEmitterNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ParticleEmitterNode.
static inline void wrapParticleEmitterNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapParticleEmitterNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoGpuParticleEmitterNode = {
	-2839508838597227498, "GpuParticleEmitterNode",
	LuaUserData::computeSizeForGarbageCollected<GpuParticleEmitterNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GpuParticleEmitterNode>()
{
	return luaUserDataTypeInfoGpuParticleEmitterNode;
}

/// Pre-wrap method GpuParticleEmitterNode::getSceneNodeBase.
static inline int pwrapGpuParticleEmitterNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGpuParticleEmitterNode, ud))
	{
		return -1;
	}

	GpuParticleEmitterNode* self = ud->getData<GpuParticleEmitterNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method GpuParticleEmitterNode::getSceneNodeBase.
static int wrapGpuParticleEmitterNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapGpuParticleEmitterNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class GpuParticleEmitterNode.
static inline void wrapGpuParticleEmitterNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoGpuParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapGpuParticleEmitterNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeNode = {
	-5375614308316633893, "ReflectionProbeNode", LuaUserData::computeSizeForGarbageCollected<ReflectionProbeNode>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ReflectionProbeNode>()
{
	return luaUserDataTypeInfoReflectionProbeNode;
}

/// Pre-wrap method ReflectionProbeNode::getSceneNodeBase.
static inline int pwrapReflectionProbeNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoReflectionProbeNode, ud))
	{
		return -1;
	}

	ReflectionProbeNode* self = ud->getData<ReflectionProbeNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method ReflectionProbeNode::getSceneNodeBase.
static int wrapReflectionProbeNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapReflectionProbeNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ReflectionProbeNode.
static inline void wrapReflectionProbeNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoReflectionProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapReflectionProbeNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoDecalNode = {
	5640592796414838784, "DecalNode", LuaUserData::computeSizeForGarbageCollected<DecalNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DecalNode>()
{
	return luaUserDataTypeInfoDecalNode;
}

/// Pre-wrap method DecalNode::getSceneNodeBase.
static inline int pwrapDecalNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoDecalNode, ud))
	{
		return -1;
	}

	DecalNode* self = ud->getData<DecalNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method DecalNode::getSceneNodeBase.
static int wrapDecalNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapDecalNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class DecalNode.
static inline void wrapDecalNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapDecalNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoTriggerNode = {
	6454325227279593862, "TriggerNode", LuaUserData::computeSizeForGarbageCollected<TriggerNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<TriggerNode>()
{
	return luaUserDataTypeInfoTriggerNode;
}

/// Pre-wrap method TriggerNode::getSceneNodeBase.
static inline int pwrapTriggerNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoTriggerNode, ud))
	{
		return -1;
	}

	TriggerNode* self = ud->getData<TriggerNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method TriggerNode::getSceneNodeBase.
static int wrapTriggerNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapTriggerNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class TriggerNode.
static inline void wrapTriggerNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoTriggerNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapTriggerNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityNode = {-2016225087705583098, "FogDensityNode",
														 LuaUserData::computeSizeForGarbageCollected<FogDensityNode>(),
														 nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FogDensityNode>()
{
	return luaUserDataTypeInfoFogDensityNode;
}

/// Pre-wrap method FogDensityNode::getSceneNodeBase.
static inline int pwrapFogDensityNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFogDensityNode, ud))
	{
		return -1;
	}

	FogDensityNode* self = ud->getData<FogDensityNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method FogDensityNode::getSceneNodeBase.
static int wrapFogDensityNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapFogDensityNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class FogDensityNode.
static inline void wrapFogDensityNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoFogDensityNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapFogDensityNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeNode = {
	881842436982629931, "GlobalIlluminationProbeNode",
	LuaUserData::computeSizeForGarbageCollected<GlobalIlluminationProbeNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GlobalIlluminationProbeNode>()
{
	return luaUserDataTypeInfoGlobalIlluminationProbeNode;
}

/// Pre-wrap method GlobalIlluminationProbeNode::getSceneNodeBase.
static inline int pwrapGlobalIlluminationProbeNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeNode, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeNode* self = ud->getData<GlobalIlluminationProbeNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method GlobalIlluminationProbeNode::getSceneNodeBase.
static int wrapGlobalIlluminationProbeNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class GlobalIlluminationProbeNode.
static inline void wrapGlobalIlluminationProbeNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoGlobalIlluminationProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapGlobalIlluminationProbeNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSkyboxNode = {
	-2160062479151552786, "SkyboxNode", LuaUserData::computeSizeForGarbageCollected<SkyboxNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SkyboxNode>()
{
	return luaUserDataTypeInfoSkyboxNode;
}

/// Pre-wrap method SkyboxNode::getSceneNodeBase.
static inline int pwrapSkyboxNodegetSceneNodeBase(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSkyboxNode, ud))
	{
		return -1;
	}

	SkyboxNode* self = ud->getData<SkyboxNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, &ret);

	return 1;
}

/// Wrap method SkyboxNode::getSceneNodeBase.
static int wrapSkyboxNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapSkyboxNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SkyboxNode.
static inline void wrapSkyboxNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSkyboxNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapSkyboxNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSceneGraph = {
	1565953082694138001, "SceneGraph", LuaUserData::computeSizeForGarbageCollected<SceneGraph>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneGraph>()
{
	return luaUserDataTypeInfoSceneGraph;
}

/// Pre-wrap method SceneGraph::newPerspectiveCameraNode.
static inline int pwrapSceneGraphnewPerspectiveCameraNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	PerspectiveCameraNode* ret = newSceneNode<PerspectiveCameraNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "PerspectiveCameraNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoPerspectiveCameraNode;
	ud->initPointed(&luaUserDataTypeInfoPerspectiveCameraNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newPerspectiveCameraNode.
static int wrapSceneGraphnewPerspectiveCameraNode(lua_State* l)
{
	int res = pwrapSceneGraphnewPerspectiveCameraNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newModelNode.
static inline int pwrapSceneGraphnewModelNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	ModelNode* ret = newSceneNode<ModelNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ModelNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoModelNode;
	ud->initPointed(&luaUserDataTypeInfoModelNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newModelNode.
static int wrapSceneGraphnewModelNode(lua_State* l)
{
	int res = pwrapSceneGraphnewModelNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newPointLightNode.
static inline int pwrapSceneGraphnewPointLightNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	PointLightNode* ret = newSceneNode<PointLightNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "PointLightNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoPointLightNode;
	ud->initPointed(&luaUserDataTypeInfoPointLightNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newPointLightNode.
static int wrapSceneGraphnewPointLightNode(lua_State* l)
{
	int res = pwrapSceneGraphnewPointLightNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newSpotLightNode.
static inline int pwrapSceneGraphnewSpotLightNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	SpotLightNode* ret = newSceneNode<SpotLightNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SpotLightNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSpotLightNode;
	ud->initPointed(&luaUserDataTypeInfoSpotLightNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newSpotLightNode.
static int wrapSceneGraphnewSpotLightNode(lua_State* l)
{
	int res = pwrapSceneGraphnewSpotLightNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newDirectionalLightNode.
static inline int pwrapSceneGraphnewDirectionalLightNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	DirectionalLightNode* ret = newSceneNode<DirectionalLightNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DirectionalLightNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoDirectionalLightNode;
	ud->initPointed(&luaUserDataTypeInfoDirectionalLightNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newDirectionalLightNode.
static int wrapSceneGraphnewDirectionalLightNode(lua_State* l)
{
	int res = pwrapSceneGraphnewDirectionalLightNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newStaticCollisionNode.
static inline int pwrapSceneGraphnewStaticCollisionNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	StaticCollisionNode* ret = newSceneNode<StaticCollisionNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "StaticCollisionNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoStaticCollisionNode;
	ud->initPointed(&luaUserDataTypeInfoStaticCollisionNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newStaticCollisionNode.
static int wrapSceneGraphnewStaticCollisionNode(lua_State* l)
{
	int res = pwrapSceneGraphnewStaticCollisionNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newParticleEmitterNode.
static inline int pwrapSceneGraphnewParticleEmitterNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	ParticleEmitterNode* ret = newSceneNode<ParticleEmitterNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitterNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterNode;
	ud->initPointed(&luaUserDataTypeInfoParticleEmitterNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newParticleEmitterNode.
static int wrapSceneGraphnewParticleEmitterNode(lua_State* l)
{
	int res = pwrapSceneGraphnewParticleEmitterNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newGpuParticleEmitterNode.
static inline int pwrapSceneGraphnewGpuParticleEmitterNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	GpuParticleEmitterNode* ret = newSceneNode<GpuParticleEmitterNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GpuParticleEmitterNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoGpuParticleEmitterNode;
	ud->initPointed(&luaUserDataTypeInfoGpuParticleEmitterNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newGpuParticleEmitterNode.
static int wrapSceneGraphnewGpuParticleEmitterNode(lua_State* l)
{
	int res = pwrapSceneGraphnewGpuParticleEmitterNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newReflectionProbeNode.
static inline int pwrapSceneGraphnewReflectionProbeNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	ReflectionProbeNode* ret = newSceneNode<ReflectionProbeNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbeNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeNode;
	ud->initPointed(&luaUserDataTypeInfoReflectionProbeNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newReflectionProbeNode.
static int wrapSceneGraphnewReflectionProbeNode(lua_State* l)
{
	int res = pwrapSceneGraphnewReflectionProbeNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newDecalNode.
static inline int pwrapSceneGraphnewDecalNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	DecalNode* ret = newSceneNode<DecalNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoDecalNode;
	ud->initPointed(&luaUserDataTypeInfoDecalNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newDecalNode.
static int wrapSceneGraphnewDecalNode(lua_State* l)
{
	int res = pwrapSceneGraphnewDecalNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newTriggerNode.
static inline int pwrapSceneGraphnewTriggerNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	TriggerNode* ret = newSceneNode<TriggerNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "TriggerNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoTriggerNode;
	ud->initPointed(&luaUserDataTypeInfoTriggerNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newTriggerNode.
static int wrapSceneGraphnewTriggerNode(lua_State* l)
{
	int res = pwrapSceneGraphnewTriggerNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newGlobalIlluminationProbeNode.
static inline int pwrapSceneGraphnewGlobalIlluminationProbeNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	GlobalIlluminationProbeNode* ret = newSceneNode<GlobalIlluminationProbeNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GlobalIlluminationProbeNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeNode;
	ud->initPointed(&luaUserDataTypeInfoGlobalIlluminationProbeNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newGlobalIlluminationProbeNode.
static int wrapSceneGraphnewGlobalIlluminationProbeNode(lua_State* l)
{
	int res = pwrapSceneGraphnewGlobalIlluminationProbeNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newSkyboxNode.
static inline int pwrapSceneGraphnewSkyboxNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	SkyboxNode* ret = newSceneNode<SkyboxNode>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkyboxNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSkyboxNode;
	ud->initPointed(&luaUserDataTypeInfoSkyboxNode, ret);

	return 1;
}

/// Wrap method SceneGraph::newSkyboxNode.
static int wrapSceneGraphnewSkyboxNode(lua_State* l)
{
	int res = pwrapSceneGraphnewSkyboxNode(l);
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoSceneNode, ud)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	SceneNode* ret = self->tryFindSceneNode(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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
	LuaBinder::pushLuaCFuncMethod(l, "newPerspectiveCameraNode", wrapSceneGraphnewPerspectiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "newModelNode", wrapSceneGraphnewModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "newPointLightNode", wrapSceneGraphnewPointLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newSpotLightNode", wrapSceneGraphnewSpotLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newDirectionalLightNode", wrapSceneGraphnewDirectionalLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newStaticCollisionNode", wrapSceneGraphnewStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitterNode", wrapSceneGraphnewParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "newGpuParticleEmitterNode", wrapSceneGraphnewGpuParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProbeNode", wrapSceneGraphnewReflectionProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "newDecalNode", wrapSceneGraphnewDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "newTriggerNode", wrapSceneGraphnewTriggerNode);
	LuaBinder::pushLuaCFuncMethod(l, "newGlobalIlluminationProbeNode", wrapSceneGraphnewGlobalIlluminationProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "newSkyboxNode", wrapSceneGraphnewSkyboxNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCameraNode", wrapSceneGraphsetActiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "tryFindSceneNode", wrapSceneGraphtryFindSceneNode);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoEvent = {-127482733468747645, "Event",
												LuaUserData::computeSizeForGarbageCollected<Event>(), nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 1)))
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

LuaUserDataTypeInfo luaUserDataTypeInfoLightEvent = {
	-1511364883147733152, "LightEvent", LuaUserData::computeSizeForGarbageCollected<LightEvent>(), nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 2)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	F32 arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 3, arg1)))
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

LuaUserDataTypeInfo luaUserDataTypeInfoScriptEvent = {
	-577692491395731788, "ScriptEvent", LuaUserData::computeSizeForGarbageCollected<ScriptEvent>(), nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoJitterMoveEvent = {
	3496086456905036915, "JitterMoveEvent", LuaUserData::computeSizeForGarbageCollected<JitterMoveEvent>(), nullptr,
	nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 2, luaUserDataTypeInfoVec4, ud)))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 3, luaUserDataTypeInfoVec4, ud)))
	{
		return -1;
	}

	Vec4* iarg1 = ud->getData<Vec4>();
	const Vec4& arg1(*iarg1);

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

LuaUserDataTypeInfo luaUserDataTypeInfoAnimationEvent = {-1032810681493696534, "AnimationEvent",
														 LuaUserData::computeSizeForGarbageCollected<AnimationEvent>(),
														 nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoEventManager = {
	330041233457660952, "EventManager", LuaUserData::computeSizeForGarbageCollected<EventManager>(), nullptr, nullptr};

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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	F32 arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 3, arg1)))
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoSceneNode, ud)))
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	LightEvent* ret = newEvent<LightEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	F32 arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 3, arg1)))
	{
		return -1;
	}

	const char* arg2;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 4, arg2)))
	{
		return -1;
	}

	// Call the method
	ScriptEvent* ret = newEvent<ScriptEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 2, arg0)))
	{
		return -1;
	}

	F32 arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 3, arg1)))
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoSceneNode, ud)))
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	JitterMoveEvent* ret = newEvent<JitterMoveEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoSceneNode, ud)))
	{
		return -1;
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	AnimationEvent* ret = newEvent<AnimationEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 0)))
	{
		return -1;
	}

	// Call the function
	SceneGraph* ret = getSceneGraph(l);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 0)))
	{
		return -1;
	}

	// Call the function
	EventManager* ret = getEventManager(l);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
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
	wrapWeakArrayBodyComponentPtr(l);
	wrapMoveComponent(l);
	wrapLightComponent(l);
	wrapDecalComponent(l);
	wrapLensFlareComponent(l);
	wrapBodyComponent(l);
	wrapTriggerComponent(l);
	wrapFogDensityComponent(l);
	wrapFrustumComponent(l);
	wrapGlobalIlluminationProbeComponent(l);
	wrapReflectionProbeComponent(l);
	wrapParticleEmitterComponent(l);
	wrapGpuParticleEmitterComponent(l);
	wrapModelComponent(l);
	wrapSkinComponent(l);
	wrapSkyboxComponent(l);
	wrapSceneNode(l);
	wrapModelNode(l);
	wrapPerspectiveCameraNode(l);
	wrapPointLightNode(l);
	wrapSpotLightNode(l);
	wrapDirectionalLightNode(l);
	wrapStaticCollisionNode(l);
	wrapParticleEmitterNode(l);
	wrapGpuParticleEmitterNode(l);
	wrapReflectionProbeNode(l);
	wrapDecalNode(l);
	wrapTriggerNode(l);
	wrapFogDensityNode(l);
	wrapGlobalIlluminationProbeNode(l);
	wrapSkyboxNode(l);
	wrapSceneGraph(l);
	wrapEvent(l);
	wrapLightEvent(l);
	wrapScriptEvent(l);
	wrapJitterMoveEvent(l);
	wrapAnimationEvent(l);
	wrapEventManager(l);
	LuaBinder::pushLuaCFunc(l, "getSceneGraph", wrapgetSceneGraph);
	LuaBinder::pushLuaCFunc(l, "getEventManager", wrapgetEventManager);
}

} // end namespace anki
