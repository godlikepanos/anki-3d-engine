// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include "anki/script/LuaBinder.h"
#include "anki/Scene.h"
#include <utility>

namespace anki {

//==============================================================================
template<typename T, typename... TArgs>
static T* newSceneNode(SceneGraph* scene, CString name, TArgs... args)
{
	T* ptr;
	Error err = scene->template newSceneNode<T>(
		name, ptr, args...);

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
// MoveComponent                                                               =
//==============================================================================

//==============================================================================
static const char* classnameMoveComponent = "MoveComponent";

//==============================================================================
/// Pre-wrap method MoveComponent::setLocalOrigin.
static inline int pwrapMoveComponentsetLocalOrigin(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMoveComponent);
	ud = reinterpret_cast<UserData*>(voidp);
	MoveComponent* self = reinterpret_cast<MoveComponent*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	self->setLocalOrigin(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method MoveComponent::setLocalOrigin.
static int wrapMoveComponentsetLocalOrigin(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalOrigin(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method MoveComponent::getLocalOrigin.
static inline int pwrapMoveComponentgetLocalOrigin(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMoveComponent);
	ud = reinterpret_cast<UserData*>(voidp);
	MoveComponent* self = reinterpret_cast<MoveComponent*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	const Vec4& ret = self->getLocalOrigin();
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->m_data = const_cast<void*>(reinterpret_cast<const void*>(&ret));
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method MoveComponent::getLocalOrigin.
static int wrapMoveComponentgetLocalOrigin(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalOrigin(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method MoveComponent::setLocalRotation.
static inline int pwrapMoveComponentsetLocalRotation(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMoveComponent);
	ud = reinterpret_cast<UserData*>(voidp);
	MoveComponent* self = reinterpret_cast<MoveComponent*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Mat3x4");
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3x4* iarg0 = reinterpret_cast<Mat3x4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Mat3x4& arg0(*iarg0);
	
	// Call the method
	self->setLocalRotation(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method MoveComponent::setLocalRotation.
static int wrapMoveComponentsetLocalRotation(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalRotation(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method MoveComponent::getLocalRotation.
static inline int pwrapMoveComponentgetLocalRotation(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMoveComponent);
	ud = reinterpret_cast<UserData*>(voidp);
	MoveComponent* self = reinterpret_cast<MoveComponent*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	const Mat3x4& ret = self->getLocalRotation();
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Mat3x4");
	ud->m_data = const_cast<void*>(reinterpret_cast<const void*>(&ret));
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method MoveComponent::getLocalRotation.
static int wrapMoveComponentgetLocalRotation(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalRotation(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method MoveComponent::setLocalScale.
static inline int pwrapMoveComponentsetLocalScale(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMoveComponent);
	ud = reinterpret_cast<UserData*>(voidp);
	MoveComponent* self = reinterpret_cast<MoveComponent*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0(luaL_checknumber(l, 2));
	
	// Call the method
	self->setLocalScale(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method MoveComponent::setLocalScale.
static int wrapMoveComponentsetLocalScale(lua_State* l)
{
	int res = pwrapMoveComponentsetLocalScale(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method MoveComponent::getLocalScale.
static inline int pwrapMoveComponentgetLocalScale(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMoveComponent);
	ud = reinterpret_cast<UserData*>(voidp);
	MoveComponent* self = reinterpret_cast<MoveComponent*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = self->getLocalScale();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method MoveComponent::getLocalScale.
static int wrapMoveComponentgetLocalScale(lua_State* l)
{
	int res = pwrapMoveComponentgetLocalScale(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
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
	lua_settop(l, 0);
}

//==============================================================================
// SceneNode                                                                   =
//==============================================================================

//==============================================================================
static const char* classnameSceneNode = "SceneNode";

//==============================================================================
/// Pre-wrap method SceneNode::getName.
static inline int pwrapSceneNodegetName(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameSceneNode);
	ud = reinterpret_cast<UserData*>(voidp);
	SceneNode* self = reinterpret_cast<SceneNode*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	CString ret = self->getName();
	
	// Push return value
	lua_pushstring(l, &ret[0]);
	
	return 1;
}

//==============================================================================
/// Wrap method SceneNode::getName.
static int wrapSceneNodegetName(lua_State* l)
{
	int res = pwrapSceneNodegetName(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method SceneNode::addChild.
static inline int pwrapSceneNodeaddChild(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameSceneNode);
	ud = reinterpret_cast<UserData*>(voidp);
	SceneNode* self = reinterpret_cast<SceneNode*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "SceneNode");
	ud = reinterpret_cast<UserData*>(voidp);
	SceneNode* iarg0 = reinterpret_cast<SceneNode*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	SceneNode* arg0(iarg0);
	
	// Call the method
	Error ret = self->addChild(arg0);
	
	// Push return value
	if(ANKI_UNLIKELY(ret))
	{
		lua_pushstring(l, "Glue code returned an error");
		return -1;
	}
	
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method SceneNode::addChild.
static int wrapSceneNodeaddChild(lua_State* l)
{
	int res = pwrapSceneNodeaddChild(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method SceneNode::getComponent<MoveComponent>.
static inline int pwrapSceneNodegetMoveComponent(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameSceneNode);
	ud = reinterpret_cast<UserData*>(voidp);
	SceneNode* self = reinterpret_cast<SceneNode*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	MoveComponent& ret = self->getComponent<MoveComponent>();
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "MoveComponent");
	ud->m_data = reinterpret_cast<void*>(&ret);
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method SceneNode::getComponent<MoveComponent>.
static int wrapSceneNodegetMoveComponent(lua_State* l)
{
	int res = pwrapSceneNodegetMoveComponent(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class SceneNode.
static inline void wrapSceneNode(lua_State* l)
{
	LuaBinder::createClass(l, classnameSceneNode);
	LuaBinder::pushLuaCFuncMethod(l, "getName", wrapSceneNodegetName);
	LuaBinder::pushLuaCFuncMethod(l, "addChild", wrapSceneNodeaddChild);
	LuaBinder::pushLuaCFuncMethod(l, "getMoveComponent", wrapSceneNodegetMoveComponent);
	lua_settop(l, 0);
}

//==============================================================================
// ModelNode                                                                   =
//==============================================================================

//==============================================================================
static const char* classnameModelNode = "ModelNode";

//==============================================================================
/// Pre-wrap method ModelNode::getSceneNodeBase.
static inline int pwrapModelNodegetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameModelNode);
	ud = reinterpret_cast<UserData*>(voidp);
	ModelNode* self = reinterpret_cast<ModelNode*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	SceneNode& ret = *self;
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->m_data = reinterpret_cast<void*>(&ret);
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method ModelNode::getSceneNodeBase.
static int wrapModelNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapModelNodegetSceneNodeBase(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class ModelNode.
static inline void wrapModelNode(lua_State* l)
{
	LuaBinder::createClass(l, classnameModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapModelNodegetSceneNodeBase);
	lua_settop(l, 0);
}

//==============================================================================
// InstanceNode                                                                =
//==============================================================================

//==============================================================================
static const char* classnameInstanceNode = "InstanceNode";

//==============================================================================
/// Pre-wrap method InstanceNode::getSceneNodeBase.
static inline int pwrapInstanceNodegetSceneNodeBase(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameInstanceNode);
	ud = reinterpret_cast<UserData*>(voidp);
	InstanceNode* self = reinterpret_cast<InstanceNode*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	SceneNode& ret = *self;
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "SceneNode");
	ud->m_data = reinterpret_cast<void*>(&ret);
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method InstanceNode::getSceneNodeBase.
static int wrapInstanceNodegetSceneNodeBase(lua_State* l)
{
	int res = pwrapInstanceNodegetSceneNodeBase(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class InstanceNode.
static inline void wrapInstanceNode(lua_State* l)
{
	LuaBinder::createClass(l, classnameInstanceNode);
	LuaBinder::pushLuaCFuncMethod(l, "getSceneNodeBase", wrapInstanceNodegetSceneNodeBase);
	lua_settop(l, 0);
}

//==============================================================================
// SceneGraph                                                                  =
//==============================================================================

//==============================================================================
static const char* classnameSceneGraph = "SceneGraph";

//==============================================================================
/// Pre-wrap method SceneGraph::newModelNode.
static inline int pwrapSceneGraphnewModelNode(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 3);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameSceneGraph);
	ud = reinterpret_cast<UserData*>(voidp);
	SceneGraph* self = reinterpret_cast<SceneGraph*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	const char* arg0(luaL_checkstring(l, 2));
	
	const char* arg1(luaL_checkstring(l, 3));
	
	// Call the method
	ModelNode* ret = newSceneNode<ModelNode>(self, arg0, arg1);
	
	// Push return value
	if(ANKI_UNLIKELY(ret == 0))
	{
		lua_pushstring(l, "Glue code returned zero");
		return -1;
	}
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "ModelNode");
	ud->m_data = reinterpret_cast<void*>(ret);
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method SceneGraph::newModelNode.
static int wrapSceneGraphnewModelNode(lua_State* l)
{
	int res = pwrapSceneGraphnewModelNode(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method SceneGraph::newInstanceNode.
static inline int pwrapSceneGraphnewInstanceNode(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameSceneGraph);
	ud = reinterpret_cast<UserData*>(voidp);
	SceneGraph* self = reinterpret_cast<SceneGraph*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	const char* arg0(luaL_checkstring(l, 2));
	
	// Call the method
	InstanceNode* ret = newSceneNode<InstanceNode>(self, arg0);
	
	// Push return value
	if(ANKI_UNLIKELY(ret == 0))
	{
		lua_pushstring(l, "Glue code returned zero");
		return -1;
	}
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "InstanceNode");
	ud->m_data = reinterpret_cast<void*>(ret);
	ud->m_gc = false;
	
	return 1;
}

//==============================================================================
/// Wrap method SceneGraph::newInstanceNode.
static int wrapSceneGraphnewInstanceNode(lua_State* l)
{
	int res = pwrapSceneGraphnewInstanceNode(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class SceneGraph.
static inline void wrapSceneGraph(lua_State* l)
{
	LuaBinder::createClass(l, classnameSceneGraph);
	LuaBinder::pushLuaCFuncMethod(l, "newModelNode", wrapSceneGraphnewModelNode);
	LuaBinder::pushLuaCFuncMethod(l, "newInstanceNode", wrapSceneGraphnewInstanceNode);
	lua_settop(l, 0);
}

//==============================================================================
/// Wrap the module.
void wrapModuleScene(lua_State* l)
{
	wrapMoveComponent(l);
	wrapSceneNode(l);
	wrapModelNode(l);
	wrapInstanceNode(l);
	wrapSceneGraph(l);
}

} // end namespace anki

