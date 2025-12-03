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

LuaUserDataTypeInfo g_luaUserDataTypeInfoLightComponentType = {-5594775432067847771, "LightComponentType", 0, nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightComponentType>()
{
	return g_luaUserDataTypeInfoLightComponentType;
}

// Wrap enum LightComponentType.
static inline void wrapLightComponentType(lua_State* l)
{
	lua_newtable(l);
	lua_setglobal(l, g_luaUserDataTypeInfoLightComponentType.m_typeName);
	lua_getglobal(l, g_luaUserDataTypeInfoLightComponentType.m_typeName);

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

LuaUserDataTypeInfo g_luaUserDataTypeInfoBodyComponentCollisionShapeType = {5886730633021150151, "BodyComponentCollisionShapeType", 0, nullptr,
																			nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<BodyComponentCollisionShapeType>()
{
	return g_luaUserDataTypeInfoBodyComponentCollisionShapeType;
}

// Wrap enum BodyComponentCollisionShapeType.
static inline void wrapBodyComponentCollisionShapeType(lua_State* l)
{
	lua_newtable(l);
	lua_setglobal(l, g_luaUserDataTypeInfoBodyComponentCollisionShapeType.m_typeName);
	lua_getglobal(l, g_luaUserDataTypeInfoBodyComponentCollisionShapeType.m_typeName);

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

LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleGeometryType = {-1970280843451934837, "ParticleGeometryType", 0, nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ParticleGeometryType>()
{
	return g_luaUserDataTypeInfoParticleGeometryType;
}

// Wrap enum ParticleGeometryType.
static inline void wrapParticleGeometryType(lua_State* l)
{
	lua_newtable(l);
	lua_setglobal(l, g_luaUserDataTypeInfoParticleGeometryType.m_typeName);
	lua_getglobal(l, g_luaUserDataTypeInfoParticleGeometryType.m_typeName);

	lua_pushstring(l, "kQuad");
	ANKI_ASSERT(ParticleGeometryType(lua_Number(ParticleGeometryType::kQuad)) == ParticleGeometryType::kQuad
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(ParticleGeometryType::kQuad));
	lua_settable(l, -3);

	lua_pushstring(l, "kMeshComponent");
	ANKI_ASSERT(ParticleGeometryType(lua_Number(ParticleGeometryType::kMeshComponent)) == ParticleGeometryType::kMeshComponent
				&& "Can't map the enumerant to a lua_Number");
	lua_pushnumber(l, lua_Number(ParticleGeometryType::kMeshComponent));
	lua_settable(l, -3);

	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoWeakArraySceneNodePtr = {
	3153302682457725628, "WeakArraySceneNodePtr", LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<WeakArraySceneNodePtr>()
{
	return g_luaUserDataTypeInfoWeakArraySceneNodePtr;
}

// Wrap method WeakArraySceneNodePtr::getSize.
static inline int wrapWeakArraySceneNodePtrgetSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoWeakArraySceneNodePtr, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	WeakArraySceneNodePtr* self = ud->getData<WeakArraySceneNodePtr>();

	// Call the method
	U32 ret = self->getSize();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method WeakArraySceneNodePtr::getAt.
static inline int wrapWeakArraySceneNodePtrgetAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoWeakArraySceneNodePtr, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	WeakArraySceneNodePtr* self = ud->getData<WeakArraySceneNodePtr>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	SceneNode* ret = (*self)[arg0];

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	ud->initPointed(&g_luaUserDataTypeInfoSceneNode, ret);

	return 1;
}

// Wrap class WeakArraySceneNodePtr.
static inline void wrapWeakArraySceneNodePtr(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoWeakArraySceneNodePtr);
	LuaBinder::pushLuaCFuncMethod(l, "getSize", wrapWeakArraySceneNodePtrgetSize);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapWeakArraySceneNodePtrgetAt);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoLightComponent = {-6661654181898649904, "LightComponent",
														   LuaUserData::computeSizeForGarbageCollected<LightComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightComponent>()
{
	return g_luaUserDataTypeInfoLightComponent;
}

// Wrap method LightComponent::setLightComponentType.
static inline int wrapLightComponentsetLightComponentType(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	lua_Number arg0Tmp;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0Tmp)) [[unlikely]]
	{
		return lua_error(l);
	}
	const LightComponentType arg0 = LightComponentType(arg0Tmp);

	// Call the method
	self->setLightComponentType(arg0);

	return 0;
}

// Wrap method LightComponent::setDiffuseColor.
static inline int wrapLightComponentsetDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setDiffuseColor(arg0);

	return 0;
}

// Wrap method LightComponent::getDiffuseColor.
static inline int wrapLightComponentgetDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	const Vec4& ret = self->getDiffuseColor();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initPointed(&g_luaUserDataTypeInfoVec4, &ret);

	return 1;
}

// Wrap method LightComponent::setRadius.
static inline int wrapLightComponentsetRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setRadius(arg0);

	return 0;
}

// Wrap method LightComponent::getRadius.
static inline int wrapLightComponentgetRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getRadius();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method LightComponent::setDistance.
static inline int wrapLightComponentsetDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setDistance(arg0);

	return 0;
}

// Wrap method LightComponent::getDistance.
static inline int wrapLightComponentgetDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getDistance();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method LightComponent::setInnerAngle.
static inline int wrapLightComponentsetInnerAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setInnerAngle(arg0);

	return 0;
}

// Wrap method LightComponent::getInnerAngle.
static inline int wrapLightComponentgetInnerAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getInnerAngle();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method LightComponent::setOuterAngle.
static inline int wrapLightComponentsetOuterAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setOuterAngle(arg0);

	return 0;
}

