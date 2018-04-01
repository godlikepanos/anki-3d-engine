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
	Error err = scene->template newSceneNode<T>(name, ptr, args...);

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

	ScriptManager* scriptManager = reinterpret_cast<ScriptManager*>(binder->getParent());

	return &scriptManager->getSceneGraph();
}

static const char* classnameMoveComponent = "MoveComponent";

template<>
I64 LuaBinder::getWrappedTypeSignature<MoveComponent>()
{
	return 2038493110845313445;
}

template<>
const char* LuaBinder::getWrappedTypeName<MoveComponent>()
{
	return classnameMoveComponent;
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
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
	ud->initPointed(6804478823655046386, const_cast<Vec4*>(&ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Mat3x4", -2654194732934255869, ud))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
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
	ud->initPointed(-2654194732934255869, const_cast<Mat3x4*>(&ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
	{
		return -1;
	}

	MoveComponent* self = ud->getData<MoveComponent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Transform", 7048620195620777229, ud))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMoveComponent, 2038493110845313445, ud))
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
	ud->initPointed(7048620195620777229, const_cast<Transform*>(&ret));

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
	LuaBinder::createClass(l, classnameMoveComponent);
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

static const char* classnameLightComponent = "LightComponent";

template<>
I64 LuaBinder::getWrappedTypeSignature<LightComponent>()
{
	return 7940823622056993903;
}

template<>
const char* LuaBinder::getWrappedTypeName<LightComponent>()
{
	return classnameLightComponent;
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
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
	ud->initPointed(6804478823655046386, const_cast<Vec4*>(&ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
	{
		return -1;
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	Bool arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLightComponent, 7940823622056993903, ud))
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
	LuaBinder::createClass(l, classnameLightComponent);
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

static const char* classnameDecalComponent = "DecalComponent";

template<>
I64 LuaBinder::getWrappedTypeSignature<DecalComponent>()
{
	return -1979693900066114370;
}

template<>
const char* LuaBinder::getWrappedTypeName<DecalComponent>()
{
	return classnameDecalComponent;
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

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameDecalComponent, -1979693900066114370, ud))
	{
		return -1;
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
	{
		return -1;
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2))
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

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameDecalComponent, -1979693900066114370, ud))
	{
		return -1;
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
	{
		return -1;
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2))
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

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameDecalComponent, -1979693900066114370, ud))
	{
		return -1;
	}

	DecalComponent* self = ud->getData<DecalComponent>();

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

	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2))
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
	LuaBinder::createClass(l, classnameDecalComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setDiffuseDecal", wrapDecalComponentsetDiffuseDecal);
	LuaBinder::pushLuaCFuncMethod(l, "setSpecularRoughnessDecal", wrapDecalComponentsetSpecularRoughnessDecal);
	LuaBinder::pushLuaCFuncMethod(l, "updateShape", wrapDecalComponentupdateShape);
	lua_settop(l, 0);
}

static const char* classnameLensFlareComponent = "LensFlareComponent";

template<>
I64 LuaBinder::getWrappedTypeSignature<LensFlareComponent>()
{
	return -2019248835133422777;
}

template<>
const char* LuaBinder::getWrappedTypeName<LensFlareComponent>()
{
	return classnameLensFlareComponent;
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLensFlareComponent, -2019248835133422777, ud))
	{
		return -1;
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameLensFlareComponent, -2019248835133422777, ud))
	{
		return -1;
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
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
	LuaBinder::createClass(l, classnameLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setFirstFlareSize", wrapLensFlareComponentsetFirstFlareSize);
	LuaBinder::pushLuaCFuncMethod(l, "setColorMultiplier", wrapLensFlareComponentsetColorMultiplier);
	lua_settop(l, 0);
}

static const char* classnameSceneNode = "SceneNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<SceneNode>()
{
	return -2220074417980276571;
}

template<>
const char* LuaBinder::getWrappedTypeName<SceneNode>()
{
	return classnameSceneNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneNode, -2220074417980276571, ud))
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneNode, -2220074417980276571, ud))
	{
		return -1;
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "SceneNode", -2220074417980276571, ud))
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

