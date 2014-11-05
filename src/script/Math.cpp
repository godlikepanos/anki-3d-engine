// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include "anki/script/LuaBinder.h"
#include "anki/Math.h"

namespace anki {

//==============================================================================
// Vec2                                                                        =
//==============================================================================

//==============================================================================
static const char* classnameVec2 = "Vec2";

//==============================================================================
/// Pre-wrap constructor for Vec2.
static inline int pwrapVec2Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 0);
	
	// Create user data
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	ud->m_data = nullptr;
	ud->m_gc = false;
	luaL_setmetatable(l, classnameVec2);
	
	void* inst = LuaBinder::luaAlloc(l, sizeof(Vec2));
	if(ANKI_UNLIKELY(inst == nullptr))
	{
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(inst) Vec2();
	
	ud->m_data = inst;
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap constructor for Vec2.
static int wrapVec2Ctor(lua_State* l)
{
	int res = pwrapVec2Ctor(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap destructor for Vec2.
static int wrapVec2Dtor(lua_State* l)
{
	LuaBinder::checkArgsCount(l, 1);
	void* voidp = luaL_checkudata(l, 1, classnameVec2);
	UserData* ud = reinterpret_cast<UserData*>(voidp);
	if(ud->m_gc)
	{
		Vec2* inst = reinterpret_cast<Vec2*>(ud->m_data);
		inst->~Vec2();
		LuaBinder::luaFree(l, inst);
	}
	
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::getX.
static inline int pwrapVec2getX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).x();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::getX.
static int wrapVec2getX(lua_State* l)
{
	int res = pwrapVec2getX(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::getY.
static inline int pwrapVec2getY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).y();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::getY.
static int wrapVec2getY(lua_State* l)
{
	int res = pwrapVec2getY(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::setX.
static inline int pwrapVec2setX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).x() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec2::setX.
static int wrapVec2setX(lua_State* l)
{
	int res = pwrapVec2setX(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::setY.
static inline int pwrapVec2setY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).y() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec2::setY.
static int wrapVec2setY(lua_State* l)
{
	int res = pwrapVec2setY(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::getAt.
static inline int pwrapVec2getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	F32 ret = (*self)[arg0];
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::getAt.
static int wrapVec2getAt(lua_State* l)
{
	int res = pwrapVec2getAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::setAt.
static inline int pwrapVec2setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 3);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	// Call the method
	(*self)[arg0] = arg1;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec2::setAt.
static int wrapVec2setAt(lua_State* l)
{
	int res = pwrapVec2setAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::operator=.
static inline int pwrapVec2copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	self->operator=(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method Vec2::operator=.
static int wrapVec2copy(lua_State* l)
{
	int res = pwrapVec2copy(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::operator+.
static inline int pwrapVec2__add(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	Vec2 ret = self->operator+(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec2");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec2));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec2(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::operator+.
static int wrapVec2__add(lua_State* l)
{
	int res = pwrapVec2__add(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::operator-.
static inline int pwrapVec2__sub(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	Vec2 ret = self->operator-(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec2");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec2));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec2(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::operator-.
static int wrapVec2__sub(lua_State* l)
{
	int res = pwrapVec2__sub(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::operator*.
static inline int pwrapVec2__mul(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	Vec2 ret = self->operator*(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec2");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec2));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec2(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::operator*.
static int wrapVec2__mul(lua_State* l)
{
	int res = pwrapVec2__mul(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::operator/.
static inline int pwrapVec2__div(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	Vec2 ret = self->operator/(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec2");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec2));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec2(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::operator/.
static int wrapVec2__div(lua_State* l)
{
	int res = pwrapVec2__div(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::operator==.
static inline int pwrapVec2__eq(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	Bool ret = self->operator==(arg0);
	
	// Push return value
	lua_pushboolean(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::operator==.
static int wrapVec2__eq(lua_State* l)
{
	int res = pwrapVec2__eq(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::getLength.
static inline int pwrapVec2getLength(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = self->getLength();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::getLength.
static int wrapVec2getLength(lua_State* l)
{
	int res = pwrapVec2getLength(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::getNormalized.
static inline int pwrapVec2getNormalized(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	Vec2 ret = self->getNormalized();
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec2");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec2));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec2(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::getNormalized.
static int wrapVec2getNormalized(lua_State* l)
{
	int res = pwrapVec2getNormalized(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::normalize.
static inline int pwrapVec2normalize(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	self->normalize();
	
	return 0;
}

//==============================================================================
/// Wrap method Vec2::normalize.
static int wrapVec2normalize(lua_State* l)
{
	int res = pwrapVec2normalize(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec2::dot.
static inline int pwrapVec2dot(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec2);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* self = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec2");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec2* iarg0 = reinterpret_cast<Vec2*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec2& arg0(*iarg0);
	
	// Call the method
	F32 ret = self->dot(arg0);
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec2::dot.
static int wrapVec2dot(lua_State* l)
{
	int res = pwrapVec2dot(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class Vec2.
static inline void wrapVec2(lua_State* l)
{
	LuaBinder::createClass(l, classnameVec2);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameVec2, "new", wrapVec2Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapVec2Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "getX", wrapVec2getX);
	LuaBinder::pushLuaCFuncMethod(l, "getY", wrapVec2getY);
	LuaBinder::pushLuaCFuncMethod(l, "setX", wrapVec2setX);
	LuaBinder::pushLuaCFuncMethod(l, "setY", wrapVec2setY);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapVec2getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapVec2setAt);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapVec2copy);
	LuaBinder::pushLuaCFuncMethod(l, "__add", wrapVec2__add);
	LuaBinder::pushLuaCFuncMethod(l, "__sub", wrapVec2__sub);
	LuaBinder::pushLuaCFuncMethod(l, "__mul", wrapVec2__mul);
	LuaBinder::pushLuaCFuncMethod(l, "__div", wrapVec2__div);
	LuaBinder::pushLuaCFuncMethod(l, "__eq", wrapVec2__eq);
	LuaBinder::pushLuaCFuncMethod(l, "getLength", wrapVec2getLength);
	LuaBinder::pushLuaCFuncMethod(l, "getNormalized", wrapVec2getNormalized);
	LuaBinder::pushLuaCFuncMethod(l, "normalize", wrapVec2normalize);
	LuaBinder::pushLuaCFuncMethod(l, "dot", wrapVec2dot);
	lua_settop(l, 0);
}

//==============================================================================
// Vec3                                                                        =
//==============================================================================

//==============================================================================
static const char* classnameVec3 = "Vec3";

//==============================================================================
/// Pre-wrap constructor for Vec3.
static inline int pwrapVec3Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 0);
	
	// Create user data
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	ud->m_data = nullptr;
	ud->m_gc = false;
	luaL_setmetatable(l, classnameVec3);
	
	void* inst = LuaBinder::luaAlloc(l, sizeof(Vec3));
	if(ANKI_UNLIKELY(inst == nullptr))
	{
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(inst) Vec3();
	
	ud->m_data = inst;
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap constructor for Vec3.
static int wrapVec3Ctor(lua_State* l)
{
	int res = pwrapVec3Ctor(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap destructor for Vec3.
static int wrapVec3Dtor(lua_State* l)
{
	LuaBinder::checkArgsCount(l, 1);
	void* voidp = luaL_checkudata(l, 1, classnameVec3);
	UserData* ud = reinterpret_cast<UserData*>(voidp);
	if(ud->m_gc)
	{
		Vec3* inst = reinterpret_cast<Vec3*>(ud->m_data);
		inst->~Vec3();
		LuaBinder::luaFree(l, inst);
	}
	
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::getX.
static inline int pwrapVec3getX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).x();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::getX.
static int wrapVec3getX(lua_State* l)
{
	int res = pwrapVec3getX(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::getY.
static inline int pwrapVec3getY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).y();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::getY.
static int wrapVec3getY(lua_State* l)
{
	int res = pwrapVec3getY(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::getZ.
static inline int pwrapVec3getZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).z();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::getZ.
static int wrapVec3getZ(lua_State* l)
{
	int res = pwrapVec3getZ(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::setX.
static inline int pwrapVec3setX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).x() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec3::setX.
static int wrapVec3setX(lua_State* l)
{
	int res = pwrapVec3setX(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::setY.
static inline int pwrapVec3setY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).y() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec3::setY.
static int wrapVec3setY(lua_State* l)
{
	int res = pwrapVec3setY(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::setZ.
static inline int pwrapVec3setZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).z() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec3::setZ.
static int wrapVec3setZ(lua_State* l)
{
	int res = pwrapVec3setZ(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::getAt.
static inline int pwrapVec3getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	F32 ret = (*self)[arg0];
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::getAt.
static int wrapVec3getAt(lua_State* l)
{
	int res = pwrapVec3getAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::setAt.
static inline int pwrapVec3setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 3);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	// Call the method
	(*self)[arg0] = arg1;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec3::setAt.
static int wrapVec3setAt(lua_State* l)
{
	int res = pwrapVec3setAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::operator=.
static inline int pwrapVec3copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	self->operator=(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method Vec3::operator=.
static int wrapVec3copy(lua_State* l)
{
	int res = pwrapVec3copy(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::operator+.
static inline int pwrapVec3__add(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	Vec3 ret = self->operator+(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec3));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec3(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::operator+.
static int wrapVec3__add(lua_State* l)
{
	int res = pwrapVec3__add(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::operator-.
static inline int pwrapVec3__sub(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	Vec3 ret = self->operator-(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec3));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec3(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::operator-.
static int wrapVec3__sub(lua_State* l)
{
	int res = pwrapVec3__sub(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::operator*.
static inline int pwrapVec3__mul(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	Vec3 ret = self->operator*(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec3));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec3(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::operator*.
static int wrapVec3__mul(lua_State* l)
{
	int res = pwrapVec3__mul(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::operator/.
static inline int pwrapVec3__div(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	Vec3 ret = self->operator/(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec3));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec3(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::operator/.
static int wrapVec3__div(lua_State* l)
{
	int res = pwrapVec3__div(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::operator==.
static inline int pwrapVec3__eq(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	Bool ret = self->operator==(arg0);
	
	// Push return value
	lua_pushboolean(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::operator==.
static int wrapVec3__eq(lua_State* l)
{
	int res = pwrapVec3__eq(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::getLength.
static inline int pwrapVec3getLength(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = self->getLength();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::getLength.
static int wrapVec3getLength(lua_State* l)
{
	int res = pwrapVec3getLength(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::getNormalized.
static inline int pwrapVec3getNormalized(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	Vec3 ret = self->getNormalized();
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec3");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec3));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec3(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::getNormalized.
static int wrapVec3getNormalized(lua_State* l)
{
	int res = pwrapVec3getNormalized(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::normalize.
static inline int pwrapVec3normalize(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	self->normalize();
	
	return 0;
}

//==============================================================================
/// Wrap method Vec3::normalize.
static int wrapVec3normalize(lua_State* l)
{
	int res = pwrapVec3normalize(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec3::dot.
static inline int pwrapVec3dot(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec3);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* self = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec3");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec3* iarg0 = reinterpret_cast<Vec3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec3& arg0(*iarg0);
	
	// Call the method
	F32 ret = self->dot(arg0);
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec3::dot.
static int wrapVec3dot(lua_State* l)
{
	int res = pwrapVec3dot(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class Vec3.
static inline void wrapVec3(lua_State* l)
{
	LuaBinder::createClass(l, classnameVec3);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameVec3, "new", wrapVec3Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapVec3Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "getX", wrapVec3getX);
	LuaBinder::pushLuaCFuncMethod(l, "getY", wrapVec3getY);
	LuaBinder::pushLuaCFuncMethod(l, "getZ", wrapVec3getZ);
	LuaBinder::pushLuaCFuncMethod(l, "setX", wrapVec3setX);
	LuaBinder::pushLuaCFuncMethod(l, "setY", wrapVec3setY);
	LuaBinder::pushLuaCFuncMethod(l, "setZ", wrapVec3setZ);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapVec3getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapVec3setAt);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapVec3copy);
	LuaBinder::pushLuaCFuncMethod(l, "__add", wrapVec3__add);
	LuaBinder::pushLuaCFuncMethod(l, "__sub", wrapVec3__sub);
	LuaBinder::pushLuaCFuncMethod(l, "__mul", wrapVec3__mul);
	LuaBinder::pushLuaCFuncMethod(l, "__div", wrapVec3__div);
	LuaBinder::pushLuaCFuncMethod(l, "__eq", wrapVec3__eq);
	LuaBinder::pushLuaCFuncMethod(l, "getLength", wrapVec3getLength);
	LuaBinder::pushLuaCFuncMethod(l, "getNormalized", wrapVec3getNormalized);
	LuaBinder::pushLuaCFuncMethod(l, "normalize", wrapVec3normalize);
	LuaBinder::pushLuaCFuncMethod(l, "dot", wrapVec3dot);
	lua_settop(l, 0);
}

//==============================================================================
// Vec4                                                                        =
//==============================================================================

//==============================================================================
static const char* classnameVec4 = "Vec4";

//==============================================================================
/// Pre-wrap constructor for Vec4.
static inline int pwrapVec4Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 0);
	
	// Create user data
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	ud->m_data = nullptr;
	ud->m_gc = false;
	luaL_setmetatable(l, classnameVec4);
	
	void* inst = LuaBinder::luaAlloc(l, sizeof(Vec4));
	if(ANKI_UNLIKELY(inst == nullptr))
	{
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(inst) Vec4();
	
	ud->m_data = inst;
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap constructor for Vec4.
static int wrapVec4Ctor(lua_State* l)
{
	int res = pwrapVec4Ctor(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap destructor for Vec4.
static int wrapVec4Dtor(lua_State* l)
{
	LuaBinder::checkArgsCount(l, 1);
	void* voidp = luaL_checkudata(l, 1, classnameVec4);
	UserData* ud = reinterpret_cast<UserData*>(voidp);
	if(ud->m_gc)
	{
		Vec4* inst = reinterpret_cast<Vec4*>(ud->m_data);
		inst->~Vec4();
		LuaBinder::luaFree(l, inst);
	}
	
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getX.
static inline int pwrapVec4getX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).x();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getX.
static int wrapVec4getX(lua_State* l)
{
	int res = pwrapVec4getX(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getY.
static inline int pwrapVec4getY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).y();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getY.
static int wrapVec4getY(lua_State* l)
{
	int res = pwrapVec4getY(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getZ.
static inline int pwrapVec4getZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).z();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getZ.
static int wrapVec4getZ(lua_State* l)
{
	int res = pwrapVec4getZ(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getW.
static inline int pwrapVec4getW(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = (*self).w();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getW.
static int wrapVec4getW(lua_State* l)
{
	int res = pwrapVec4getW(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::setX.
static inline int pwrapVec4setX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).x() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::setX.
static int wrapVec4setX(lua_State* l)
{
	int res = pwrapVec4setX(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::setY.
static inline int pwrapVec4setY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).y() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::setY.
static int wrapVec4setY(lua_State* l)
{
	int res = pwrapVec4setY(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::setZ.
static inline int pwrapVec4setZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).z() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::setZ.
static int wrapVec4setZ(lua_State* l)
{
	int res = pwrapVec4setZ(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::setW.
static inline int pwrapVec4setW(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	(*self).w() = arg0;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::setW.
static int wrapVec4setW(lua_State* l)
{
	int res = pwrapVec4setW(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getAt.
static inline int pwrapVec4getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	// Call the method
	F32 ret = (*self)[arg0];
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getAt.
static int wrapVec4getAt(lua_State* l)
{
	int res = pwrapVec4getAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::setAt.
static inline int pwrapVec4setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 3);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	F32 arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	// Call the method
	(*self)[arg0] = arg1;
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::setAt.
static int wrapVec4setAt(lua_State* l)
{
	int res = pwrapVec4setAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::operator=.
static inline int pwrapVec4copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	self->operator=(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::operator=.
static int wrapVec4copy(lua_State* l)
{
	int res = pwrapVec4copy(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::operator+.
static inline int pwrapVec4__add(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	Vec4 ret = self->operator+(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec4));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec4(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::operator+.
static int wrapVec4__add(lua_State* l)
{
	int res = pwrapVec4__add(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::operator-.
static inline int pwrapVec4__sub(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	Vec4 ret = self->operator-(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec4));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec4(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::operator-.
static int wrapVec4__sub(lua_State* l)
{
	int res = pwrapVec4__sub(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::operator*.
static inline int pwrapVec4__mul(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	Vec4 ret = self->operator*(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec4));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec4(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::operator*.
static int wrapVec4__mul(lua_State* l)
{
	int res = pwrapVec4__mul(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::operator/.
static inline int pwrapVec4__div(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	Vec4 ret = self->operator/(arg0);
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec4));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec4(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::operator/.
static int wrapVec4__div(lua_State* l)
{
	int res = pwrapVec4__div(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::operator==.
static inline int pwrapVec4__eq(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	Bool ret = self->operator==(arg0);
	
	// Push return value
	lua_pushboolean(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::operator==.
static int wrapVec4__eq(lua_State* l)
{
	int res = pwrapVec4__eq(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getLength.
static inline int pwrapVec4getLength(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	F32 ret = self->getLength();
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getLength.
static int wrapVec4getLength(lua_State* l)
{
	int res = pwrapVec4getLength(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::getNormalized.
static inline int pwrapVec4getNormalized(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	Vec4 ret = self->getNormalized();
	
	// Push return value
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	luaL_setmetatable(l, "Vec4");
	ud->m_data = LuaBinder::luaAlloc(l, sizeof(Vec4));
	if(ANKI_UNLIKELY(ud->m_data == nullptr))
	{
		ud->m_gc = false;
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(ud->m_data) Vec4(std::move(ret));
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::getNormalized.
static int wrapVec4getNormalized(lua_State* l)
{
	int res = pwrapVec4getNormalized(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::normalize.
static inline int pwrapVec4normalize(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 1);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Call the method
	self->normalize();
	
	return 0;
}

//==============================================================================
/// Wrap method Vec4::normalize.
static int wrapVec4normalize(lua_State* l)
{
	int res = pwrapVec4normalize(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Vec4::dot.
static inline int pwrapVec4dot(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameVec4);
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* self = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Vec4");
	ud = reinterpret_cast<UserData*>(voidp);
	Vec4* iarg0 = reinterpret_cast<Vec4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Vec4& arg0(*iarg0);
	
	// Call the method
	F32 ret = self->dot(arg0);
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Vec4::dot.
static int wrapVec4dot(lua_State* l)
{
	int res = pwrapVec4dot(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class Vec4.
static inline void wrapVec4(lua_State* l)
{
	LuaBinder::createClass(l, classnameVec4);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameVec4, "new", wrapVec4Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapVec4Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "getX", wrapVec4getX);
	LuaBinder::pushLuaCFuncMethod(l, "getY", wrapVec4getY);
	LuaBinder::pushLuaCFuncMethod(l, "getZ", wrapVec4getZ);
	LuaBinder::pushLuaCFuncMethod(l, "getW", wrapVec4getW);
	LuaBinder::pushLuaCFuncMethod(l, "setX", wrapVec4setX);
	LuaBinder::pushLuaCFuncMethod(l, "setY", wrapVec4setY);
	LuaBinder::pushLuaCFuncMethod(l, "setZ", wrapVec4setZ);
	LuaBinder::pushLuaCFuncMethod(l, "setW", wrapVec4setW);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapVec4getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapVec4setAt);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapVec4copy);
	LuaBinder::pushLuaCFuncMethod(l, "__add", wrapVec4__add);
	LuaBinder::pushLuaCFuncMethod(l, "__sub", wrapVec4__sub);
	LuaBinder::pushLuaCFuncMethod(l, "__mul", wrapVec4__mul);
	LuaBinder::pushLuaCFuncMethod(l, "__div", wrapVec4__div);
	LuaBinder::pushLuaCFuncMethod(l, "__eq", wrapVec4__eq);
	LuaBinder::pushLuaCFuncMethod(l, "getLength", wrapVec4getLength);
	LuaBinder::pushLuaCFuncMethod(l, "getNormalized", wrapVec4getNormalized);
	LuaBinder::pushLuaCFuncMethod(l, "normalize", wrapVec4normalize);
	LuaBinder::pushLuaCFuncMethod(l, "dot", wrapVec4dot);
	lua_settop(l, 0);
}

//==============================================================================
// Mat3                                                                        =
//==============================================================================

//==============================================================================
static const char* classnameMat3 = "Mat3";

//==============================================================================
/// Pre-wrap constructor for Mat3.
static inline int pwrapMat3Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 0);
	
	// Create user data
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	ud->m_data = nullptr;
	ud->m_gc = false;
	luaL_setmetatable(l, classnameMat3);
	
	void* inst = LuaBinder::luaAlloc(l, sizeof(Mat3));
	if(ANKI_UNLIKELY(inst == nullptr))
	{
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(inst) Mat3();
	
	ud->m_data = inst;
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap constructor for Mat3.
static int wrapMat3Ctor(lua_State* l)
{
	int res = pwrapMat3Ctor(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap destructor for Mat3.
static int wrapMat3Dtor(lua_State* l)
{
	LuaBinder::checkArgsCount(l, 1);
	void* voidp = luaL_checkudata(l, 1, classnameMat3);
	UserData* ud = reinterpret_cast<UserData*>(voidp);
	if(ud->m_gc)
	{
		Mat3* inst = reinterpret_cast<Mat3*>(ud->m_data);
		inst->~Mat3();
		LuaBinder::luaFree(l, inst);
	}
	
	return 0;
}

//==============================================================================
/// Pre-wrap method Mat3::operator=.
static inline int pwrapMat3copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMat3);
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3* self = reinterpret_cast<Mat3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Mat3");
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3* iarg0 = reinterpret_cast<Mat3*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Mat3& arg0(*iarg0);
	
	// Call the method
	self->operator=(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method Mat3::operator=.
static int wrapMat3copy(lua_State* l)
{
	int res = pwrapMat3copy(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Mat3::getAt.
static inline int pwrapMat3getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 3);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMat3);
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3* self = reinterpret_cast<Mat3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	U arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	// Call the method
	F32 ret = (*self)(arg0, arg1);
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Mat3::getAt.
static int wrapMat3getAt(lua_State* l)
{
	int res = pwrapMat3getAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Mat3::setAt.
static inline int pwrapMat3setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 4);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMat3);
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3* self = reinterpret_cast<Mat3*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	U arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2)) return -1;
	
	// Call the method
	(*self)(arg0, arg1) = arg2;
	
	return 0;
}

//==============================================================================
/// Wrap method Mat3::setAt.
static int wrapMat3setAt(lua_State* l)
{
	int res = pwrapMat3setAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class Mat3.
static inline void wrapMat3(lua_State* l)
{
	LuaBinder::createClass(l, classnameMat3);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameMat3, "new", wrapMat3Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapMat3Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapMat3copy);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapMat3getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapMat3setAt);
	lua_settop(l, 0);
}

//==============================================================================
// Mat3x4                                                                      =
//==============================================================================

//==============================================================================
static const char* classnameMat3x4 = "Mat3x4";

//==============================================================================
/// Pre-wrap constructor for Mat3x4.
static inline int pwrapMat3x4Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 0);
	
	// Create user data
	voidp = lua_newuserdata(l, sizeof(UserData));
	ud = reinterpret_cast<UserData*>(voidp);
	ud->m_data = nullptr;
	ud->m_gc = false;
	luaL_setmetatable(l, classnameMat3x4);
	
	void* inst = LuaBinder::luaAlloc(l, sizeof(Mat3x4));
	if(ANKI_UNLIKELY(inst == nullptr))
	{
		lua_pushstring(l, "Out of memory");
		return -1;
	}
	
	::new(inst) Mat3x4();
	
	ud->m_data = inst;
	ud->m_gc = true;
	
	return 1;
}

//==============================================================================
/// Wrap constructor for Mat3x4.
static int wrapMat3x4Ctor(lua_State* l)
{
	int res = pwrapMat3x4Ctor(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap destructor for Mat3x4.
static int wrapMat3x4Dtor(lua_State* l)
{
	LuaBinder::checkArgsCount(l, 1);
	void* voidp = luaL_checkudata(l, 1, classnameMat3x4);
	UserData* ud = reinterpret_cast<UserData*>(voidp);
	if(ud->m_gc)
	{
		Mat3x4* inst = reinterpret_cast<Mat3x4*>(ud->m_data);
		inst->~Mat3x4();
		LuaBinder::luaFree(l, inst);
	}
	
	return 0;
}

//==============================================================================
/// Pre-wrap method Mat3x4::operator=.
static inline int pwrapMat3x4copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 2);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMat3x4);
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3x4* self = reinterpret_cast<Mat3x4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	voidp = luaL_checkudata(l, 2, "Mat3x4");
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3x4* iarg0 = reinterpret_cast<Mat3x4*>(ud->m_data);
	ANKI_ASSERT(iarg0 != nullptr);
	const Mat3x4& arg0(*iarg0);
	
	// Call the method
	self->operator=(arg0);
	
	return 0;
}

//==============================================================================
/// Wrap method Mat3x4::operator=.
static int wrapMat3x4copy(lua_State* l)
{
	int res = pwrapMat3x4copy(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Mat3x4::getAt.
static inline int pwrapMat3x4getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 3);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMat3x4);
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3x4* self = reinterpret_cast<Mat3x4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	U arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	// Call the method
	F32 ret = (*self)(arg0, arg1);
	
	// Push return value
	lua_pushnumber(l, ret);
	
	return 1;
}

//==============================================================================
/// Wrap method Mat3x4::getAt.
static int wrapMat3x4getAt(lua_State* l)
{
	int res = pwrapMat3x4getAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Pre-wrap method Mat3x4::setAt.
static inline int pwrapMat3x4setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	Error err = ErrorCode::NONE;
	(void)err;
	
	LuaBinder::checkArgsCount(l, 4);
	
	// Get "this" as "self"
	voidp = luaL_checkudata(l, 1, classnameMat3x4);
	ud = reinterpret_cast<UserData*>(voidp);
	Mat3x4* self = reinterpret_cast<Mat3x4*>(ud->m_data);
	ANKI_ASSERT(self != nullptr);
	
	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0)) return -1;
	
	U arg1;
	if(LuaBinder::checkNumber(l, 3, arg1)) return -1;
	
	F32 arg2;
	if(LuaBinder::checkNumber(l, 4, arg2)) return -1;
	
	// Call the method
	(*self)(arg0, arg1) = arg2;
	
	return 0;
}

//==============================================================================
/// Wrap method Mat3x4::setAt.
static int wrapMat3x4setAt(lua_State* l)
{
	int res = pwrapMat3x4setAt(l);
	if(res >= 0) return res;
	lua_error(l);
	return 0;
}

//==============================================================================
/// Wrap class Mat3x4.
static inline void wrapMat3x4(lua_State* l)
{
	LuaBinder::createClass(l, classnameMat3x4);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameMat3x4, "new", wrapMat3x4Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapMat3x4Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapMat3x4copy);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapMat3x4getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapMat3x4setAt);
	lua_settop(l, 0);
}

//==============================================================================
/// Wrap the module.
void wrapModuleMath(lua_State* l)
{
	wrapVec2(l);
	wrapVec3(l);
	wrapVec4(l);
	wrapMat3(l);
	wrapMat3x4(l);
}

} // end namespace anki