// Wrap method LightComponent::getOuterAngle.
static inline int wrapLightComponentgetOuterAngle(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	F32 ret = self->getOuterAngle();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method LightComponent::setShadowEnabled.
static inline int wrapLightComponentsetShadowEnabled(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Pop arguments
	Bool arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setShadowEnabled(arg0);

	return 0;
}

// Wrap method LightComponent::getShadowEnabled.
static inline int wrapLightComponentgetShadowEnabled(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightComponent* self = ud->getData<LightComponent>();

	// Call the method
	Bool ret = self->getShadowEnabled();

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

// Wrap class LightComponent.
static inline void wrapLightComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoLightComponent);
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

LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent = {-5520034723432182506, "DecalComponent",
														   LuaUserData::computeSizeForGarbageCollected<DecalComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<DecalComponent>()
{
	return g_luaUserDataTypeInfoDecalComponent;
}

// Wrap method DecalComponent::setDiffuseImageFilename.
static inline int wrapDecalComponentsetDiffuseImageFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	DecalComponent& ret = self->setDiffuseImageFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&g_luaUserDataTypeInfoDecalComponent, &ret);

	return 1;
}

// Wrap method DecalComponent::getDiffuseImageFilename.
static inline int wrapDecalComponentgetDiffuseImageFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Call the method
	CString ret = self->getDiffuseImageFilename();

	// Push return value
	lua_pushstring(l, &ret[0]);

	return 1;
}

// Wrap method DecalComponent::setDiffuseBlendFactor.
static inline int wrapDecalComponentsetDiffuseBlendFactor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	DecalComponent& ret = self->setDiffuseBlendFactor(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&g_luaUserDataTypeInfoDecalComponent, &ret);

	return 1;
}

// Wrap method DecalComponent::getDiffuseBlendFactor.
static inline int wrapDecalComponentgetDiffuseBlendFactor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Call the method
	F32 ret = self->getDiffuseBlendFactor();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method DecalComponent::setRoughnessMetalnessImageFilename.
static inline int wrapDecalComponentsetRoughnessMetalnessImageFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	DecalComponent& ret = self->setRoughnessMetalnessImageFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&g_luaUserDataTypeInfoDecalComponent, &ret);

	return 1;
}

// Wrap method DecalComponent::getRoughnessMetalnessImageFilename.
static inline int wrapDecalComponentgetRoughnessMetalnessImageFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Call the method
	CString ret = self->getRoughnessMetalnessImageFilename();

	// Push return value
	lua_pushstring(l, &ret[0]);

	return 1;
}

// Wrap method DecalComponent::setRoughnessMetalnessBlendFactor.
static inline int wrapDecalComponentsetRoughnessMetalnessBlendFactor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	DecalComponent& ret = self->setRoughnessMetalnessBlendFactor(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&g_luaUserDataTypeInfoDecalComponent, &ret);

	return 1;
}

// Wrap method DecalComponent::getRoughnessMetalnessBlendFactor.
static inline int wrapDecalComponentgetRoughnessMetalnessBlendFactor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoDecalComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	DecalComponent* self = ud->getData<DecalComponent>();

	// Call the method
	F32 ret = self->getRoughnessMetalnessBlendFactor();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class DecalComponent.
static inline void wrapDecalComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoDecalComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setDiffuseImageFilename", wrapDecalComponentsetDiffuseImageFilename);
	LuaBinder::pushLuaCFuncMethod(l, "getDiffuseImageFilename", wrapDecalComponentgetDiffuseImageFilename);
	LuaBinder::pushLuaCFuncMethod(l, "setDiffuseBlendFactor", wrapDecalComponentsetDiffuseBlendFactor);
	LuaBinder::pushLuaCFuncMethod(l, "getDiffuseBlendFactor", wrapDecalComponentgetDiffuseBlendFactor);
	LuaBinder::pushLuaCFuncMethod(l, "setRoughnessMetalnessImageFilename", wrapDecalComponentsetRoughnessMetalnessImageFilename);
	LuaBinder::pushLuaCFuncMethod(l, "getRoughnessMetalnessImageFilename", wrapDecalComponentgetRoughnessMetalnessImageFilename);
	LuaBinder::pushLuaCFuncMethod(l, "setRoughnessMetalnessBlendFactor", wrapDecalComponentsetRoughnessMetalnessBlendFactor);
	LuaBinder::pushLuaCFuncMethod(l, "getRoughnessMetalnessBlendFactor", wrapDecalComponentgetRoughnessMetalnessBlendFactor);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoLensFlareComponent = {1436135364773864160, "LensFlareComponent",
															   LuaUserData::computeSizeForGarbageCollected<LensFlareComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LensFlareComponent>()
{
	return g_luaUserDataTypeInfoLensFlareComponent;
}

// Wrap method LensFlareComponent::loadImageResource.
static inline int wrapLensFlareComponentloadImageResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLensFlareComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->loadImageResource(arg0);

	return 0;
}

// Wrap method LensFlareComponent::setFirstFlareSize.
static inline int wrapLensFlareComponentsetFirstFlareSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLensFlareComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	self->setFirstFlareSize(arg0);

	return 0;
}

// Wrap method LensFlareComponent::setColorMultiplier.
static inline int wrapLensFlareComponentsetColorMultiplier(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLensFlareComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LensFlareComponent* self = ud->getData<LensFlareComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setColorMultiplier(arg0);

	return 0;
}

// Wrap class LensFlareComponent.
static inline void wrapLensFlareComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoLensFlareComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadImageResource", wrapLensFlareComponentloadImageResource);
	LuaBinder::pushLuaCFuncMethod(l, "setFirstFlareSize", wrapLensFlareComponentsetFirstFlareSize);
	LuaBinder::pushLuaCFuncMethod(l, "setColorMultiplier", wrapLensFlareComponentsetColorMultiplier);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoBodyComponent = {4859266942639207558, "BodyComponent",
														  LuaUserData::computeSizeForGarbageCollected<BodyComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<BodyComponent>()
{
	return g_luaUserDataTypeInfoBodyComponent;
}

// Wrap method BodyComponent::setCollisionShapeType.
static inline int wrapBodyComponentsetCollisionShapeType(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoBodyComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	lua_Number arg0Tmp;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0Tmp)) [[unlikely]]
	{
		return lua_error(l);
	}
	const BodyComponentCollisionShapeType arg0 = BodyComponentCollisionShapeType(arg0Tmp);

	// Call the method
	self->setCollisionShapeType(arg0);

	return 0;
}

