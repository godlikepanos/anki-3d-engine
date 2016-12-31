// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

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
	UserData* ud;
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

/// Pre-wrap method LightComponent::setSpecularColor.
static inline int pwrapLightComponentsetSpecularColor(lua_State* l)
{
	UserData* ud;
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
	self->setSpecularColor(arg0);

	return 0;
}

/// Wrap method LightComponent::setSpecularColor.
static int wrapLightComponentsetSpecularColor(lua_State* l)
{
	int res = pwrapLightComponentsetSpecularColor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method LightComponent::getSpecularColor.
static inline int pwrapLightComponentgetSpecularColor(lua_State* l)
{
	UserData* ud;
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
	const Vec4& ret = self->getSpecularColor();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->initPointed(6804478823655046386, const_cast<Vec4*>(&ret));

	return 1;
}

/// Wrap method LightComponent::getSpecularColor.
static int wrapLightComponentgetSpecularColor(lua_State* l)
{
	int res = pwrapLightComponentgetSpecularColor(l);
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	LuaBinder::pushLuaCFuncMethod(l, "setSpecularColor", wrapLightComponentsetSpecularColor);
	LuaBinder::pushLuaCFuncMethod(l, "getSpecularColor", wrapLightComponentgetSpecularColor);
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
	UserData* ud;
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

/// Pre-wrap method DecalComponent::setNormalRoughnessDecal.
static inline int pwrapDecalComponentsetNormalRoughnessDecal(lua_State* l)
{
	UserData* ud;
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
	Error ret = self->setNormalRoughnessDecal(arg0, arg1, arg2);

	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}

	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method DecalComponent::setNormalRoughnessDecal.
static int wrapDecalComponentsetNormalRoughnessDecal(lua_State* l)
{
	int res = pwrapDecalComponentsetNormalRoughnessDecal(l);
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
	UserData* ud;
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
	LuaBinder::pushLuaCFuncMethod(l, "setNormalRoughnessDecal", wrapDecalComponentsetNormalRoughnessDecal);
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

static const char* classnamePerspectiveCamera = "PerspectiveCamera";

template<>
I64 LuaBinder::getWrappedTypeSignature<PerspectiveCamera>()
{
	return -4317960537256382878;
}

template<>
const char* LuaBinder::getWrappedTypeName<PerspectiveCamera>()
{
	return classnamePerspectiveCamera;
}

/// Pre-wrap method PerspectiveCamera::getSceneNodeBase.
static inline int pwrapPerspectiveCameragetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePerspectiveCamera, -4317960537256382878, ud))
	{
		return -1;
	}

	PerspectiveCamera* self = ud->getData<PerspectiveCamera>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method PerspectiveCamera::getSceneNodeBase.
static int wrapPerspectiveCameragetSceneNodeBase(lua_State* l)
{
	int res = pwrapPerspectiveCameragetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method PerspectiveCamera::setAll.
static inline int pwrapPerspectiveCamerasetAll(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 5);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePerspectiveCamera, -4317960537256382878, ud))
	{
		return -1;
	}

	PerspectiveCamera* self = ud->getData<PerspectiveCamera>();

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

/// Wrap method PerspectiveCamera::setAll.
static int wrapPerspectiveCamerasetAll(lua_State* l)
{
	int res = pwrapPerspectiveCamerasetAll(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class PerspectiveCamera.
static inline void wrapPerspectiveCamera(lua_State* l)
{
	LuaBinder::createClass(l, classnamePerspectiveCamera);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPerspectiveCameragetSceneNodeBase);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapPerspectiveCamerasetAll);
	lua_settop(l, 0);
}

static const char* classnamePointLight = "PointLight";

template<>
I64 LuaBinder::getWrappedTypeSignature<PointLight>()
{
	return 3561037663389896020;
}

template<>
const char* LuaBinder::getWrappedTypeName<PointLight>()
{
	return classnamePointLight;
}

/// Pre-wrap method PointLight::getSceneNodeBase.
static inline int pwrapPointLightgetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePointLight, 3561037663389896020, ud))
	{
		return -1;
	}

	PointLight* self = ud->getData<PointLight>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method PointLight::getSceneNodeBase.
