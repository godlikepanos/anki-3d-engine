// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <anki/script/LuaBinder.h>
#include <anki/script/ScriptManager.h>
#include <anki/Scene.h>

namespace anki
{

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

LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr = {4158963409681942864,
	"WeakArraySceneNodePtr",
	LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<WeakArraySceneNodePtr>()
{
	return luaUserDataTypeInfoWeakArraySceneNodePtr;
}

/// Pre-wrap method WeakArraySceneNodePtr::getSize.
static inline int pwrapWeakArraySceneNodePtrgetSize(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoWeakArraySceneNodePtr, ud))
	{
		return -1;
	}

	WeakArraySceneNodePtr* self = ud->getData<WeakArraySceneNodePtr>();

	// Call the method
	PtrSize ret = self->getSize();

	// Push return value
	lua_pushnumber(l, ret);

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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoWeakArraySceneNodePtr, ud))
	{
		return -1;
	}

	WeakArraySceneNodePtr* self = ud->getData<WeakArraySceneNodePtr>();

	// Pop arguments
	U arg0;
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoMoveComponent = {2038493110845313445,
	"MoveComponent",
	LuaUserData::computeSizeForGarbageCollected<MoveComponent>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MoveComponent>()
{
	return luaUserDataTypeInfoMoveComponent;
}

/// Pre-wrap method MoveComponent::setLocalOrigin.
static inline int pwrapMoveComponentsetLocalOrigin(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoVec4, const_cast<Vec4*>(&ret));

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
	ud->initPointed(&luaUserDataTypeInfoMat3x4, const_cast<Mat3x4*>(&ret));

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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoMoveComponent, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Call the method
	F32 ret = self->getLocalScale();

	// Push return value
	lua_pushnumber(l, ret);

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
	ud->initPointed(&luaUserDataTypeInfoTransform, const_cast<Transform*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoLightComponent = {7940823622056993903,
	"LightComponent",
	LuaUserData::computeSizeForGarbageCollected<LightComponent>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightComponent>()
{
	return luaUserDataTypeInfoLightComponent;
}

/// Pre-wrap method LightComponent::setDiffuseColor.
static inline int pwrapLightComponentsetDiffuseColor(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoVec4, const_cast<Vec4*>(&ret));

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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getRadius();

	// Push return value
	lua_pushnumber(l, ret);

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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getDistance();

	// Push return value
	lua_pushnumber(l, ret);

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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getInnerAngle();

	// Push return value
	lua_pushnumber(l, ret);

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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoLightComponent, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getOuterAngle();

	// Push return value
	lua_pushnumber(l, ret);

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

LuaUserDataTypeInfo luaUserDataTypeInfoDecalComponent = {-1979693900066114370,
	"DecalComponent",
	LuaUserData::computeSizeForGarbageCollected<DecalComponent>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DecalComponent>()
{
	return luaUserDataTypeInfoDecalComponent;
}

/// Pre-wrap method DecalComponent::setDiffuseDecal.
static inline int pwrapDecalComponentsetDiffuseDecal(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

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

	lua_pushnumber(l, ret);

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
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

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

	lua_pushnumber(l, ret);

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

/// Pre-wrap method DecalComponent::updateShape.
static inline int pwrapDecalComponentupdateShape(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

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

	// Call the method
	self->updateShape(arg0, arg1, arg2);

	return 0;
}

/// Wrap method DecalComponent::updateShape.
static int wrapDecalComponentupdateShape(lua_State* l)
{
	int res = pwrapDecalComponentupdateShape(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "updateShape", wrapDecalComponentupdateShape);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoLensFlareComponent = {-2019248835133422777,
	"LensFlareComponent",
	LuaUserData::computeSizeForGarbageCollected<LensFlareComponent>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LensFlareComponent>()
{
	return luaUserDataTypeInfoLensFlareComponent;
}

/// Pre-wrap method LensFlareComponent::setFirstFlareSize.
static inline int pwrapLensFlareComponentsetFirstFlareSize(lua_State* l)
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
	LuaBinder::pushLuaCFuncMethod(l, "setFirstFlareSize", wrapLensFlareComponentsetFirstFlareSize);
	LuaBinder::pushLuaCFuncMethod(l, "setColorMultiplier", wrapLensFlareComponentsetColorMultiplier);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoTriggerComponent = {7180780522076545145,
	"TriggerComponent",
	LuaUserData::computeSizeForGarbageCollected<TriggerComponent>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<TriggerComponent>()
{
	return luaUserDataTypeInfoTriggerComponent;
}

/// Pre-wrap method TriggerComponent::getContactSceneNodes.
static inline int pwrapTriggerComponentgetContactSceneNodes(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoTriggerComponent, ud))
	{
		return -1;
	}

	TriggerComponent* self = ud->getData<TriggerComponent>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getContactSceneNodes();

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

/// Wrap method TriggerComponent::getContactSceneNodes.
static int wrapTriggerComponentgetContactSceneNodes(lua_State* l)
{
	int res = pwrapTriggerComponentgetContactSceneNodes(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "getContactSceneNodes", wrapTriggerComponentgetContactSceneNodes);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityComponent = {3433641273323722630,
	"FogDensityComponent",
	LuaUserData::computeSizeForGarbageCollected<FogDensityComponent>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FogDensityComponent>()
{
	return luaUserDataTypeInfoFogDensityComponent;
}

/// Pre-wrap method FogDensityComponent::setAabb.
static inline int pwrapFogDensityComponentsetAabb(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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
	self->setAabb(arg0, arg1);

	return 0;
}

/// Wrap method FogDensityComponent::setAabb.
static int wrapFogDensityComponentsetAabb(lua_State* l)
{
	int res = pwrapFogDensityComponentsetAabb(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method FogDensityComponent::setSphere.
static inline int pwrapFogDensityComponentsetSphere(lua_State* l)
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
	self->setSphere(arg0);

	return 0;
}

/// Wrap method FogDensityComponent::setSphere.
static int wrapFogDensityComponentsetSphere(lua_State* l)
{
	int res = pwrapFogDensityComponentsetSphere(l);
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoFogDensityComponent, ud))
	{
		return -1;
	}

	FogDensityComponent* self = ud->getData<FogDensityComponent>();

	// Call the method
	F32 ret = self->getDensity();

	// Push return value
	lua_pushnumber(l, ret);

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
	LuaBinder::pushLuaCFuncMethod(l, "setAabb", wrapFogDensityComponentsetAabb);
	LuaBinder::pushLuaCFuncMethod(l, "setSphere", wrapFogDensityComponentsetSphere);
	LuaBinder::pushLuaCFuncMethod(l, "setDensity", wrapFogDensityComponentsetDensity);
	LuaBinder::pushLuaCFuncMethod(l, "getDensity", wrapFogDensityComponentgetDensity);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode = {
	-2220074417980276571, "SceneNode", LuaUserData::computeSizeForGarbageCollected<SceneNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneNode>()
{
	return luaUserDataTypeInfoSceneNode;
}

/// Pre-wrap method SceneNode::getName.
static inline int pwrapSceneNodegetName(lua_State* l)
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

/// Pre-wrap method SceneNode::tryGetComponent<MoveComponent>.
static inline int pwrapSceneNodegetMoveComponent(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MoveComponent* ret = self->tryGetComponent<MoveComponent>();

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
	ud->initPointed(&luaUserDataTypeInfoMoveComponent, const_cast<MoveComponent*>(ret));

	return 1;
}

/// Wrap method SceneNode::tryGetComponent<MoveComponent>.
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

/// Pre-wrap method SceneNode::tryGetComponent<LightComponent>.
static inline int pwrapSceneNodegetLightComponent(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LightComponent* ret = self->tryGetComponent<LightComponent>();

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
	ud->initPointed(&luaUserDataTypeInfoLightComponent, const_cast<LightComponent*>(ret));

	return 1;
}

/// Wrap method SceneNode::tryGetComponent<LightComponent>.
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

/// Pre-wrap method SceneNode::tryGetComponent<LensFlareComponent>.
static inline int pwrapSceneNodegetLensFlareComponent(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LensFlareComponent* ret = self->tryGetComponent<LensFlareComponent>();

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
	ud->initPointed(&luaUserDataTypeInfoLensFlareComponent, const_cast<LensFlareComponent*>(ret));

	return 1;
}

/// Wrap method SceneNode::tryGetComponent<LensFlareComponent>.
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

/// Pre-wrap method SceneNode::tryGetComponent<DecalComponent>.
static inline int pwrapSceneNodegetDecalComponent(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	DecalComponent* ret = self->tryGetComponent<DecalComponent>();

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
	ud->initPointed(&luaUserDataTypeInfoDecalComponent, const_cast<DecalComponent*>(ret));

	return 1;
}

/// Wrap method SceneNode::tryGetComponent<DecalComponent>.
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

/// Pre-wrap method SceneNode::tryGetComponent<TriggerComponent>.
static inline int pwrapSceneNodegetTriggerComponent(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	TriggerComponent* ret = self->tryGetComponent<TriggerComponent>();

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
	ud->initPointed(&luaUserDataTypeInfoTriggerComponent, const_cast<TriggerComponent*>(ret));

	return 1;
}

/// Wrap method SceneNode::tryGetComponent<TriggerComponent>.
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

/// Pre-wrap method SceneNode::tryGetComponent<FogDensityComponent>.
static inline int pwrapSceneNodegetFogDensityComponent(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoSceneNode, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	FogDensityComponent* ret = self->tryGetComponent<FogDensityComponent>();

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
	ud->initPointed(&luaUserDataTypeInfoFogDensityComponent, const_cast<FogDensityComponent*>(ret));

	return 1;
}

/// Wrap method SceneNode::tryGetComponent<FogDensityComponent>.
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
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoModelNode = {
	-1856316251880904290, "ModelNode", LuaUserData::computeSizeForGarbageCollected<ModelNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ModelNode>()
{
	return luaUserDataTypeInfoModelNode;
}

/// Pre-wrap method ModelNode::getSceneNodeBase.
static inline int pwrapModelNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoPerspectiveCameraNode = {-7590637754681648962,
	"PerspectiveCameraNode",
	LuaUserData::computeSizeForGarbageCollected<PerspectiveCameraNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<PerspectiveCameraNode>()
{
	return luaUserDataTypeInfoPerspectiveCameraNode;
}

/// Pre-wrap method PerspectiveCameraNode::getSceneNodeBase.
static inline int pwrapPerspectiveCameraNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

/// Pre-wrap method PerspectiveCameraNode::setAll.
static inline int pwrapPerspectiveCameraNodesetAll(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 5)))
	{
		return -1;
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoPerspectiveCameraNode, ud))
	{
		return -1;
	}

	PerspectiveCameraNode* self = ud->getData<PerspectiveCameraNode>();

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
	self->setAll(arg0, arg1, arg2, arg3);

	return 0;
}

/// Wrap method PerspectiveCameraNode::setAll.
static int wrapPerspectiveCameraNodesetAll(lua_State* l)
{
	int res = pwrapPerspectiveCameraNodesetAll(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapPerspectiveCameraNodesetAll);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoPointLightNode = {8507789763949195644,
	"PointLightNode",
	LuaUserData::computeSizeForGarbageCollected<PointLightNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<PointLightNode>()
{
	return luaUserDataTypeInfoPointLightNode;
}

/// Pre-wrap method PointLightNode::getSceneNodeBase.
static inline int pwrapPointLightNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

/// Pre-wrap method PointLightNode::loadLensFlare.
static inline int pwrapPointLightNodeloadLensFlare(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoPointLightNode, ud))
	{
		return -1;
	}

	PointLightNode* self = ud->getData<PointLightNode>();

	// Pop arguments
	const char* arg0;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 2, arg0)))
	{
		return -1;
	}

	// Call the method
	Error ret = self->loadLensFlare(arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method PointLightNode::loadLensFlare.
static int wrapPointLightNodeloadLensFlare(lua_State* l)
{
	int res = pwrapPointLightNodeloadLensFlare(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "loadLensFlare", wrapPointLightNodeloadLensFlare);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSpotLightNode = {-9214759951813290587,
	"SpotLightNode",
	LuaUserData::computeSizeForGarbageCollected<SpotLightNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SpotLightNode>()
{
	return luaUserDataTypeInfoSpotLightNode;
}

/// Pre-wrap method SpotLightNode::getSceneNodeBase.
static inline int pwrapSpotLightNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoStaticCollisionNode = {-4376619865753613291,
	"StaticCollisionNode",
	LuaUserData::computeSizeForGarbageCollected<StaticCollisionNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<StaticCollisionNode>()
{
	return luaUserDataTypeInfoStaticCollisionNode;
}

/// Pre-wrap method StaticCollisionNode::getSceneNodeBase.
static inline int pwrapStaticCollisionNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterNode = {4851204309813771919,
	"ParticleEmitterNode",
	LuaUserData::computeSizeForGarbageCollected<ParticleEmitterNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ParticleEmitterNode>()
{
	return luaUserDataTypeInfoParticleEmitterNode;
}

/// Pre-wrap method ParticleEmitterNode::getSceneNodeBase.
static inline int pwrapParticleEmitterNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoReflectionProbeNode = {-801309373000950648,
	"ReflectionProbeNode",
	LuaUserData::computeSizeForGarbageCollected<ReflectionProbeNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ReflectionProbeNode>()
{
	return luaUserDataTypeInfoReflectionProbeNode;
}

/// Pre-wrap method ReflectionProbeNode::getSceneNodeBase.
static inline int pwrapReflectionProbeNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoOccluderNode = {-6885028590097645115,
	"OccluderNode",
	LuaUserData::computeSizeForGarbageCollected<OccluderNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<OccluderNode>()
{
	return luaUserDataTypeInfoOccluderNode;
}

/// Pre-wrap method OccluderNode::getSceneNodeBase.
static inline int pwrapOccluderNodegetSceneNodeBase(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoOccluderNode, ud))
	{
		return -1;
	}

	OccluderNode* self = ud->getData<OccluderNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode;
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method OccluderNode::getSceneNodeBase.
static int wrapOccluderNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapOccluderNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class OccluderNode.
static inline void wrapOccluderNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoOccluderNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapOccluderNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoDecalNode = {
	1097508121406753350, "DecalNode", LuaUserData::computeSizeForGarbageCollected<DecalNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DecalNode>()
{
	return luaUserDataTypeInfoDecalNode;
}

/// Pre-wrap method DecalNode::getSceneNodeBase.
static inline int pwrapDecalNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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
	-3029786875306006141, "TriggerNode", LuaUserData::computeSizeForGarbageCollected<TriggerNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<TriggerNode>()
{
	return luaUserDataTypeInfoTriggerNode;
}

/// Pre-wrap method TriggerNode::getSceneNodeBase.
static inline int pwrapTriggerNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityNode = {-2463430472135938886,
	"FogDensityNode",
	LuaUserData::computeSizeForGarbageCollected<FogDensityNode>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FogDensityNode>()
{
	return luaUserDataTypeInfoFogDensityNode;
}

/// Pre-wrap method FogDensityNode::getSceneNodeBase.
static inline int pwrapFogDensityNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoSceneGraph = {
	-7754439619132389154, "SceneGraph", LuaUserData::computeSizeForGarbageCollected<SceneGraph>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneGraph>()
{
	return luaUserDataTypeInfoSceneGraph;
}

/// Pre-wrap method SceneGraph::newPerspectiveCameraNode.
static inline int pwrapSceneGraphnewPerspectiveCameraNode(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoPerspectiveCameraNode, const_cast<PerspectiveCameraNode*>(ret));

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
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	// Call the method
	ModelNode* ret = newSceneNode<ModelNode>(self, arg0, arg1);

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
	ud->initPointed(&luaUserDataTypeInfoModelNode, const_cast<ModelNode*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoPointLightNode, const_cast<PointLightNode*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoSpotLightNode, const_cast<SpotLightNode*>(ret));

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

/// Pre-wrap method SceneGraph::newStaticCollisionNode.
static inline int pwrapSceneGraphnewStaticCollisionNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	extern LuaUserDataTypeInfo luaUserDataTypeInfoTransform;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoTransform, ud)))
	{
		return -1;
	}

	Transform* iarg2 = ud->getData<Transform>();
	const Transform& arg2(*iarg2);

	// Call the method
	StaticCollisionNode* ret = newSceneNode<StaticCollisionNode>(self, arg0, arg1, arg2);

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
	ud->initPointed(&luaUserDataTypeInfoStaticCollisionNode, const_cast<StaticCollisionNode*>(ret));

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
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	// Call the method
	ParticleEmitterNode* ret = newSceneNode<ParticleEmitterNode>(self, arg0, arg1);

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
	ud->initPointed(&luaUserDataTypeInfoParticleEmitterNode, const_cast<ParticleEmitterNode*>(ret));

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

/// Pre-wrap method SceneGraph::newReflectionProbeNode.
static inline int pwrapSceneGraphnewReflectionProbeNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 4)))
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

	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 3, luaUserDataTypeInfoVec4, ud)))
	{
		return -1;
	}

	Vec4* iarg1 = ud->getData<Vec4>();
	const Vec4& arg1(*iarg1);

	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	if(ANKI_UNLIKELY(LuaBinder::checkUserData(l, 4, luaUserDataTypeInfoVec4, ud)))
	{
		return -1;
	}

	Vec4* iarg2 = ud->getData<Vec4>();
	const Vec4& arg2(*iarg2);

	// Call the method
	ReflectionProbeNode* ret = newSceneNode<ReflectionProbeNode>(self, arg0, arg1, arg2);

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
	ud->initPointed(&luaUserDataTypeInfoReflectionProbeNode, const_cast<ReflectionProbeNode*>(ret));

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

/// Pre-wrap method SceneGraph::newOccluderNode.
static inline int pwrapSceneGraphnewOccluderNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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

	const char* arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkString(l, 3, arg1)))
	{
		return -1;
	}

	// Call the method
	OccluderNode* ret = newSceneNode<OccluderNode>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "OccluderNode");
	extern LuaUserDataTypeInfo luaUserDataTypeInfoOccluderNode;
	ud->initPointed(&luaUserDataTypeInfoOccluderNode, const_cast<OccluderNode*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newOccluderNode.
static int wrapSceneGraphnewOccluderNode(lua_State* l)
{
	int res = pwrapSceneGraphnewOccluderNode(l);
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
	ud->initPointed(&luaUserDataTypeInfoDecalNode, const_cast<DecalNode*>(ret));

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
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	if(ANKI_UNLIKELY(LuaBinder::checkArgsCount(l, 3)))
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

	F32 arg1;
	if(ANKI_UNLIKELY(LuaBinder::checkNumber(l, 3, arg1)))
	{
		return -1;
	}

	// Call the method
	TriggerNode* ret = newSceneNode<TriggerNode>(self, arg0, arg1);

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
	ud->initPointed(&luaUserDataTypeInfoTriggerNode, const_cast<TriggerNode*>(ret));

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

/// Pre-wrap method SceneGraph::setActiveCameraNode.
static inline int pwrapSceneGraphsetActiveCameraNode(lua_State* l)
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

/// Wrap class SceneGraph.
static inline void wrapSceneGraph(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoSceneGraph);
	LuaBinder::pushLuaCFuncMethod(l, "newPerspectiveCameraNode", wrapSceneGraphnewPerspectiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "newModelNode", wrapSceneGraphnewModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "newPointLightNode", wrapSceneGraphnewPointLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newSpotLightNode", wrapSceneGraphnewSpotLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newStaticCollisionNode", wrapSceneGraphnewStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitterNode", wrapSceneGraphnewParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProbeNode", wrapSceneGraphnewReflectionProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "newOccluderNode", wrapSceneGraphnewOccluderNode);
	LuaBinder::pushLuaCFuncMethod(l, "newDecalNode", wrapSceneGraphnewDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "newTriggerNode", wrapSceneGraphnewTriggerNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCameraNode", wrapSceneGraphsetActiveCameraNode);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoEvent = {
	1660689530604735101, "Event", LuaUserData::computeSizeForGarbageCollected<Event>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Event>()
{
	return luaUserDataTypeInfoEvent;
}

/// Pre-wrap method Event::getAssociatedSceneNodes.
static inline int pwrapEventgetAssociatedSceneNodes(lua_State* l)
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
	840634010629725278, "LightEvent", LuaUserData::computeSizeForGarbageCollected<LightEvent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightEvent>()
{
	return luaUserDataTypeInfoLightEvent;
}

/// Pre-wrap method LightEvent::setIntensityMultiplier.
static inline int pwrapLightEventsetIntensityMultiplier(lua_State* l)
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
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

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

LuaUserDataTypeInfo luaUserDataTypeInfoEventManager = {-6959305329499243407,
	"EventManager",
	LuaUserData::computeSizeForGarbageCollected<EventManager>(),
	nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<EventManager>()
{
	return luaUserDataTypeInfoEventManager;
}

/// Pre-wrap method EventManager::newLightEvent.
static inline int pwrapEventManagernewLightEvent(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

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
	ud->initPointed(&luaUserDataTypeInfoLightEvent, const_cast<LightEvent*>(ret));

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
	LuaBinder::createClass(l, &luaUserDataTypeInfoEventManager);
	LuaBinder::pushLuaCFuncMethod(l, "newLightEvent", wrapEventManagernewLightEvent);
	lua_settop(l, 0);
}

/// Pre-wrap function getSceneGraph.
static inline int pwrapgetSceneGraph(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneGraph, const_cast<SceneGraph*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoEventManager, const_cast<EventManager*>(ret));

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
	wrapMoveComponent(l);
	wrapLightComponent(l);
	wrapDecalComponent(l);
	wrapLensFlareComponent(l);
	wrapTriggerComponent(l);
	wrapFogDensityComponent(l);
	wrapSceneNode(l);
	wrapModelNode(l);
	wrapPerspectiveCameraNode(l);
	wrapPointLightNode(l);
	wrapSpotLightNode(l);
	wrapStaticCollisionNode(l);
	wrapParticleEmitterNode(l);
	wrapReflectionProbeNode(l);
	wrapOccluderNode(l);
	wrapDecalNode(l);
	wrapTriggerNode(l);
	wrapFogDensityNode(l);
	wrapSceneGraph(l);
	wrapEvent(l);
	wrapLightEvent(l);
	wrapEventManager(l);
	LuaBinder::pushLuaCFunc(l, "getSceneGraph", wrapgetSceneGraph);
	LuaBinder::pushLuaCFunc(l, "getEventManager", wrapgetEventManager);
}

} // end namespace anki