// Wrap method BodyComponent::setBoxExtend.
static inline int wrapBodyComponentsetBoxExtend(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoBodyComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setBoxExtend(arg0);

	return 0;
}

// Wrap method BodyComponent::getBoxExtend.
static inline int wrapBodyComponentgetBoxExtend(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoBodyComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Call the method
	const Vec3& ret = self->getBoxExtend();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	ud->initPointed(&g_luaUserDataTypeInfoVec3, &ret);

	return 1;
}

// Wrap method BodyComponent::setSphereRadius.
static inline int wrapBodyComponentsetSphereRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoBodyComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setSphereRadius(arg0);

	return 0;
}

// Wrap method BodyComponent::getSphereRadius.
static inline int wrapBodyComponentgetSphereRadius(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoBodyComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	BodyComponent* self = ud->getData<BodyComponent>();

	// Call the method
	F32 ret = self->getSphereRadius();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class BodyComponent.
static inline void wrapBodyComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoBodyComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setCollisionShapeType", wrapBodyComponentsetCollisionShapeType);
	LuaBinder::pushLuaCFuncMethod(l, "setBoxExtend", wrapBodyComponentsetBoxExtend);
	LuaBinder::pushLuaCFuncMethod(l, "getBoxExtend", wrapBodyComponentgetBoxExtend);
	LuaBinder::pushLuaCFuncMethod(l, "setSphereRadius", wrapBodyComponentsetSphereRadius);
	LuaBinder::pushLuaCFuncMethod(l, "getSphereRadius", wrapBodyComponentgetSphereRadius);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoTriggerComponent = {-8532250760246815794, "TriggerComponent",
															 LuaUserData::computeSizeForGarbageCollected<TriggerComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<TriggerComponent>()
{
	return g_luaUserDataTypeInfoTriggerComponent;
}

// Wrap method TriggerComponent::getSceneNodesEnter.
static inline int wrapTriggerComponentgetSceneNodesEnter(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTriggerComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	TriggerComponent* self = ud->getData<TriggerComponent>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getSceneNodesEnter();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArraySceneNodePtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoWeakArraySceneNodePtr;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoWeakArraySceneNodePtr);
	::new(ud->getData<WeakArraySceneNodePtr>()) WeakArraySceneNodePtr(std::move(ret));

	return 1;
}

// Wrap method TriggerComponent::getSceneNodesExit.
static inline int wrapTriggerComponentgetSceneNodesExit(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTriggerComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	TriggerComponent* self = ud->getData<TriggerComponent>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getSceneNodesExit();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArraySceneNodePtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoWeakArraySceneNodePtr;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoWeakArraySceneNodePtr);
	::new(ud->getData<WeakArraySceneNodePtr>()) WeakArraySceneNodePtr(std::move(ret));

	return 1;
}

// Wrap class TriggerComponent.
static inline void wrapTriggerComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoTriggerComponent);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodesEnter", wrapTriggerComponentgetSceneNodesEnter);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodesExit", wrapTriggerComponentgetSceneNodesExit);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoFogDensityComponent = {-6273562933061222924, "FogDensityComponent",
																LuaUserData::computeSizeForGarbageCollected<FogDensityComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<FogDensityComponent>()
{
	return g_luaUserDataTypeInfoFogDensityComponent;
}

// Wrap method FogDensityComponent::setDensity.
static inline int wrapFogDensityComponentsetDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoFogDensityComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	FogDensityComponent* self = ud->getData<FogDensityComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setDensity(arg0);

	return 0;
}

// Wrap method FogDensityComponent::getDensity.
static inline int wrapFogDensityComponentgetDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoFogDensityComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	FogDensityComponent* self = ud->getData<FogDensityComponent>();

	// Call the method
	F32 ret = self->getDensity();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class FogDensityComponent.
static inline void wrapFogDensityComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoFogDensityComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setDensity", wrapFogDensityComponentsetDensity);
	LuaBinder::pushLuaCFuncMethod(l, "getDensity", wrapFogDensityComponentgetDensity);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoCameraComponent = {5614357661140762094, "CameraComponent",
															LuaUserData::computeSizeForGarbageCollected<CameraComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<CameraComponent>()
{
	return g_luaUserDataTypeInfoCameraComponent;
}

// Wrap method CameraComponent::setPerspective.
static inline int wrapCameraComponentsetPerspective(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 5)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoCameraComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	CameraComponent* self = ud->getData<CameraComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, arg2)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg3;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 5, arg3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setPerspective(arg0, arg1, arg2, arg3);

	return 0;
}

// Wrap class CameraComponent.
static inline void wrapCameraComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoCameraComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setPerspective", wrapCameraComponentsetPerspective);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoGlobalIlluminationProbeComponent = {
	-3401910529701313771, "GlobalIlluminationProbeComponent", LuaUserData::computeSizeForGarbageCollected<GlobalIlluminationProbeComponent>(),
	nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<GlobalIlluminationProbeComponent>()
{
	return g_luaUserDataTypeInfoGlobalIlluminationProbeComponent;
}

// Wrap method GlobalIlluminationProbeComponent::setCellSize.
static inline int wrapGlobalIlluminationProbeComponentsetCellSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setCellSize(arg0);

	return 0;
}

// Wrap method GlobalIlluminationProbeComponent::getCellSize.
static inline int wrapGlobalIlluminationProbeComponentgetCellSize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Call the method
	F32 ret = self->getCellSize();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method GlobalIlluminationProbeComponent::setFadeDistance.
static inline int wrapGlobalIlluminationProbeComponentsetFadeDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setFadeDistance(arg0);

	return 0;
}