static int wrapPointLightgetSceneNodeBase(lua_State* l)
{
	int res = pwrapPointLightgetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method PointLight::loadLensFlare.
static inline int pwrapPointLightloadLensFlare(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePointLight, 3561037663389896020, ud))
	{
		return -1;
	}

	PointLight* self = ud->getData<PointLight>();

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

/// Wrap method PointLight::loadLensFlare.
static int wrapPointLightloadLensFlare(lua_State* l)
{
	int res = pwrapPointLightloadLensFlare(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class PointLight.
static inline void wrapPointLight(lua_State* l)
{
	LuaBinder::createClass(l, classnamePointLight);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPointLightgetSceneNodeBase);
	LuaBinder::pushLuaCFuncMethod(l, "loadLensFlare", wrapPointLightloadLensFlare);
	lua_settop(l, 0);
}

static const char* classnameSpotLight = "SpotLight";

template<>
I64 LuaBinder::getWrappedTypeSignature<SpotLight>()
{
	return 7940385212889719421;
}

template<>
const char* LuaBinder::getWrappedTypeName<SpotLight>()
{
	return classnameSpotLight;
}

/// Pre-wrap method SpotLight::getSceneNodeBase.
static inline int pwrapSpotLightgetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSpotLight, 7940385212889719421, ud))
	{
		return -1;
	}

	SpotLight* self = ud->getData<SpotLight>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method SpotLight::getSceneNodeBase.
static int wrapSpotLightgetSceneNodeBase(lua_State* l)
{
	int res = pwrapSpotLightgetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class SpotLight.
static inline void wrapSpotLight(lua_State* l)
{
	LuaBinder::createClass(l, classnameSpotLight);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapSpotLightgetSceneNodeBase);
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

static const char* classnamePortal = "Portal";

template<>
I64 LuaBinder::getWrappedTypeSignature<Portal>()
{
	return 7450426072538297652;
}

template<>
const char* LuaBinder::getWrappedTypeName<Portal>()
{
	return classnamePortal;
}

/// Pre-wrap method Portal::getSceneNodeBase.
static inline int pwrapPortalgetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnamePortal, 7450426072538297652, ud))
	{
		return -1;
	}

	Portal* self = ud->getData<Portal>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method Portal::getSceneNodeBase.
static int wrapPortalgetSceneNodeBase(lua_State* l)
{
	int res = pwrapPortalgetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Portal.
static inline void wrapPortal(lua_State* l)
{
	LuaBinder::createClass(l, classnamePortal);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapPortalgetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameSector = "Sector";

template<>
I64 LuaBinder::getWrappedTypeSignature<Sector>()
{
	return 2371391302432627552;
}

template<>
const char* LuaBinder::getWrappedTypeName<Sector>()
{
	return classnameSector;
}

/// Pre-wrap method Sector::getSceneNodeBase.
static inline int pwrapSectorgetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameSector, 2371391302432627552, ud))
	{
		return -1;
	}

	Sector* self = ud->getData<Sector>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method Sector::getSceneNodeBase.
static int wrapSectorgetSceneNodeBase(lua_State* l)
{
	int res = pwrapSectorgetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Sector.
static inline void wrapSector(lua_State* l)
{
	LuaBinder::createClass(l, classnameSector);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapSectorgetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameParticleEmitter = "ParticleEmitter";

template<>
I64 LuaBinder::getWrappedTypeSignature<ParticleEmitter>()
{
	return -1716560948193631017;
}

template<>
const char* LuaBinder::getWrappedTypeName<ParticleEmitter>()
{
	return classnameParticleEmitter;
}

/// Pre-wrap method ParticleEmitter::getSceneNodeBase.
static inline int pwrapParticleEmittergetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameParticleEmitter, -1716560948193631017, ud))
	{
		return -1;
	}

	ParticleEmitter* self = ud->getData<ParticleEmitter>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method ParticleEmitter::getSceneNodeBase.
static int wrapParticleEmittergetSceneNodeBase(lua_State* l)
{
	int res = pwrapParticleEmittergetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ParticleEmitter.
static inline void wrapParticleEmitter(lua_State* l)
{
	LuaBinder::createClass(l, classnameParticleEmitter);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapParticleEmittergetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameReflectionProbe = "ReflectionProbe";

template<>
I64 LuaBinder::getWrappedTypeSignature<ReflectionProbe>()
{
	return 205427259474779436;
}

template<>
const char* LuaBinder::getWrappedTypeName<ReflectionProbe>()
{
	return classnameReflectionProbe;
}

/// Pre-wrap method ReflectionProbe::getSceneNodeBase.
static inline int pwrapReflectionProbegetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameReflectionProbe, 205427259474779436, ud))
	{
		return -1;
	}

	ReflectionProbe* self = ud->getData<ReflectionProbe>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method ReflectionProbe::getSceneNodeBase.