/// Pre-wrap method SceneNode::tryGetComponent<MoveComponent>.
static inline int pwrapSceneNodegetMoveComponent(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneNode, -2220074417980276571, ud))
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
	ud->initPointed(2038493110845313445, const_cast<MoveComponent*>(ret));

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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneNode, -2220074417980276571, ud))
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
	ud->initPointed(7940823622056993903, const_cast<LightComponent*>(ret));

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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneNode, -2220074417980276571, ud))
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
	ud->initPointed(-2019248835133422777, const_cast<LensFlareComponent*>(ret));

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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneNode, -2220074417980276571, ud))
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
	ud->initPointed(-1979693900066114370, const_cast<DecalComponent*>(ret));

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

/// Wrap class SceneNode.
static inline void wrapSceneNode(lua_State* l)
{
	LuaBinder::createClass(l, classnameSceneNode);
	LuaBinder::pushLuaCFuncMethod(l, "getName", wrapSceneNodegetName);
	LuaBinder::pushLuaCFuncMethod(l, "addChild", wrapSceneNodeaddChild);
	LuaBinder::pushLuaCFuncMethod(l, "getMoveComponent", wrapSceneNodegetMoveComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getLightComponent", wrapSceneNodegetLightComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getLensFlareComponent", wrapSceneNodegetLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getDecalComponent", wrapSceneNodegetDecalComponent);
	lua_settop(l, 0);
}

static const char* classnameModelNode = "ModelNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<ModelNode>()
{
	return -1856316251880904290;
}

template<>
const char* LuaBinder::getWrappedTypeName<ModelNode>()
{
	return classnameModelNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameModelNode, -1856316251880904290, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapModelNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnamePerspectiveCameraNode = "PerspectiveCameraNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<PerspectiveCameraNode>()
{
	return -7590637754681648962;
}

template<>
const char* LuaBinder::getWrappedTypeName<PerspectiveCameraNode>()
{
	return classnamePerspectiveCameraNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePerspectiveCameraNode, -7590637754681648962, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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

	LuaBinder::checkArgsCount(l, 5);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePerspectiveCameraNode, -7590637754681648962, ud))
	{
		return -1;
	}

	PerspectiveCameraNode* self = ud->getData<PerspectiveCameraNode>();

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

	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2))
	{
		return -1;
	}

	F32 arg3;
	if(LuaBinder::checkNumber(l, 5, arg3))
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
	LuaBinder::createClass(l, classnamePerspectiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPerspectiveCameraNodegetSceneNodeBase);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapPerspectiveCameraNodesetAll);
	lua_settop(l, 0);
}

static const char* classnamePointLightNode = "PointLightNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<PointLightNode>()
{
	return 8507789763949195644;
}

template<>
const char* LuaBinder::getWrappedTypeName<PointLightNode>()
{
	return classnamePointLightNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePointLightNode, 8507789763949195644, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePointLightNode, 8507789763949195644, ud))
	{
		return -1;
	}

	PointLightNode* self = ud->getData<PointLightNode>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
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
	LuaBinder::createClass(l, classnamePointLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPointLightNodegetSceneNodeBase);
	LuaBinder::pushLuaCFuncMethod(l, "loadLensFlare", wrapPointLightNodeloadLensFlare);
	lua_settop(l, 0);
}

static const char* classnameSpotLightNode = "SpotLightNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<SpotLightNode>()
{
	return -9214759951813290587;
}