// Wrap method GlobalIlluminationProbeComponent::getFadeDistance.
static inline int wrapGlobalIlluminationProbeComponentgetFadeDistance(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoGlobalIlluminationProbeComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	GlobalIlluminationProbeComponent* self = ud->getData<GlobalIlluminationProbeComponent>();

	// Call the method
	F32 ret = self->getFadeDistance();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class GlobalIlluminationProbeComponent.
static inline void wrapGlobalIlluminationProbeComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoGlobalIlluminationProbeComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setCellSize", wrapGlobalIlluminationProbeComponentsetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "getCellSize", wrapGlobalIlluminationProbeComponentgetCellSize);
	LuaBinder::pushLuaCFuncMethod(l, "setFadeDistance", wrapGlobalIlluminationProbeComponentsetFadeDistance);
	LuaBinder::pushLuaCFuncMethod(l, "getFadeDistance", wrapGlobalIlluminationProbeComponentgetFadeDistance);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoReflectionProbeComponent = {
	1875218110388114974, "ReflectionProbeComponent", LuaUserData::computeSizeForGarbageCollected<ReflectionProbeComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ReflectionProbeComponent>()
{
	return g_luaUserDataTypeInfoReflectionProbeComponent;
}

// Wrap class ReflectionProbeComponent.
static inline void wrapReflectionProbeComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoReflectionProbeComponent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitterComponent = {
	2925717460779801052, "ParticleEmitterComponent", LuaUserData::computeSizeForGarbageCollected<ParticleEmitterComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ParticleEmitterComponent>()
{
	return g_luaUserDataTypeInfoParticleEmitterComponent;
}

// Wrap method ParticleEmitterComponent::loadParticleEmitterResource.
static inline int wrapParticleEmitterComponentloadParticleEmitterResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoParticleEmitterComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	ParticleEmitterComponent* self = ud->getData<ParticleEmitterComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->loadParticleEmitterResource(arg0);

	return 0;
}

// Wrap class ParticleEmitterComponent.
static inline void wrapParticleEmitterComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoParticleEmitterComponent);
	LuaBinder::pushLuaCFuncMethod(l, "loadParticleEmitterResource", wrapParticleEmitterComponentloadParticleEmitterResource);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitter2Component = {
	5715853988873556419, "ParticleEmitter2Component", LuaUserData::computeSizeForGarbageCollected<ParticleEmitter2Component>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ParticleEmitter2Component>()
{
	return g_luaUserDataTypeInfoParticleEmitter2Component;
}

// Wrap method ParticleEmitter2Component::setParticleEmitterFilename.
static inline int wrapParticleEmitter2ComponentsetParticleEmitterFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoParticleEmitter2Component, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	ParticleEmitter2Component* self = ud->getData<ParticleEmitter2Component>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	ParticleEmitter2Component& ret = self->setParticleEmitterFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitter2Component");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitter2Component;
	ud->initPointed(&g_luaUserDataTypeInfoParticleEmitter2Component, &ret);

	return 1;
}

// Wrap method ParticleEmitter2Component::getParticleEmitterFilename.
static inline int wrapParticleEmitter2ComponentgetParticleEmitterFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoParticleEmitter2Component, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	ParticleEmitter2Component* self = ud->getData<ParticleEmitter2Component>();

	// Call the method
	CString ret = self->getParticleEmitterFilename();

	// Push return value
	lua_pushstring(l, &ret[0]);

	return 1;
}

// Wrap method ParticleEmitter2Component::setParticleGeometryType.
static inline int wrapParticleEmitter2ComponentsetParticleGeometryType(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoParticleEmitter2Component, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	ParticleEmitter2Component* self = ud->getData<ParticleEmitter2Component>();

	// Pop arguments
	lua_Number arg0Tmp;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0Tmp)) [[unlikely]]
	{
		return lua_error(l);
	}
	const ParticleGeometryType arg0 = ParticleGeometryType(arg0Tmp);

	// Call the method
	ParticleEmitter2Component& ret = self->setParticleGeometryType(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitter2Component");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitter2Component;
	ud->initPointed(&g_luaUserDataTypeInfoParticleEmitter2Component, &ret);

	return 1;
}

// Wrap method ParticleEmitter2Component::getParticleGeometryType.
static inline int wrapParticleEmitter2ComponentgetParticleGeometryType(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoParticleEmitter2Component, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	ParticleEmitter2Component* self = ud->getData<ParticleEmitter2Component>();

	// Call the method
	ParticleGeometryType ret = self->getParticleGeometryType();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<ParticleGeometryType>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "ParticleGeometryType");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleGeometryType;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoParticleGeometryType);
	::new(ud->getData<ParticleGeometryType>()) ParticleGeometryType(std::move(ret));

	return 1;
}

// Wrap class ParticleEmitter2Component.
static inline void wrapParticleEmitter2Component(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoParticleEmitter2Component);
	LuaBinder::pushLuaCFuncMethod(l, "setParticleEmitterFilename", wrapParticleEmitter2ComponentsetParticleEmitterFilename);
	LuaBinder::pushLuaCFuncMethod(l, "getParticleEmitterFilename", wrapParticleEmitter2ComponentgetParticleEmitterFilename);
	LuaBinder::pushLuaCFuncMethod(l, "setParticleGeometryType", wrapParticleEmitter2ComponentsetParticleGeometryType);
	LuaBinder::pushLuaCFuncMethod(l, "getParticleGeometryType", wrapParticleEmitter2ComponentgetParticleGeometryType);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoMeshComponent = {7129233230963760247, "MeshComponent",
														  LuaUserData::computeSizeForGarbageCollected<MeshComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MeshComponent>()
{
	return g_luaUserDataTypeInfoMeshComponent;
}

// Wrap method MeshComponent::setMeshFilename.
static inline int wrapMeshComponentsetMeshFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMeshComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	MeshComponent* self = ud->getData<MeshComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	MeshComponent& ret = self->setMeshFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MeshComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMeshComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMeshComponent, &ret);

	return 1;
}

// Wrap class MeshComponent.
static inline void wrapMeshComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoMeshComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setMeshFilename", wrapMeshComponentsetMeshFilename);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoMaterialComponent = {-5240416883337639304, "MaterialComponent",
															  LuaUserData::computeSizeForGarbageCollected<MaterialComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<MaterialComponent>()
{
	return g_luaUserDataTypeInfoMaterialComponent;
}

