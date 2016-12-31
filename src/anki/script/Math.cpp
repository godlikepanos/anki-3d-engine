// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: The file is auto generated.

#include <anki/script/LuaBinder.h>
#include <anki/Math.h>

namespace anki
{

static const char* classnameVec2 = "Vec2";

template<>
I64 LuaBinder::getWrappedTypeSignature<Vec2>()
{
	return 6804478823655046388;
}

template<>
const char* LuaBinder::getWrappedTypeName<Vec2>()
{
	return classnameVec2;
}

/// Pre-wrap constructor for Vec2.
static inline int pwrapVec2Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 1, arg0))
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 2, arg1))
	{
		return -1;
	}

	// Create user data
	size = UserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, classnameVec2);
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046388);
	::new(ud->getData<Vec2>()) Vec2(arg0, arg1);

	return 1;
}

/// Wrap constructor for Vec2.
static int wrapVec2Ctor(lua_State* l)
{
	int res = pwrapVec2Ctor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap destructor for Vec2.
static int wrapVec2Dtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	if(ud->isGarbageCollected())
	{
		Vec2* inst = ud->getData<Vec2>();
		inst->~Vec2();
	}

	return 0;
}

/// Pre-wrap method Vec2::getX.
static inline int pwrapVec2getX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	F32 ret = (*self).x();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec2::getX.
static int wrapVec2getX(lua_State* l)
{
	int res = pwrapVec2getX(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::getY.
static inline int pwrapVec2getY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	F32 ret = (*self).y();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec2::getY.
static int wrapVec2getY(lua_State* l)
{
	int res = pwrapVec2getY(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::setX.
static inline int pwrapVec2setX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).x() = arg0;

	return 0;
}

/// Wrap method Vec2::setX.
static int wrapVec2setX(lua_State* l)
{
	int res = pwrapVec2setX(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::setY.
static inline int pwrapVec2setY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).y() = arg0;

	return 0;
}

/// Wrap method Vec2::setY.
static int wrapVec2setY(lua_State* l)
{
	int res = pwrapVec2setY(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::setAll.
static inline int pwrapVec2setAll(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

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
	(*self) = Vec2(arg0, arg1);

	return 0;
}

/// Wrap method Vec2::setAll.
static int wrapVec2setAll(lua_State* l)
{
	int res = pwrapVec2setAll(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::getAt.
static inline int pwrapVec2getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	F32 ret = (*self)[arg0];

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec2::getAt.
static int wrapVec2getAt(lua_State* l)
{
	int res = pwrapVec2getAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::setAt.
static inline int pwrapVec2setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	U arg0;
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
	(*self)[arg0] = arg1;

	return 0;
}

/// Wrap method Vec2::setAt.
static int wrapVec2setAt(lua_State* l)
{
	int res = pwrapVec2setAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::operator=.
static inline int pwrapVec2copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

/// Wrap method Vec2::operator=.
static int wrapVec2copy(lua_State* l)
{
	int res = pwrapVec2copy(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::operator+.
static inline int pwrapVec2__add(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator+(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046388);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

/// Wrap method Vec2::operator+.
static int wrapVec2__add(lua_State* l)
{
	int res = pwrapVec2__add(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::operator-.
static inline int pwrapVec2__sub(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator-(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046388);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

/// Wrap method Vec2::operator-.
static int wrapVec2__sub(lua_State* l)
{
	int res = pwrapVec2__sub(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::operator*.
static inline int pwrapVec2__mul(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator*(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046388);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

/// Wrap method Vec2::operator*.
static int wrapVec2__mul(lua_State* l)
{
	int res = pwrapVec2__mul(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::operator/.
static inline int pwrapVec2__div(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator/(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046388);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

/// Wrap method Vec2::operator/.
static int wrapVec2__div(lua_State* l)
{
	int res = pwrapVec2__div(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::operator==.
static inline int pwrapVec2__eq(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Bool ret = self->operator==(arg0);

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

/// Wrap method Vec2::operator==.
static int wrapVec2__eq(lua_State* l)
{
	int res = pwrapVec2__eq(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::getLength.
static inline int pwrapVec2getLength(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	F32 ret = self->getLength();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec2::getLength.
static int wrapVec2getLength(lua_State* l)
{
	int res = pwrapVec2getLength(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::getNormalized.
static inline int pwrapVec2getNormalized(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	Vec2 ret = self->getNormalized();

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046388);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

/// Wrap method Vec2::getNormalized.
static int wrapVec2getNormalized(lua_State* l)
{
	int res = pwrapVec2getNormalized(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::normalize.
static inline int pwrapVec2normalize(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	self->normalize();

	return 0;
}

/// Wrap method Vec2::normalize.
static int wrapVec2normalize(lua_State* l)
{
	int res = pwrapVec2normalize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec2::dot.
static inline int pwrapVec2dot(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec2, 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec2", 6804478823655046388, ud))
	{
		return -1;
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	F32 ret = self->dot(arg0);

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec2::dot.
static int wrapVec2dot(lua_State* l)
{
	int res = pwrapVec2dot(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

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
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapVec2setAll);
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

static const char* classnameVec3 = "Vec3";

template<>
I64 LuaBinder::getWrappedTypeSignature<Vec3>()
{
	return 6804478823655046389;
}

template<>
const char* LuaBinder::getWrappedTypeName<Vec3>()
{
	return classnameVec3;
}

/// Pre-wrap constructor for Vec3.
static inline int pwrapVec3Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 1, arg0))
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 2, arg1))
	{
		return -1;
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, 3, arg2))
	{
		return -1;
	}

	// Create user data
	size = UserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, classnameVec3);
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046389);
	::new(ud->getData<Vec3>()) Vec3(arg0, arg1, arg2);

	return 1;
}

/// Wrap constructor for Vec3.
static int wrapVec3Ctor(lua_State* l)
{
	int res = pwrapVec3Ctor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap destructor for Vec3.
static int wrapVec3Dtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	if(ud->isGarbageCollected())
	{
		Vec3* inst = ud->getData<Vec3>();
		inst->~Vec3();
	}

	return 0;
}

/// Pre-wrap method Vec3::getX.
static inline int pwrapVec3getX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	F32 ret = (*self).x();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec3::getX.
static int wrapVec3getX(lua_State* l)
{
	int res = pwrapVec3getX(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::getY.
static inline int pwrapVec3getY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	F32 ret = (*self).y();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec3::getY.
static int wrapVec3getY(lua_State* l)
{
	int res = pwrapVec3getY(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::getZ.
static inline int pwrapVec3getZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	F32 ret = (*self).z();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec3::getZ.
static int wrapVec3getZ(lua_State* l)
{
	int res = pwrapVec3getZ(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::setX.
static inline int pwrapVec3setX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).x() = arg0;

	return 0;
}

/// Wrap method Vec3::setX.
static int wrapVec3setX(lua_State* l)
{
	int res = pwrapVec3setX(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::setY.
static inline int pwrapVec3setY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).y() = arg0;

	return 0;
}

/// Wrap method Vec3::setY.
static int wrapVec3setY(lua_State* l)
{
	int res = pwrapVec3setY(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::setZ.
static inline int pwrapVec3setZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).z() = arg0;

	return 0;
}

/// Wrap method Vec3::setZ.
static int wrapVec3setZ(lua_State* l)
{
	int res = pwrapVec3setZ(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::setAll.
static inline int pwrapVec3setAll(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

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
	(*self) = Vec3(arg0, arg1, arg2);

	return 0;
}

/// Wrap method Vec3::setAll.
static int wrapVec3setAll(lua_State* l)
{
	int res = pwrapVec3setAll(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::getAt.
static inline int pwrapVec3getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	F32 ret = (*self)[arg0];

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec3::getAt.
static int wrapVec3getAt(lua_State* l)
{
	int res = pwrapVec3getAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::setAt.
static inline int pwrapVec3setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	U arg0;
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
	(*self)[arg0] = arg1;

	return 0;
}

/// Wrap method Vec3::setAt.
static int wrapVec3setAt(lua_State* l)
{
	int res = pwrapVec3setAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::operator=.
static inline int pwrapVec3copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

/// Wrap method Vec3::operator=.
static int wrapVec3copy(lua_State* l)
{
	int res = pwrapVec3copy(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::operator+.
static inline int pwrapVec3__add(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator+(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046389);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method Vec3::operator+.
static int wrapVec3__add(lua_State* l)
{
	int res = pwrapVec3__add(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::operator-.
static inline int pwrapVec3__sub(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator-(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046389);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method Vec3::operator-.
static int wrapVec3__sub(lua_State* l)
{
	int res = pwrapVec3__sub(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::operator*.
static inline int pwrapVec3__mul(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator*(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046389);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method Vec3::operator*.
static int wrapVec3__mul(lua_State* l)
{
	int res = pwrapVec3__mul(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::operator/.
static inline int pwrapVec3__div(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator/(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046389);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method Vec3::operator/.
static int wrapVec3__div(lua_State* l)
{
	int res = pwrapVec3__div(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::operator==.
static inline int pwrapVec3__eq(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Bool ret = self->operator==(arg0);

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

/// Wrap method Vec3::operator==.
static int wrapVec3__eq(lua_State* l)
{
	int res = pwrapVec3__eq(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::getLength.
static inline int pwrapVec3getLength(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	F32 ret = self->getLength();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec3::getLength.
static int wrapVec3getLength(lua_State* l)
{
	int res = pwrapVec3getLength(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::getNormalized.
static inline int pwrapVec3getNormalized(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	Vec3 ret = self->getNormalized();

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec3");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046389);
	::new(ud->getData<Vec3>()) Vec3(std::move(ret));

	return 1;
}

/// Wrap method Vec3::getNormalized.
static int wrapVec3getNormalized(lua_State* l)
{
	int res = pwrapVec3getNormalized(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::normalize.
static inline int pwrapVec3normalize(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	self->normalize();

	return 0;
}

/// Wrap method Vec3::normalize.
static int wrapVec3normalize(lua_State* l)
{
	int res = pwrapVec3normalize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec3::dot.
static inline int pwrapVec3dot(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec3, 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec3", 6804478823655046389, ud))
	{
		return -1;
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	F32 ret = self->dot(arg0);

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec3::dot.
static int wrapVec3dot(lua_State* l)
{
	int res = pwrapVec3dot(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

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
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapVec3setAll);
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

static const char* classnameVec4 = "Vec4";

template<>
I64 LuaBinder::getWrappedTypeSignature<Vec4>()
{
	return 6804478823655046386;
}

template<>
const char* LuaBinder::getWrappedTypeName<Vec4>()
{
	return classnameVec4;
}

/// Pre-wrap constructor for Vec4.
static inline int pwrapVec4Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 4);

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 1, arg0))
	{
		return -1;
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, 2, arg1))
	{
		return -1;
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, 3, arg2))
	{
		return -1;
	}

	F32 arg3;
	if(LuaBinder::checkNumber(l, 4, arg3))
	{
		return -1;
	}

	// Create user data
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, classnameVec4);
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(arg0, arg1, arg2, arg3);

	return 1;
}

/// Wrap constructor for Vec4.
static int wrapVec4Ctor(lua_State* l)
{
	int res = pwrapVec4Ctor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap destructor for Vec4.
static int wrapVec4Dtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	if(ud->isGarbageCollected())
	{
		Vec4* inst = ud->getData<Vec4>();
		inst->~Vec4();
	}

	return 0;
}

/// Pre-wrap method Vec4::getX.
static inline int pwrapVec4getX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	F32 ret = (*self).x();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::getX.
static int wrapVec4getX(lua_State* l)
{
	int res = pwrapVec4getX(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::getY.
static inline int pwrapVec4getY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	F32 ret = (*self).y();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::getY.
static int wrapVec4getY(lua_State* l)
{
	int res = pwrapVec4getY(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::getZ.
static inline int pwrapVec4getZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	F32 ret = (*self).z();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::getZ.
static int wrapVec4getZ(lua_State* l)
{
	int res = pwrapVec4getZ(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::getW.
static inline int pwrapVec4getW(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	F32 ret = (*self).w();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::getW.
static int wrapVec4getW(lua_State* l)
{
	int res = pwrapVec4getW(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::setX.
static inline int pwrapVec4setX(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).x() = arg0;

	return 0;
}

/// Wrap method Vec4::setX.
static int wrapVec4setX(lua_State* l)
{
	int res = pwrapVec4setX(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::setY.
static inline int pwrapVec4setY(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).y() = arg0;

	return 0;
}

/// Wrap method Vec4::setY.
static int wrapVec4setY(lua_State* l)
{
	int res = pwrapVec4setY(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::setZ.
static inline int pwrapVec4setZ(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).z() = arg0;

	return 0;
}

/// Wrap method Vec4::setZ.
static int wrapVec4setZ(lua_State* l)
{
	int res = pwrapVec4setZ(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::setW.
static inline int pwrapVec4setW(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	(*self).w() = arg0;

	return 0;
}

/// Wrap method Vec4::setW.
static int wrapVec4setW(lua_State* l)
{
	int res = pwrapVec4setW(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::setAll.
static inline int pwrapVec4setAll(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 5);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

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
	(*self) = Vec4(arg0, arg1, arg2, arg3);

	return 0;
}

/// Wrap method Vec4::setAll.
static int wrapVec4setAll(lua_State* l)
{
	int res = pwrapVec4setAll(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::getAt.
static inline int pwrapVec4getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	F32 ret = (*self)[arg0];

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::getAt.
static int wrapVec4getAt(lua_State* l)
{
	int res = pwrapVec4getAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::setAt.
static inline int pwrapVec4setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	U arg0;
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
	(*self)[arg0] = arg1;

	return 0;
}

/// Wrap method Vec4::setAt.
static int wrapVec4setAt(lua_State* l)
{
	int res = pwrapVec4setAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::operator=.
static inline int pwrapVec4copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

/// Wrap method Vec4::operator=.
static int wrapVec4copy(lua_State* l)
{
	int res = pwrapVec4copy(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::operator+.
static inline int pwrapVec4__add(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator+(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method Vec4::operator+.
static int wrapVec4__add(lua_State* l)
{
	int res = pwrapVec4__add(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::operator-.
static inline int pwrapVec4__sub(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator-(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method Vec4::operator-.
static int wrapVec4__sub(lua_State* l)
{
	int res = pwrapVec4__sub(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::operator*.
static inline int pwrapVec4__mul(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator*(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method Vec4::operator*.
static int wrapVec4__mul(lua_State* l)
{
	int res = pwrapVec4__mul(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::operator/.
static inline int pwrapVec4__div(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator/(arg0);

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method Vec4::operator/.
static int wrapVec4__div(lua_State* l)
{
	int res = pwrapVec4__div(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::operator==.
static inline int pwrapVec4__eq(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Bool ret = self->operator==(arg0);

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

/// Wrap method Vec4::operator==.
static int wrapVec4__eq(lua_State* l)
{
	int res = pwrapVec4__eq(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::getLength.
static inline int pwrapVec4getLength(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	F32 ret = self->getLength();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::getLength.
static int wrapVec4getLength(lua_State* l)
{
	int res = pwrapVec4getLength(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::getNormalized.
static inline int pwrapVec4getNormalized(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	Vec4 ret = self->getNormalized();

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method Vec4::getNormalized.
static int wrapVec4getNormalized(lua_State* l)
{
	int res = pwrapVec4getNormalized(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::normalize.
static inline int pwrapVec4normalize(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	self->normalize();

	return 0;
}

/// Wrap method Vec4::normalize.
static int wrapVec4normalize(lua_State* l)
{
	int res = pwrapVec4normalize(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Vec4::dot.
static inline int pwrapVec4dot(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameVec4, 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	F32 ret = self->dot(arg0);

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Vec4::dot.
static int wrapVec4dot(lua_State* l)
{
	int res = pwrapVec4dot(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

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
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapVec4setAll);
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

static const char* classnameMat3 = "Mat3";

template<>
I64 LuaBinder::getWrappedTypeSignature<Mat3>()
{
	return 6306819796139686981;
}

template<>
const char* LuaBinder::getWrappedTypeName<Mat3>()
{
	return classnameMat3;
}

/// Pre-wrap constructor for Mat3.
static inline int pwrapMat3Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 0);

	// Create user data
	size = UserData::computeSizeForGarbageCollected<Mat3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, classnameMat3);
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6306819796139686981);
	::new(ud->getData<Mat3>()) Mat3();

	return 1;
}

/// Wrap constructor for Mat3.
static int wrapMat3Ctor(lua_State* l)
{
	int res = pwrapMat3Ctor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap destructor for Mat3.
static int wrapMat3Dtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);
	if(LuaBinder::checkUserData(l, 1, classnameMat3, 6306819796139686981, ud))
	{
		return -1;
	}

	if(ud->isGarbageCollected())
	{
		Mat3* inst = ud->getData<Mat3>();
		inst->~Mat3();
	}

	return 0;
}

/// Pre-wrap method Mat3::operator=.
static inline int pwrapMat3copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3, 6306819796139686981, ud))
	{
		return -1;
	}

	Mat3* self = ud->getData<Mat3>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Mat3", 6306819796139686981, ud))
	{
		return -1;
	}

	Mat3* iarg0 = ud->getData<Mat3>();
	const Mat3& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

/// Wrap method Mat3::operator=.
static int wrapMat3copy(lua_State* l)
{
	int res = pwrapMat3copy(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Mat3::getAt.
static inline int pwrapMat3getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3, 6306819796139686981, ud))
	{
		return -1;
	}

	Mat3* self = ud->getData<Mat3>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	U arg1;
	if(LuaBinder::checkNumber(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	F32 ret = (*self)(arg0, arg1);

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Mat3::getAt.
static int wrapMat3getAt(lua_State* l)
{
	int res = pwrapMat3getAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Mat3::setAt.
static inline int pwrapMat3setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3, 6306819796139686981, ud))
	{
		return -1;
	}

	Mat3* self = ud->getData<Mat3>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	U arg1;
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
	(*self)(arg0, arg1) = arg2;

	return 0;
}

/// Wrap method Mat3::setAt.
static int wrapMat3setAt(lua_State* l)
{
	int res = pwrapMat3setAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Mat3::setAll.
static inline int pwrapMat3setAll(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 10);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3, 6306819796139686981, ud))
	{
		return -1;
	}

	Mat3* self = ud->getData<Mat3>();

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

	F32 arg4;
	if(LuaBinder::checkNumber(l, 6, arg4))
	{
		return -1;
	}

	F32 arg5;
	if(LuaBinder::checkNumber(l, 7, arg5))
	{
		return -1;
	}

	F32 arg6;
	if(LuaBinder::checkNumber(l, 8, arg6))
	{
		return -1;
	}

	F32 arg7;
	if(LuaBinder::checkNumber(l, 9, arg7))
	{
		return -1;
	}

	F32 arg8;
	if(LuaBinder::checkNumber(l, 10, arg8))
	{
		return -1;
	}

	// Call the method
	(*self) = Mat3(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

	return 0;
}

/// Wrap method Mat3::setAll.
static int wrapMat3setAll(lua_State* l)
{
	int res = pwrapMat3setAll(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Mat3.
static inline void wrapMat3(lua_State* l)
{
	LuaBinder::createClass(l, classnameMat3);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameMat3, "new", wrapMat3Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapMat3Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapMat3copy);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapMat3getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapMat3setAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapMat3setAll);
	lua_settop(l, 0);
}

static const char* classnameMat3x4 = "Mat3x4";

template<>
I64 LuaBinder::getWrappedTypeSignature<Mat3x4>()
{
	return -2654194732934255869;
}

template<>
const char* LuaBinder::getWrappedTypeName<Mat3x4>()
{
	return classnameMat3x4;
}

/// Pre-wrap constructor for Mat3x4.
static inline int pwrapMat3x4Ctor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 0);

	// Create user data
	size = UserData::computeSizeForGarbageCollected<Mat3x4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, classnameMat3x4);
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(-2654194732934255869);
	::new(ud->getData<Mat3x4>()) Mat3x4();

	return 1;
}

/// Wrap constructor for Mat3x4.
static int wrapMat3x4Ctor(lua_State* l)
{
	int res = pwrapMat3x4Ctor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap destructor for Mat3x4.
static int wrapMat3x4Dtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);
	if(LuaBinder::checkUserData(l, 1, classnameMat3x4, -2654194732934255869, ud))
	{
		return -1;
	}

	if(ud->isGarbageCollected())
	{
		Mat3x4* inst = ud->getData<Mat3x4>();
		inst->~Mat3x4();
	}

	return 0;
}

/// Pre-wrap method Mat3x4::operator=.
static inline int pwrapMat3x4copy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3x4, -2654194732934255869, ud))
	{
		return -1;
	}

	Mat3x4* self = ud->getData<Mat3x4>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Mat3x4", -2654194732934255869, ud))
	{
		return -1;
	}

	Mat3x4* iarg0 = ud->getData<Mat3x4>();
	const Mat3x4& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

/// Wrap method Mat3x4::operator=.
static int wrapMat3x4copy(lua_State* l)
{
	int res = pwrapMat3x4copy(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Mat3x4::getAt.
static inline int pwrapMat3x4getAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 3);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3x4, -2654194732934255869, ud))
	{
		return -1;
	}

	Mat3x4* self = ud->getData<Mat3x4>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	U arg1;
	if(LuaBinder::checkNumber(l, 3, arg1))
	{
		return -1;
	}

	// Call the method
	F32 ret = (*self)(arg0, arg1);

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Mat3x4::getAt.
static int wrapMat3x4getAt(lua_State* l)
{
	int res = pwrapMat3x4getAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Mat3x4::setAt.
static inline int pwrapMat3x4setAt(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 4);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3x4, -2654194732934255869, ud))
	{
		return -1;
	}

	Mat3x4* self = ud->getData<Mat3x4>();

	// Pop arguments
	U arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	U arg1;
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
	(*self)(arg0, arg1) = arg2;

	return 0;
}

/// Wrap method Mat3x4::setAt.
static int wrapMat3x4setAt(lua_State* l)
{
	int res = pwrapMat3x4setAt(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Mat3x4::setAll.
static inline int pwrapMat3x4setAll(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 13);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameMat3x4, -2654194732934255869, ud))
	{
		return -1;
	}

	Mat3x4* self = ud->getData<Mat3x4>();

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

	F32 arg4;
	if(LuaBinder::checkNumber(l, 6, arg4))
	{
		return -1;
	}

	F32 arg5;
	if(LuaBinder::checkNumber(l, 7, arg5))
	{
		return -1;
	}

	F32 arg6;
	if(LuaBinder::checkNumber(l, 8, arg6))
	{
		return -1;
	}

	F32 arg7;
	if(LuaBinder::checkNumber(l, 9, arg7))
	{
		return -1;
	}

	F32 arg8;
	if(LuaBinder::checkNumber(l, 10, arg8))
	{
		return -1;
	}

	F32 arg9;
	if(LuaBinder::checkNumber(l, 11, arg9))
	{
		return -1;
	}

	F32 arg10;
	if(LuaBinder::checkNumber(l, 12, arg10))
	{
		return -1;
	}

	F32 arg11;
	if(LuaBinder::checkNumber(l, 13, arg11))
	{
		return -1;
	}

	// Call the method
	(*self) = Mat3x4(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

	return 0;
}

/// Wrap method Mat3x4::setAll.
static int wrapMat3x4setAll(lua_State* l)
{
	int res = pwrapMat3x4setAll(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Mat3x4.
static inline void wrapMat3x4(lua_State* l)
{
	LuaBinder::createClass(l, classnameMat3x4);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameMat3x4, "new", wrapMat3x4Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapMat3x4Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapMat3x4copy);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapMat3x4getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapMat3x4setAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapMat3x4setAll);
	lua_settop(l, 0);
}

static const char* classnameTransform = "Transform";

template<>
I64 LuaBinder::getWrappedTypeSignature<Transform>()
{
	return 7048620195620777229;
}

template<>
const char* LuaBinder::getWrappedTypeName<Transform>()
{
	return classnameTransform;
}

/// Pre-wrap constructor for Transform.
static inline int pwrapTransformCtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 0);

	// Create user data
	size = UserData::computeSizeForGarbageCollected<Transform>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, classnameTransform);
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(7048620195620777229);
	::new(ud->getData<Transform>()) Transform();

	return 1;
}

/// Wrap constructor for Transform.
static int wrapTransformCtor(lua_State* l)
{
	int res = pwrapTransformCtor(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap destructor for Transform.
static int wrapTransformDtor(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	if(ud->isGarbageCollected())
	{
		Transform* inst = ud->getData<Transform>();
		inst->~Transform();
	}

	return 0;
}

/// Pre-wrap method Transform::operator=.
static inline int pwrapTransformcopy(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Transform", 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* iarg0 = ud->getData<Transform>();
	const Transform& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

/// Wrap method Transform::operator=.
static int wrapTransformcopy(lua_State* l)
{
	int res = pwrapTransformcopy(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Transform::getOrigin.
static inline int pwrapTransformgetOrigin(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Call the method
	Vec4 ret = self->getOrigin();

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(6804478823655046386);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

/// Wrap method Transform::getOrigin.
static int wrapTransformgetOrigin(lua_State* l)
{
	int res = pwrapTransformgetOrigin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Transform::setOrigin.
static inline int pwrapTransformsetOrigin(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Vec4", 6804478823655046386, ud))
	{
		return -1;
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->setOrigin(arg0);

	return 0;
}

/// Wrap method Transform::setOrigin.
static int wrapTransformsetOrigin(lua_State* l)
{
	int res = pwrapTransformsetOrigin(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Transform::getRotation.
static inline int pwrapTransformgetRotation(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Call the method
	Mat3x4 ret = self->getRotation();

	// Push return value
	size = UserData::computeSizeForGarbageCollected<Mat3x4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Mat3x4");
	ud = static_cast<UserData*>(voidp);
	ud->initGarbageCollected(-2654194732934255869);
	::new(ud->getData<Mat3x4>()) Mat3x4(std::move(ret));

	return 1;
}

/// Wrap method Transform::getRotation.
static int wrapTransformgetRotation(lua_State* l)
{
	int res = pwrapTransformgetRotation(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Transform::setRotation.
static inline int pwrapTransformsetRotation(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	if(LuaBinder::checkUserData(l, 2, "Mat3x4", -2654194732934255869, ud))
	{
		return -1;
	}

	Mat3x4* iarg0 = ud->getData<Mat3x4>();
	const Mat3x4& arg0(*iarg0);

	// Call the method
	self->setRotation(arg0);

	return 0;
}

/// Wrap method Transform::setRotation.
static int wrapTransformsetRotation(lua_State* l)
{
	int res = pwrapTransformsetRotation(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Transform::getScale.
static inline int pwrapTransformgetScale(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Call the method
	F32 ret = self->getScale();

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap method Transform::getScale.
static int wrapTransformgetScale(lua_State* l)
{
	int res = pwrapTransformgetScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Pre-wrap method Transform::setScale.
static inline int pwrapTransformsetScale(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 2);

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, 1, classnameTransform, 7048620195620777229, ud))
	{
		return -1;
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 2, arg0))
	{
		return -1;
	}

	// Call the method
	self->setScale(arg0);

	return 0;
}

/// Wrap method Transform::setScale.
static int wrapTransformsetScale(lua_State* l)
{
	int res = pwrapTransformsetScale(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap class Transform.
static inline void wrapTransform(lua_State* l)
{
	LuaBinder::createClass(l, classnameTransform);
	LuaBinder::pushLuaCFuncStaticMethod(l, classnameTransform, "new", wrapTransformCtor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapTransformDtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapTransformcopy);
	LuaBinder::pushLuaCFuncMethod(l, "getOrigin", wrapTransformgetOrigin);
	LuaBinder::pushLuaCFuncMethod(l, "setOrigin", wrapTransformsetOrigin);
	LuaBinder::pushLuaCFuncMethod(l, "getRotation", wrapTransformgetRotation);
	LuaBinder::pushLuaCFuncMethod(l, "setRotation", wrapTransformsetRotation);
	LuaBinder::pushLuaCFuncMethod(l, "getScale", wrapTransformgetScale);
	LuaBinder::pushLuaCFuncMethod(l, "setScale", wrapTransformsetScale);
	lua_settop(l, 0);
}

/// Pre-wrap function toRad.
static inline int pwraptoRad(lua_State* l)
{
	UserData* ud;
	(void)ud;
	void* voidp;
	(void)voidp;
	PtrSize size;
	(void)size;

	LuaBinder::checkArgsCount(l, 1);

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, 1, arg0))
	{
		return -1;
	}

	// Call the function
	F32 ret = toRad(arg0);

	// Push return value
	lua_pushnumber(l, ret);

	return 1;
}

/// Wrap function toRad.
static int wraptoRad(lua_State* l)
{
	int res = pwraptoRad(l);
	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

/// Wrap the module.
void wrapModuleMath(lua_State* l)
{
	wrapVec2(l);
	wrapVec3(l);
	wrapVec4(l);
	wrapMat3(l);
	wrapMat3x4(l);
	wrapTransform(l);
	LuaBinder::pushLuaCFunc(l, "toRad", wraptoRad);
}

} // end namespace anki