template<>
const char* LuaBinder::getWrappedTypeName<SpotLightNode>()
{
	return classnameSpotLightNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSpotLightNode, -9214759951813290587, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameSpotLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapSpotLightNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameStaticCollisionNode = "StaticCollisionNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<StaticCollisionNode>()
{
	return -4376619865753613291;
}

template<>
const char* LuaBinder::getWrappedTypeName<StaticCollisionNode>()
{
	return classnameStaticCollisionNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameStaticCollisionNode, -4376619865753613291, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapStaticCollisionNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnamePortalNode = "PortalNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<PortalNode>()
{
	return 8385999185171246748;
}

template<>
const char* LuaBinder::getWrappedTypeName<PortalNode>()
{
	return classnamePortalNode;
}

/// Pre-wrap method PortalNode::getSceneNodeBase.
static inline int pwrapPortalNodegetSceneNodeBase(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePortalNode, 8385999185171246748, ud))
	{
		return -1;
	}

	PortalNode* self = ud->getData<PortalNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method PortalNode::getSceneNodeBase.
static int wrapPortalNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapPortalNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class PortalNode.
static inline void wrapPortalNode(lua_State* l)
{
	LuaBinder::createClass(l, classnamePortalNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPortalNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameSectorNode = "SectorNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<SectorNode>()
{
	return 1288065288496288368;
}

template<>
const char* LuaBinder::getWrappedTypeName<SectorNode>()
{
	return classnameSectorNode;
}

/// Pre-wrap method SectorNode::getSceneNodeBase.
static inline int pwrapSectorNodegetSceneNodeBase(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSectorNode, 1288065288496288368, ud))
	{
		return -1;
	}

	SectorNode* self = ud->getData<SectorNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method SectorNode::getSceneNodeBase.
static int wrapSectorNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapSectorNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SectorNode.
static inline void wrapSectorNode(lua_State* l)
{
	LuaBinder::createClass(l, classnameSectorNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapSectorNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameParticleEmitterNode = "ParticleEmitterNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<ParticleEmitterNode>()
{
	return 4851204309813771919;
}

template<>
const char* LuaBinder::getWrappedTypeName<ParticleEmitterNode>()
{
	return classnameParticleEmitterNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameParticleEmitterNode, 4851204309813771919, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapParticleEmitterNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameReflectionProbeNode = "ReflectionProbeNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<ReflectionProbeNode>()
{
	return -801309373000950648;
}

template<>
const char* LuaBinder::getWrappedTypeName<ReflectionProbeNode>()
{
	return classnameReflectionProbeNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameReflectionProbeNode, -801309373000950648, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameReflectionProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapReflectionProbeNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameReflectionProxyNode = "ReflectionProxyNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<ReflectionProxyNode>()
{
	return 2307826176097073810;
}

template<>
const char* LuaBinder::getWrappedTypeName<ReflectionProxyNode>()
{
	return classnameReflectionProxyNode;
}

/// Pre-wrap method ReflectionProxyNode::getSceneNodeBase.
static inline int pwrapReflectionProxyNodegetSceneNodeBase(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameReflectionProxyNode, 2307826176097073810, ud))
	{
		return -1;
	}

	ReflectionProxyNode* self = ud->getData<ReflectionProxyNode>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method ReflectionProxyNode::getSceneNodeBase.
static int wrapReflectionProxyNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapReflectionProxyNodegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ReflectionProxyNode.
static inline void wrapReflectionProxyNode(lua_State* l)
{
	LuaBinder::createClass(l, classnameReflectionProxyNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapReflectionProxyNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameOccluderNode = "OccluderNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<OccluderNode>()
{
	return -6885028590097645115;
}

template<>
const char* LuaBinder::getWrappedTypeName<OccluderNode>()
{
	return classnameOccluderNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameOccluderNode, -6885028590097645115, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameOccluderNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapOccluderNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameDecalNode = "DecalNode";

template<>
I64 LuaBinder::getWrappedTypeSignature<DecalNode>()
{
	return 1097508121406753350;
}

template<>
const char* LuaBinder::getWrappedTypeName<DecalNode>()
{
	return classnameDecalNode;
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

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameDecalNode, 1097508121406753350, ud))
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
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

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
	LuaBinder::createClass(l, classnameDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapDecalNodegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameSceneGraph = "SceneGraph";

template<>
I64 LuaBinder::getWrappedTypeSignature<SceneGraph>()
{
	return -7754439619132389154;
}

template<>
const char* LuaBinder::getWrappedTypeName<SceneGraph>()
{
	return classnameSceneGraph;
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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
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
	ud->initPointed(-7590637754681648962, const_cast<PerspectiveCameraNode*>(ret));

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

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
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
	ud->initPointed(-1856316251880904290, const_cast<ModelNode*>(ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
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
	ud->initPointed(8507789763949195644, const_cast<PointLightNode*>(ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
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
	ud->initPointed(-9214759951813290587, const_cast<SpotLightNode*>(ret));

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

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
	{
		return -1;
	}

	if(LuaBinder::checkUserData(l, 4, "Transform", 7048620195620777229, ud))
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
	ud->initPointed(-4376619865753613291, const_cast<StaticCollisionNode*>(ret));

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

/// Pre-wrap method SceneGraph::newPortalNode.
static inline int pwrapSceneGraphnewPortalNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	PortalNode* ret = newSceneNode<PortalNode>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "PortalNode");
	ud->initPointed(8385999185171246748, const_cast<PortalNode*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newPortalNode.
static int wrapSceneGraphnewPortalNode(lua_State* l)
{
	int res = pwrapSceneGraphnewPortalNode(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newSectorNode.
static inline int pwrapSceneGraphnewSectorNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	SectorNode* ret = newSceneNode<SectorNode>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SectorNode");
	ud->initPointed(1288065288496288368, const_cast<SectorNode*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newSectorNode.
static int wrapSceneGraphnewSectorNode(lua_State* l)
{
	int res = pwrapSceneGraphnewSectorNode(l);
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

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
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
	ud->initPointed(4851204309813771919, const_cast<ParticleEmitterNode*>(ret));

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

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	ReflectionProbeNode* ret = newSceneNode<ReflectionProbeNode>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbeNode");
	ud->initPointed(-801309373000950648, const_cast<ReflectionProbeNode*>(ret));

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

/// Pre-wrap method SceneGraph::newReflectionProxyNode.
static inline int pwrapSceneGraphnewReflectionProxyNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	ReflectionProxyNode* ret = newSceneNode<ReflectionProxyNode>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProxyNode");
	ud->initPointed(2307826176097073810, const_cast<ReflectionProxyNode*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newReflectionProxyNode.
static int wrapSceneGraphnewReflectionProxyNode(lua_State* l)
{
	int res = pwrapSceneGraphnewReflectionProxyNode(l);
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

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
	{
		return -1;
	}

	const char* arg1;
	if(LuaBinder::checkString(l, 3, arg1))
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
	ud->initPointed(-6885028590097645115, const_cast<OccluderNode*>(ret));

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

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, 2, arg0))
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
	ud->initPointed(1097508121406753350, const_cast<DecalNode*>(ret));

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

/// Pre-wrap method SceneGraph::setActiveCameraNode.
static inline int pwrapSceneGraphsetActiveCameraNode(lua_State* l)
{
	LuaUserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSceneGraph, -7754439619132389154, ud))
	{
		return -1;
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "SceneNode", -2220074417980276571, ud))
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
	LuaBinder::createClass(l, classnameSceneGraph);
	LuaBinder::pushLuaCFuncMethod(l, "newPerspectiveCameraNode", wrapSceneGraphnewPerspectiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "newModelNode", wrapSceneGraphnewModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "newPointLightNode", wrapSceneGraphnewPointLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newSpotLightNode", wrapSceneGraphnewSpotLightNode);
	LuaBinder::pushLuaCFuncMethod(l, "newStaticCollisionNode", wrapSceneGraphnewStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "newPortalNode", wrapSceneGraphnewPortalNode);
	LuaBinder::pushLuaCFuncMethod(l, "newSectorNode", wrapSceneGraphnewSectorNode);
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitterNode", wrapSceneGraphnewParticleEmitterNode);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProbeNode", wrapSceneGraphnewReflectionProbeNode);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProxyNode", wrapSceneGraphnewReflectionProxyNode);
	LuaBinder::pushLuaCFuncMethod(l, "newOccluderNode", wrapSceneGraphnewOccluderNode);
	LuaBinder::pushLuaCFuncMethod(l, "newDecalNode", wrapSceneGraphnewDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCameraNode", wrapSceneGraphsetActiveCameraNode);
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

	LuaBinder::checkArgsCount(l, 0);

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
	ud->initPointed(-7754439619132389154, const_cast<SceneGraph*>(ret));

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

/// Wrap the module.
void wrapModuleScene(lua_State* l)
{
	wrapMoveComponent(l);
	wrapLightComponent(l);
	wrapDecalComponent(l);
	wrapLensFlareComponent(l);
	wrapSceneNode(l);
	wrapModelNode(l);
	wrapPerspectiveCameraNode(l);
	wrapPointLightNode(l);
	wrapSpotLightNode(l);
	wrapStaticCollisionNode(l);
	wrapPortalNode(l);
	wrapSectorNode(l);
	wrapParticleEmitterNode(l);
	wrapReflectionProbeNode(l);
	wrapReflectionProxyNode(l);
	wrapOccluderNode(l);
	wrapDecalNode(l);
	wrapSceneGraph(l);
	LuaBinder::pushLuaCFunc(l, "getSceneGraph", wrapgetSceneGraph);
}

} // end namespace anki