// Wrap method MaterialComponent::setMaterialFilename.
static inline int wrapMaterialComponentsetMaterialFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMaterialComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	MaterialComponent* self = ud->getData<MaterialComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	MaterialComponent& ret = self->setMaterialFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMaterialComponent, &ret);

	return 1;
}

// Wrap method MaterialComponent::setSubmeshIndex.
static inline int wrapMaterialComponentsetSubmeshIndex(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMaterialComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	MaterialComponent* self = ud->getData<MaterialComponent>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	MaterialComponent& ret = self->setSubmeshIndex(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMaterialComponent, &ret);

	return 1;
}

// Wrap class MaterialComponent.
static inline void wrapMaterialComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoMaterialComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setMaterialFilename", wrapMaterialComponentsetMaterialFilename);
	LuaBinder::pushLuaCFuncMethod(l, "setSubmeshIndex", wrapMaterialComponentsetSubmeshIndex);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoSkinComponent = {3016724804384428667, "SkinComponent",
														  LuaUserData::computeSizeForGarbageCollected<SkinComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SkinComponent>()
{
	return g_luaUserDataTypeInfoSkinComponent;
}

// Wrap method SkinComponent::setSkeletonFilename.
static inline int wrapSkinComponentsetSkeletonFilename(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkinComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkinComponent* self = ud->getData<SkinComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	SkinComponent& ret = self->setSkeletonFilename(arg0);

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkinComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSkinComponent;
	ud->initPointed(&g_luaUserDataTypeInfoSkinComponent, &ret);

	return 1;
}

// Wrap class SkinComponent.
static inline void wrapSkinComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoSkinComponent);
	LuaBinder::pushLuaCFuncMethod(l, "setSkeletonFilename", wrapSkinComponentsetSkeletonFilename);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoSkyboxComponent = {-3538427196989067145, "SkyboxComponent",
															LuaUserData::computeSizeForGarbageCollected<SkyboxComponent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SkyboxComponent>()
{
	return g_luaUserDataTypeInfoSkyboxComponent;
}

// Wrap method SkyboxComponent::setSolidColor.
static inline int wrapSkyboxComponentsetSolidColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setSolidColor(arg0);

	return 0;
}

