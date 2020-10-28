// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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

LuaUserDataTypeInfo luaUserDataTypeInfoWeakArraySceneNodePtr = {
	-7803856304390433192, "WeakArraySceneNodePtr", LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>(),
	nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoMoveComponent = {-4954964769395972691, "MoveComponent",
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

LuaUserDataTypeInfo luaUserDataTypeInfoLightComponent = {-2465694465014111240, "LightComponent",
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

LuaUserDataTypeInfo luaUserDataTypeInfoDecalComponent = {2056814406591907591, "DecalComponent",
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

LuaUserDataTypeInfo luaUserDataTypeInfoLensFlareComponent = {
	2254908652583381035, "LensFlareComponent", LuaUserData::computeSizeForGarbageCollected<LensFlareComponent>(),
	nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoTriggerComponent = {
	-725997510931751156, "TriggerComponent", LuaUserData::computeSizeForGarbageCollected<TriggerComponent>(), nullptr,
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

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityComponent = {
	-4216538413369385171, "FogDensityComponent", LuaUserData::computeSizeForGarbageCollected<FogDensityComponent>(),
	nullptr, nullptr};

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
	LuaBinder::pushLuaCFuncMethod(l, "setAabb", wrapFogDensityComponentsetAabb);
	LuaBinder::pushLuaCFuncMethod(l, "setSphere", wrapFogDensityComponentsetSphere);
	LuaBinder::pushLuaCFuncMethod(l, "setDensity", wrapFogDensityComponentsetDensity);
	LuaBinder::pushLuaCFuncMethod(l, "getDensity", wrapFogDensityComponentgetDensity);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoFrustumComponent = {
	1685454671119522630, "FrustumComponent", LuaUserData::computeSizeForGarbageCollected<FrustumComponent>(), nullptr,
	nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FrustumComponent>()
{
	return luaUserDataTypeInfoFrustumComponent;
}

/// Pre-wrap method FrustumComponent::setPerspective.
static inline int pwrapFrustumComponentsetPerspective(lua_State* l)
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
	6593269886846506122, "GlobalIlluminationProbeComponent",
	LuaUserData::computeSizeForGarbageCollected<GlobalIlluminationProbeComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GlobalIlluminationProbeComponent>()
{
	return luaUserDataTypeInfoGlobalIlluminationProbeComponent;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::setBoundingBox.
static inline int pwrapGlobalIlluminationProbeComponentsetBoundingBox(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

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
	self->setBoundingBox(arg0, arg1);

	return 0;
}

/// Wrap method GlobalIlluminationProbeComponent::setBoundingBox.
static int wrapGlobalIlluminationProbeComponentsetBoundingBox(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentsetBoundingBox(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::getAlignedBoundingBoxMin.
static inline int pwrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMin(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Call the method
	Vec4 ret = self->getAlignedBoundingBoxMin();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method GlobalIlluminationProbeComponent::getAlignedBoundingBoxMin.
static int wrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMin(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method GlobalIlluminationProbeComponent::getAlignedBoundingBoxMax.
static inline int pwrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMax(lua_State* l)
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
	if(LuaBinder::checkUserData(l, 1, luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud))
	{
		return -1;
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Call the method
	Vec4 ret = self->getAlignedBoundingBoxMax();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method GlobalIlluminationProbeComponent::getAlignedBoundingBoxMax.
static int wrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMax(lua_State* l)
{
	int res = pwrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMax(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "setBoundingBox", wrapGlobalIlluminationProbeComponentsetBoundingBox);
	LuaBinder::pushLuaCFuncMethod(l, "getAlignedBoundingBoxMin",
								  wrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMin);
	LuaBinder::pushLuaCFuncMethod(l, "getAlignedBoundingBoxMax",
								  wrapGlobalIlluminationProbeComponentgetAlignedBoundingBoxMax);
	LuaBinder::pushLuaCFuncMethod(l, "setCellSize", wrapGlobalIlluminationProbeComponentsetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "getCellSize", wrapGlobalIlluminationProbeComponentgetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "setFadeDistance", wrapGlobalIlluminationProbeComponentsetFadeDistance);
	LuaBinder::pushLuaCFuncMethod(l, "getFadeDistance", wrapGlobalIlluminationProbeComponentgetFadeDistance);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoSceneNode = {
	-4596567984428317122, "SceneNode", LuaUserData::computeSizeForGarbageCollected<SceneNode>(), nullptr, nullptr};

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

/// Pre-wrap method SceneNode::tryGetFirstComponentOfType<MoveComponent>.
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
	ud->initPointed(&luaUserDataTypeInfoMoveComponent, const_cast<MoveComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoLightComponent, const_cast<LightComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoLensFlareComponent, const_cast<LensFlareComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoDecalComponent, const_cast<DecalComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoTriggerComponent, const_cast<TriggerComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoFogDensityComponent, const_cast<FogDensityComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoFrustumComponent, const_cast<FrustumComponent*>(ret));

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
	ud->initPointed(&luaUserDataTypeInfoGlobalIlluminationProbeComponent,
					const_cast<GlobalIlluminationProbeComponent*>(ret));

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
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoModelNode = {
	-450705026354471566, "ModelNode", LuaUserData::computeSizeForGarbageCollected<ModelNode>(), nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoPerspectiveCameraNode = {
	5203874346323105211, "PerspectiveCameraNode", LuaUserData::computeSizeForGarbageCollected<PerspectiveCameraNode>(),
	nullptr, nullptr};

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

/// Wrap class PerspectiveCameraNode.
static inline void wrapPerspectiveCameraNode(lua_State* l)
{
	LuaBinder::createClass(l, &luaUserDataTypeInfoPerspectiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPerspectiveCameraNodegetSceneNodeBase);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoPointLightNode = {5481907254599316993, "PointLightNode",
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

	lua_pushnumber(l, lua_Number(!!ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoSpotLightNode = {-4171144965887956516, "SpotLightNode",
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

LuaUserDataTypeInfo luaUserDataTypeInfoDirectionalLightNode = {
	-5947638574932730667, "DirectionalLightNode", LuaUserData::computeSizeForGarbageCollected<DirectionalLightNode>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DirectionalLightNode>()
{
	return luaUserDataTypeInfoDirectionalLightNode;
}

/// Pre-wrap method DirectionalLightNode::getSceneNodeBase.
static inline int pwrapDirectionalLightNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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
	7611855183596540382, "StaticCollisionNode", LuaUserData::computeSizeForGarbageCollected<StaticCollisionNode>(),
	nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoParticleEmitterNode = {
	-4528616999498906935, "ParticleEmitterNode", LuaUserData::computeSizeForGarbageCollected<ParticleEmitterNode>(),
	nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoGpuParticleEmitterNode = {
	-1474617733666958838, "GpuParticleEmitterNode",
	LuaUserData::computeSizeForGarbageCollected<GpuParticleEmitterNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GpuParticleEmitterNode>()
{
	return luaUserDataTypeInfoGpuParticleEmitterNode;
}

/// Pre-wrap method GpuParticleEmitterNode::getSceneNodeBase.
static inline int pwrapGpuParticleEmitterNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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
	7820862353662459657, "ReflectionProbeNode", LuaUserData::computeSizeForGarbageCollected<ReflectionProbeNode>(),
	nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoOccluderNode = {-8953939426134655474, "OccluderNode",
													   LuaUserData::computeSizeForGarbageCollected<OccluderNode>(),
													   nullptr, nullptr};

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
	8416942623307160724, "DecalNode", LuaUserData::computeSizeForGarbageCollected<DecalNode>(), nullptr, nullptr};

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
	2501977285852887294, "TriggerNode", LuaUserData::computeSizeForGarbageCollected<TriggerNode>(), nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoFogDensityNode = {-4989050962579528993, "FogDensityNode",
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

LuaUserDataTypeInfo luaUserDataTypeInfoGlobalIlluminationProbeNode = {
	710930745580002886, "GlobalIlluminationProbeNode",
	LuaUserData::computeSizeForGarbageCollected<GlobalIlluminationProbeNode>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GlobalIlluminationProbeNode>()
{
	return luaUserDataTypeInfoGlobalIlluminationProbeNode;
}

/// Pre-wrap method GlobalIlluminationProbeNode::getSceneNodeBase.
static inline int pwrapGlobalIlluminationProbeNodegetSceneNodeBase(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoSceneNode, const_cast<SceneNode*>(&ret));

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

LuaUserDataTypeInfo luaUserDataTypeInfoSceneGraph = {
	4017435066089144029, "SceneGraph", LuaUserData::computeSizeForGarbageCollected<SceneGraph>(), nullptr, nullptr};

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

/// Pre-wrap method SceneGraph::newDirectionalLightNode.
static inline int pwrapSceneGraphnewDirectionalLightNode(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoDirectionalLightNode, const_cast<DirectionalLightNode*>(ret));

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

/// Pre-wrap method SceneGraph::newGpuParticleEmitterNode.
static inline int pwrapSceneGraphnewGpuParticleEmitterNode(lua_State* l)
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
	GpuParticleEmitterNode* ret = newSceneNode<GpuParticleEmitterNode>(self, arg0, arg1);

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
	ud->initPointed(&luaUserDataTypeInfoGpuParticleEmitterNode, const_cast<GpuParticleEmitterNode*>(ret));

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

/// Pre-wrap method SceneGraph::newGlobalIlluminationProbeNode.
static inline int pwrapSceneGraphnewGlobalIlluminationProbeNode(lua_State* l)
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
	ud->initPointed(&luaUserDataTypeInfoGlobalIlluminationProbeNode, const_cast<GlobalIlluminationProbeNode*>(ret));

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
	LuaBinder::pushLuaCFuncMethod(l, "newDirectionalLightNode", wrapSceneGraphnewDirectionalLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newStaticCollisionNode", wrapSceneGraphnewStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitterNode", wrapSceneGraphnewParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "newGpuParticleEmitterNode", wrapSceneGraphnewGpuParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProbeNode", wrapSceneGraphnewReflectionProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "newOccluderNode", wrapSceneGraphnewOccluderNode);
	LuaBinder::pushLuaCFuncMethod(l, "newDecalNode", wrapSceneGraphnewDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "newTriggerNode", wrapSceneGraphnewTriggerNode);
	LuaBinder::pushLuaCFuncMethod(l, "newGlobalIlluminationProbeNode", wrapSceneGraphnewGlobalIlluminationProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCameraNode", wrapSceneGraphsetActiveCameraNode);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo luaUserDataTypeInfoEvent = {8747813287410577862, "Event",
												LuaUserData::computeSizeForGarbageCollected<Event>(), nullptr, nullptr};

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
	8003708600719537087, "LightEvent", LuaUserData::computeSizeForGarbageCollected<LightEvent>(), nullptr, nullptr};

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

LuaUserDataTypeInfo luaUserDataTypeInfoEventManager = {-2344492732810723892, "EventManager",
													   LuaUserData::computeSizeForGarbageCollected<EventManager>(),
													   nullptr, nullptr};

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
	wrapFrustumComponent(l);
	wrapGlobalIlluminationProbeComponent(l);
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
	wrapOccluderNode(l);
	wrapDecalNode(l);
	wrapTriggerNode(l);
	wrapFogDensityNode(l);
	wrapGlobalIlluminationProbeNode(l);
	wrapSceneGraph(l);
	wrapEvent(l);
	wrapLightEvent(l);
	wrapEventManager(l);
	LuaBinder::pushLuaCFunc(l, "getSceneGraph", wrapgetSceneGraph);
	LuaBinder::pushLuaCFunc(l, "getEventManager", wrapgetEventManager);
}

} // end namespace anki