static int wrapReflectionProbegetSceneNodeBase(lua_State* l)
{
	int res = pwrapReflectionProbegetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ReflectionProbe.
static inline void wrapReflectionProbe(lua_State* l)
{
	LuaBinder::createClass(l, classnameReflectionProbe);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapReflectionProbegetSceneNodeBase);
	lua_settop(l, 0);
}

static const char* classnameReflectionProxy = "ReflectionProxy";

template<>
I64 LuaBinder::getWrappedTypeSignature<ReflectionProxy>()
{
	return 205427259496779646;
}

template<>
const char* LuaBinder::getWrappedTypeName<ReflectionProxy>()
{
	return classnameReflectionProxy;
}

/// Pre-wrap method ReflectionProxy::getSceneNodeBase.
static inline int pwrapReflectionProxygetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameReflectionProxy, 205427259496779646, ud))
	{
		return -1;
	}

	ReflectionProxy* self = ud->getData<ReflectionProxy>();

	// Call the method
	SceneNode& ret = *self;

	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->initPointed(-2220074417980276571, const_cast<SceneNode*>(&ret));

	return 1;
}

/// Wrap method ReflectionProxy::getSceneNodeBase.
static int wrapReflectionProxygetSceneNodeBase(lua_State* l)
{
	int res = pwrapReflectionProxygetSceneNodeBase(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class ReflectionProxy.
static inline void wrapReflectionProxy(lua_State* l)
{
	LuaBinder::createClass(l, classnameReflectionProxy);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapReflectionProxygetSceneNodeBase);
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

/// Pre-wrap method SceneGraph::newPerspectiveCamera.
static inline int pwrapSceneGraphnewPerspectiveCamera(lua_State* l)
{
	UserData* ud;
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
	PerspectiveCamera* ret = newSceneNode<PerspectiveCamera>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "PerspectiveCamera");
	ud->initPointed(-4317960537256382878, const_cast<PerspectiveCamera*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newPerspectiveCamera.
static int wrapSceneGraphnewPerspectiveCamera(lua_State* l)
{
	int res = pwrapSceneGraphnewPerspectiveCamera(l);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

/// Pre-wrap method SceneGraph::newPointLight.
static inline int pwrapSceneGraphnewPointLight(lua_State* l)
{
	UserData* ud;
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
	PointLight* ret = newSceneNode<PointLight>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "PointLight");
	ud->initPointed(3561037663389896020, const_cast<PointLight*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newPointLight.
static int wrapSceneGraphnewPointLight(lua_State* l)
{
	int res = pwrapSceneGraphnewPointLight(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newSpotLight.
static inline int pwrapSceneGraphnewSpotLight(lua_State* l)
{
	UserData* ud;
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
	SpotLight* ret = newSceneNode<SpotLight>(self, arg0);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SpotLight");
	ud->initPointed(7940385212889719421, const_cast<SpotLight*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newSpotLight.
static int wrapSceneGraphnewSpotLight(lua_State* l)
{
	int res = pwrapSceneGraphnewSpotLight(l);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

/// Pre-wrap method SceneGraph::newPortal.
static inline int pwrapSceneGraphnewPortal(lua_State* l)
{
	UserData* ud;
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
	Portal* ret = newSceneNode<Portal>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Portal");
	ud->initPointed(7450426072538297652, const_cast<Portal*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newPortal.
static int wrapSceneGraphnewPortal(lua_State* l)
{
	int res = pwrapSceneGraphnewPortal(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newSector.
static inline int pwrapSceneGraphnewSector(lua_State* l)
{
	UserData* ud;
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
	Sector* ret = newSceneNode<Sector>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Sector");
	ud->initPointed(2371391302432627552, const_cast<Sector*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newSector.
static int wrapSceneGraphnewSector(lua_State* l)
{
	int res = pwrapSceneGraphnewSector(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newParticleEmitter.
static inline int pwrapSceneGraphnewParticleEmitter(lua_State* l)
{
	UserData* ud;
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
	ParticleEmitter* ret = newSceneNode<ParticleEmitter>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitter");
	ud->initPointed(-1716560948193631017, const_cast<ParticleEmitter*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newParticleEmitter.
static int wrapSceneGraphnewParticleEmitter(lua_State* l)
{
	int res = pwrapSceneGraphnewParticleEmitter(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newReflectionProbe.
static inline int pwrapSceneGraphnewReflectionProbe(lua_State* l)
{
	UserData* ud;
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
	ReflectionProbe* ret = newSceneNode<ReflectionProbe>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbe");
	ud->initPointed(205427259474779436, const_cast<ReflectionProbe*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newReflectionProbe.
static int wrapSceneGraphnewReflectionProbe(lua_State* l)
{
	int res = pwrapSceneGraphnewReflectionProbe(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method SceneGraph::newReflectionProxy.
static inline int pwrapSceneGraphnewReflectionProxy(lua_State* l)
{
	UserData* ud;
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
	ReflectionProxy* ret = newSceneNode<ReflectionProxy>(self, arg0, arg1);

	// Push return value
	if(ANKI_UNLIKELY(ret == nullptr))
	{
		lua_pushstring(l, "Glue code returned nullptr");
		return -1;
	}

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProxy");
	ud->initPointed(205427259496779646, const_cast<ReflectionProxy*>(ret));

	return 1;
}

/// Wrap method SceneGraph::newReflectionProxy.
static int wrapSceneGraphnewReflectionProxy(lua_State* l)
{
	int res = pwrapSceneGraphnewReflectionProxy(l);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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

/// Pre-wrap method SceneGraph::setActiveCamera.
static inline int pwrapSceneGraphsetActiveCamera(lua_State* l)
{
	UserData* ud;
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
	self->setActiveCamera(arg0);

	return 0;
}

/// Wrap method SceneGraph::setActiveCamera.
static int wrapSceneGraphsetActiveCamera(lua_State* l)
{
	int res = pwrapSceneGraphsetActiveCamera(l);
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
	LuaBinder::pushLuaCFuncMethod(l, "newPerspectiveCamera", wrapSceneGraphnewPerspectiveCamera);
	LuaBinder::pushLuaCFuncMethod(l, "newModelNode", wrapSceneGraphnewModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "newPointLight", wrapSceneGraphnewPointLight);
	LuaBinder::pushLuaCFuncMethod(l, "newSpotLight", wrapSceneGraphnewSpotLight);
	LuaBinder::pushLuaCFuncMethod(l, "newStaticCollisionNode", wrapSceneGraphnewStaticCollisionNode);
	LuaBinder::pushLuaCFuncMethod(l, "newPortal", wrapSceneGraphnewPortal);
	LuaBinder::pushLuaCFuncMethod(l, "newSector", wrapSceneGraphnewSector);
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitter", wrapSceneGraphnewParticleEmitter);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProbe", wrapSceneGraphnewReflectionProbe);
	LuaBinder::pushLuaCFuncMethod(l, "newReflectionProxy", wrapSceneGraphnewReflectionProxy);
	LuaBinder::pushLuaCFuncMethod(l, "newOccluderNode", wrapSceneGraphnewOccluderNode);
	LuaBinder::pushLuaCFuncMethod(l, "newDecalNode", wrapSceneGraphnewDecalNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCamera", wrapSceneGraphsetActiveCamera);
	lua_settop(l, 0);
}

/// Pre-wrap function getSceneGraph.
static inline int pwrapgetSceneGraph(lua_State* l)
{
	UserData* ud;
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

	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = static_cast<UserData*>(voidp);
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
	wrapPerspectiveCamera(l);
	wrapPointLight(l);
	wrapSpotLight(l);
	wrapStaticCollisionNode(l);
	wrapPortal(l);
	wrapSector(l);
	wrapParticleEmitter(l);
	wrapReflectionProbe(l);
	wrapReflectionProxy(l);
	wrapOccluderNode(l);
	wrapDecalNode(l);
	wrapSceneGraph(l);
	LuaBinder::pushLuaCFunc(l, "getSceneGraph", wrapgetSceneGraph);
}

} // end namespace anki