// Wrap method SkyboxComponent::loadImageResource.
static inline int wrapSkyboxComponentloadImageResource(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->loadImageResource(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setGeneratedSky.
static inline int wrapSkyboxComponentsetGeneratedSky(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Call the method
	self->setGeneratedSky();

	return 0;
}

// Wrap method SkyboxComponent::setMinFogDensity.
static inline int wrapSkyboxComponentsetMinFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setMinFogDensity(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setMaxFogDensity.
static inline int wrapSkyboxComponentsetMaxFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setMaxFogDensity(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setHeightOfMinFogDensity.
static inline int wrapSkyboxComponentsetHeightOfMinFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setHeightOfMinFogDensity(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setHeightOfMaxFogDensity.
static inline int wrapSkyboxComponentsetHeightOfMaxFogDensity(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setHeightOfMaxFogDensity(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setFogDiffuseColor.
static inline int wrapSkyboxComponentsetFogDiffuseColor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setFogDiffuseColor(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setImageBias.
static inline int wrapSkyboxComponentsetImageBias(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setImageBias(arg0);

	return 0;
}

// Wrap method SkyboxComponent::setImageScale.
static inline int wrapSkyboxComponentsetImageScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSkyboxComponent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SkyboxComponent* self = ud->getData<SkyboxComponent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	// Call the method
	self->setImageScale(arg0);

	return 0;
}

// Wrap class SkyboxComponent.
static inline void wrapSkyboxComponent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoSkyboxComponent);
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

LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode = {-1325538657283171084, "SceneNode", LuaUserData::computeSizeForGarbageCollected<SceneNode>(),
													  nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneNode>()
{
	return g_luaUserDataTypeInfoSceneNode;
}

// Wrap method SceneNode::getName.
static inline int wrapSceneNodegetName(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	CString ret = self->getName();

	// Push return value
	lua_pushstring(l, &ret[0]);

	return 1;
}

// Wrap method SceneNode::addChild.
static inline int wrapSceneNodeaddChild(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* iarg0 = ud->getData<SceneNode>();
	SceneNode* arg0(iarg0);

	// Call the method
	self->addChild(arg0);

	return 0;
}

// Wrap method SceneNode::markForDeletion.
static inline int wrapSceneNodemarkForDeletion(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	self->markForDeletion();

	return 0;
}

// Wrap method SceneNode::setLocalOrigin.
static inline int wrapSceneNodesetLocalOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setLocalOrigin(arg0);

	return 0;
}

// Wrap method SceneNode::getLocalOrigin.
static inline int wrapSceneNodegetLocalOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	Vec3 ret = self->getLocalOrigin();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

// Wrap method SceneNode::setLocalRotation.
static inline int wrapSceneNodesetLocalRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* iarg0 = ud->getData<Mat3>();
	const Mat3& arg0(*iarg0);

	// Call the method
	self->setLocalRotation(arg0);

	return 0;
}

// Wrap method SceneNode::getLocalRotation.
static inline int wrapSceneNodegetLocalRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	Mat3 ret = self->getLocalRotation();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Mat3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Mat3");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoMat3);
	::new(ud->getData<Mat3>()) Mat3(std::move(ret));

	return 1;
}

// Wrap method SceneNode::setLocalScale.
static inline int wrapSceneNodesetLocalScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setLocalScale(arg0);

	return 0;
}

// Wrap method SceneNode::getLocalScale.
static inline int wrapSceneNodegetLocalScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	Vec3 ret = self->getLocalScale();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

// Wrap method SceneNode::setLocalTransform.
static inline int wrapSceneNodesetLocalTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTransform;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* iarg0 = ud->getData<Transform>();
	const Transform& arg0(*iarg0);

	// Call the method
	self->setLocalTransform(arg0);

	return 0;
}

// Wrap method SceneNode::getLocalTransform.
static inline int wrapSceneNodegetLocalTransform(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	const Transform& ret = self->getLocalTransform();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "Transform");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTransform;
	ud->initPointed(&g_luaUserDataTypeInfoTransform, &ret);

	return 1;
}

// Wrap method SceneNode::newComponent<LightComponent>.
static inline int wrapSceneNodenewLightComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LightComponent* ret = self->newComponent<LightComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LightComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoLightComponent;
	ud->initPointed(&g_luaUserDataTypeInfoLightComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<LensFlareComponent>.
static inline int wrapSceneNodenewLensFlareComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LensFlareComponent* ret = self->newComponent<LensFlareComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LensFlareComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoLensFlareComponent;
	ud->initPointed(&g_luaUserDataTypeInfoLensFlareComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<DecalComponent>.
static inline int wrapSceneNodenewDecalComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	DecalComponent* ret = self->newComponent<DecalComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&g_luaUserDataTypeInfoDecalComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<TriggerComponent>.
static inline int wrapSceneNodenewTriggerComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	TriggerComponent* ret = self->newComponent<TriggerComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "TriggerComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTriggerComponent;
	ud->initPointed(&g_luaUserDataTypeInfoTriggerComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<FogDensityComponent>.
static inline int wrapSceneNodenewFogDensityComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	FogDensityComponent* ret = self->newComponent<FogDensityComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "FogDensityComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoFogDensityComponent;
	ud->initPointed(&g_luaUserDataTypeInfoFogDensityComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<CameraComponent>.
static inline int wrapSceneNodenewCameraComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	CameraComponent* ret = self->newComponent<CameraComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "CameraComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoCameraComponent;
	ud->initPointed(&g_luaUserDataTypeInfoCameraComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<GlobalIlluminationProbeComponent>.
static inline int wrapSceneNodenewGlobalIlluminationProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	GlobalIlluminationProbeComponent* ret = self->newComponent<GlobalIlluminationProbeComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GlobalIlluminationProbeComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoGlobalIlluminationProbeComponent;
	ud->initPointed(&g_luaUserDataTypeInfoGlobalIlluminationProbeComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<ReflectionProbeComponent>.
static inline int wrapSceneNodenewReflectionProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ReflectionProbeComponent* ret = self->newComponent<ReflectionProbeComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbeComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoReflectionProbeComponent;
	ud->initPointed(&g_luaUserDataTypeInfoReflectionProbeComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<BodyComponent>.
static inline int wrapSceneNodenewBodyComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	BodyComponent* ret = self->newComponent<BodyComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "BodyComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoBodyComponent;
	ud->initPointed(&g_luaUserDataTypeInfoBodyComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<ParticleEmitterComponent>.
static inline int wrapSceneNodenewParticleEmitterComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ParticleEmitterComponent* ret = self->newComponent<ParticleEmitterComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitterComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitterComponent;
	ud->initPointed(&g_luaUserDataTypeInfoParticleEmitterComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<ParticleEmitter2Component>.
static inline int wrapSceneNodenewParticleEmitter2Component(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ParticleEmitter2Component* ret = self->newComponent<ParticleEmitter2Component>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitter2Component");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitter2Component;
	ud->initPointed(&g_luaUserDataTypeInfoParticleEmitter2Component, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<MeshComponent>.
static inline int wrapSceneNodenewMeshComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MeshComponent* ret = self->newComponent<MeshComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MeshComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMeshComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMeshComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<MaterialComponent>.
static inline int wrapSceneNodenewMaterialComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MaterialComponent* ret = self->newComponent<MaterialComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMaterialComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<SkinComponent>.
static inline int wrapSceneNodenewSkinComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkinComponent* ret = self->newComponent<SkinComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkinComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSkinComponent;
	ud->initPointed(&g_luaUserDataTypeInfoSkinComponent, ret);

	return 1;
}

// Wrap method SceneNode::newComponent<SkyboxComponent>.
static inline int wrapSceneNodenewSkyboxComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkyboxComponent* ret = self->newComponent<SkyboxComponent>();

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkyboxComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSkyboxComponent;
	ud->initPointed(&g_luaUserDataTypeInfoSkyboxComponent, ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<LightComponent>.
static inline int wrapSceneNodegetFirstLightComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LightComponent& ret = self->getFirstComponentOfType<LightComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LightComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoLightComponent;
	ud->initPointed(&g_luaUserDataTypeInfoLightComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<LensFlareComponent>.
static inline int wrapSceneNodegetFirstLensFlareComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	LensFlareComponent& ret = self->getFirstComponentOfType<LensFlareComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LensFlareComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoLensFlareComponent;
	ud->initPointed(&g_luaUserDataTypeInfoLensFlareComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<DecalComponent>.
static inline int wrapSceneNodegetFirstDecalComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	DecalComponent& ret = self->getFirstComponentOfType<DecalComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "DecalComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoDecalComponent;
	ud->initPointed(&g_luaUserDataTypeInfoDecalComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<TriggerComponent>.
static inline int wrapSceneNodegetFirstTriggerComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	TriggerComponent& ret = self->getFirstComponentOfType<TriggerComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "TriggerComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTriggerComponent;
	ud->initPointed(&g_luaUserDataTypeInfoTriggerComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<FogDensityComponent>.
static inline int wrapSceneNodegetFirstFogDensityComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	FogDensityComponent& ret = self->getFirstComponentOfType<FogDensityComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "FogDensityComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoFogDensityComponent;
	ud->initPointed(&g_luaUserDataTypeInfoFogDensityComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<CameraComponent>.
static inline int wrapSceneNodegetFirstCameraComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	CameraComponent& ret = self->getFirstComponentOfType<CameraComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "CameraComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoCameraComponent;
	ud->initPointed(&g_luaUserDataTypeInfoCameraComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<GlobalIlluminationProbeComponent>.
static inline int wrapSceneNodegetFirstGlobalIlluminationProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	GlobalIlluminationProbeComponent& ret = self->getFirstComponentOfType<GlobalIlluminationProbeComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "GlobalIlluminationProbeComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoGlobalIlluminationProbeComponent;
	ud->initPointed(&g_luaUserDataTypeInfoGlobalIlluminationProbeComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<ReflectionProbeComponent>.
static inline int wrapSceneNodegetFirstReflectionProbeComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ReflectionProbeComponent& ret = self->getFirstComponentOfType<ReflectionProbeComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ReflectionProbeComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoReflectionProbeComponent;
	ud->initPointed(&g_luaUserDataTypeInfoReflectionProbeComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<BodyComponent>.
static inline int wrapSceneNodegetFirstBodyComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	BodyComponent& ret = self->getFirstComponentOfType<BodyComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "BodyComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoBodyComponent;
	ud->initPointed(&g_luaUserDataTypeInfoBodyComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<ParticleEmitterComponent>.
static inline int wrapSceneNodegetFirstParticleEmitterComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	ParticleEmitterComponent& ret = self->getFirstComponentOfType<ParticleEmitterComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ParticleEmitterComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoParticleEmitterComponent;
	ud->initPointed(&g_luaUserDataTypeInfoParticleEmitterComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<MeshComponent>.
static inline int wrapSceneNodegetFirstMeshComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MeshComponent& ret = self->getFirstComponentOfType<MeshComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MeshComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMeshComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMeshComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<MaterialComponent>.
static inline int wrapSceneNodegetFirstMaterialComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	MaterialComponent& ret = self->getFirstComponentOfType<MaterialComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "MaterialComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMaterialComponent;
	ud->initPointed(&g_luaUserDataTypeInfoMaterialComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<SkinComponent>.
static inline int wrapSceneNodegetFirstSkinComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkinComponent& ret = self->getFirstComponentOfType<SkinComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkinComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSkinComponent;
	ud->initPointed(&g_luaUserDataTypeInfoSkinComponent, &ret);

	return 1;
}

// Wrap method SceneNode::getFirstComponentOfType<SkyboxComponent>.
static inline int wrapSceneNodegetFirstSkyboxComponent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* self = ud->getData<SceneNode>();

	// Call the method
	SkyboxComponent& ret = self->getFirstComponentOfType<SkyboxComponent>();

	// Push return value
	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SkyboxComponent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSkyboxComponent;
	ud->initPointed(&g_luaUserDataTypeInfoSkyboxComponent, &ret);

	return 1;
}

// Wrap class SceneNode.
static inline void wrapSceneNode(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoSceneNode);
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
	LuaBinder::pushLuaCFuncMethod(l, "newParticleEmitter2Component", wrapSceneNodenewParticleEmitter2Component);
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

LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneGraph = {8901208207670225695, "SceneGraph", LuaUserData::computeSizeForGarbageCollected<SceneGraph>(),
													   nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<SceneGraph>()
{
	return g_luaUserDataTypeInfoSceneGraph;
}

// Wrap method SceneGraph::newSceneNode.
static inline int wrapSceneGraphnewSceneNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneGraph, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	SceneNode* ret = newSceneNode<SceneNode>(self, arg0);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	ud->initPointed(&g_luaUserDataTypeInfoSceneNode, ret);

	return 1;
}

// Wrap method SceneGraph::setActiveCameraNode.
static inline int wrapSceneGraphsetActiveCameraNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneGraph, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* iarg0 = ud->getData<SceneNode>();
	SceneNode* arg0(iarg0);

	// Call the method
	self->setActiveCameraNode(arg0);

	return 0;
}

// Wrap method SceneGraph::tryFindSceneNode.
static inline int wrapSceneGraphtryFindSceneNode(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoSceneGraph, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneGraph* self = ud->getData<SceneGraph>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	SceneNode* ret = self->tryFindSceneNode(arg0);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		lua_pushnil(l);
		return 1;
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	ud->initPointed(&g_luaUserDataTypeInfoSceneNode, ret);

	return 1;
}

// Wrap class SceneGraph.
static inline void wrapSceneGraph(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoSceneGraph);
	LuaBinder::pushLuaCFuncMethod(l, "newSceneNode", wrapSceneGraphnewSceneNode);
	LuaBinder::pushLuaCFuncMethod(l, "setActiveCameraNode", wrapSceneGraphsetActiveCameraNode);
	LuaBinder::pushLuaCFuncMethod(l, "tryFindSceneNode", wrapSceneGraphtryFindSceneNode);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoEvent = {-7252054505959214788, "Event", LuaUserData::computeSizeForGarbageCollected<Event>(), nullptr,
												  nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Event>()
{
	return g_luaUserDataTypeInfoEvent;
}

// Wrap method Event::getAssociatedSceneNodes.
static inline int wrapEventgetAssociatedSceneNodes(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoEvent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Event* self = ud->getData<Event>();

	// Call the method
	WeakArraySceneNodePtr ret = self->getAssociatedSceneNodes();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<WeakArraySceneNodePtr>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "WeakArraySceneNodePtr");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoWeakArraySceneNodePtr;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoWeakArraySceneNodePtr);
	::new(ud->getData<WeakArraySceneNodePtr>()) WeakArraySceneNodePtr(std::move(ret));

	return 1;
}

// Wrap class Event.
static inline void wrapEvent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoEvent);
	LuaBinder::pushLuaCFuncMethod(l, "getAssociatedSceneNodes", wrapEventgetAssociatedSceneNodes);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoLightEvent = {-8586733957439759552, "LightEvent", LuaUserData::computeSizeForGarbageCollected<LightEvent>(),
													   nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<LightEvent>()
{
	return g_luaUserDataTypeInfoLightEvent;
}

// Wrap method LightEvent::setIntensityMultiplier.
static inline int wrapLightEventsetIntensityMultiplier(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightEvent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightEvent* self = ud->getData<LightEvent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setIntensityMultiplier(arg0);

	return 0;
}

// Wrap method LightEvent::setFrequency.
static inline int wrapLightEventsetFrequency(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoLightEvent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	LightEvent* self = ud->getData<LightEvent>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	self->setFrequency(arg0, arg1);

	return 0;
}

// Wrap class LightEvent.
static inline void wrapLightEvent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoLightEvent);
	LuaBinder::pushLuaCFuncMethod(l, "setIntensityMultiplier", wrapLightEventsetIntensityMultiplier);
	LuaBinder::pushLuaCFuncMethod(l, "setFrequency", wrapLightEventsetFrequency);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoScriptEvent = {3521092713286297735, "ScriptEvent",
														LuaUserData::computeSizeForGarbageCollected<ScriptEvent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<ScriptEvent>()
{
	return g_luaUserDataTypeInfoScriptEvent;
}

// Wrap class ScriptEvent.
static inline void wrapScriptEvent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoScriptEvent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoJitterMoveEvent = {-8794680355368287829, "JitterMoveEvent",
															LuaUserData::computeSizeForGarbageCollected<JitterMoveEvent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<JitterMoveEvent>()
{
	return g_luaUserDataTypeInfoJitterMoveEvent;
}

// Wrap method JitterMoveEvent::setPositionLimits.
static inline int wrapJitterMoveEventsetPositionLimits(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoJitterMoveEvent, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	JitterMoveEvent* self = ud->getData<JitterMoveEvent>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg1 = ud->getData<Vec3>();
	Vec3 arg1(*iarg1);

	// Call the method
	self->setPositionLimits(arg0, arg1);

	return 0;
}

// Wrap class JitterMoveEvent.
static inline void wrapJitterMoveEvent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoJitterMoveEvent);
	LuaBinder::pushLuaCFuncMethod(l, "setPositionLimits", wrapJitterMoveEventsetPositionLimits);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoAnimationEvent = {-8172624010586710412, "AnimationEvent",
														   LuaUserData::computeSizeForGarbageCollected<AnimationEvent>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<AnimationEvent>()
{
	return g_luaUserDataTypeInfoAnimationEvent;
}

// Wrap class AnimationEvent.
static inline void wrapAnimationEvent(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoAnimationEvent);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoEventManager = {1713399154262730355, "EventManager",
														 LuaUserData::computeSizeForGarbageCollected<EventManager>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<EventManager>()
{
	return g_luaUserDataTypeInfoEventManager;
}

// Wrap method EventManager::newLightEvent.
static inline int wrapEventManagernewLightEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoEventManager, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	LightEvent* ret = newEvent<LightEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "LightEvent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoLightEvent;
	ud->initPointed(&g_luaUserDataTypeInfoLightEvent, ret);

	return 1;
}

// Wrap method EventManager::newScriptEvent.
static inline int wrapEventManagernewScriptEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoEventManager, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	const char* arg2;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, arg2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	ScriptEvent* ret = newEvent<ScriptEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "ScriptEvent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoScriptEvent;
	ud->initPointed(&g_luaUserDataTypeInfoScriptEvent, ret);

	return 1;
}

// Wrap method EventManager::newJitterMoveEvent.
static inline int wrapEventManagernewJitterMoveEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoEventManager, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	JitterMoveEvent* ret = newEvent<JitterMoveEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "JitterMoveEvent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoJitterMoveEvent;
	ud->initPointed(&g_luaUserDataTypeInfoJitterMoveEvent, ret);

	return 1;
}

// Wrap method EventManager::newAnimationEvent.
static inline int wrapEventManagernewAnimationEvent(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoEventManager, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	EventManager* self = ud->getData<EventManager>();

	// Pop arguments
	const char* arg0;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	const char* arg1;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneNode;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, g_luaUserDataTypeInfoSceneNode, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	SceneNode* iarg2 = ud->getData<SceneNode>();
	SceneNode* arg2(iarg2);

	// Call the method
	AnimationEvent* ret = newEvent<AnimationEvent>(self, arg0, arg1, arg2);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "AnimationEvent");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoAnimationEvent;
	ud->initPointed(&g_luaUserDataTypeInfoAnimationEvent, ret);

	return 1;
}

// Wrap class EventManager.
static inline void wrapEventManager(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoEventManager);
	LuaBinder::pushLuaCFuncMethod(l, "newLightEvent", wrapEventManagernewLightEvent);
	LuaBinder::pushLuaCFuncMethod(l, "newScriptEvent", wrapEventManagernewScriptEvent);
	LuaBinder::pushLuaCFuncMethod(l, "newJitterMoveEvent", wrapEventManagernewJitterMoveEvent);
	LuaBinder::pushLuaCFuncMethod(l, "newAnimationEvent", wrapEventManagernewAnimationEvent);
	lua_settop(l, 0);
}

// Wrap function getSceneGraph.
static inline int wrapgetSceneGraph(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	SceneGraph* ret = getSceneGraph(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "SceneGraph");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoSceneGraph;
	ud->initPointed(&g_luaUserDataTypeInfoSceneGraph, ret);

	return 1;
}

// Wrap function getEventManager.
static inline int wrapgetEventManager(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	EventManager* ret = getEventManager(l);

	// Push return value
	if(ret == nullptr) [[unlikely]]
	{
		return luaL_error(l, "Returned nullptr. Location %s:%d %s", ANKI_FILE, __LINE__, ANKI_FUNC);
	}

	voidp = lua_newuserdata(l, sizeof(LuaUserData));
	ud = static_cast<LuaUserData*>(voidp);
	luaL_setmetatable(l, "EventManager");
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoEventManager;
	ud->initPointed(&g_luaUserDataTypeInfoEventManager, ret);

	return 1;
}

// Wrap the module.
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
	wrapParticleEmitter2Component(l);
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
	wrapParticleGeometryType(l);
}

} // end namespace anki
