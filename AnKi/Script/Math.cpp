// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#include <AnKi/Script/LuaBinder.h>
#include <AnKi/Math.h>

namespace anki {

// Serialize Vec2
static void serializeVec2(LuaUserData& self, void* data, PtrSize& size)
{
	Vec2* obj = self.getData<Vec2>();
	obj->serialize(data, size);
}

// De-serialize Vec2
static void deserializeVec2(const void* data, LuaUserData& self)
{
	ANKI_ASSERT(data);
	Vec2* obj = self.getData<Vec2>();
	::new(obj) Vec2();
	obj->deserialize(data);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2 = {-6244337268269259101, "Vec2", LuaUserData::computeSizeForGarbageCollected<Vec2>(), serializeVec2,
												 deserializeVec2};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Vec2>()
{
	return g_luaUserDataTypeInfoVec2;
}

// Pre-wrap constructor for Vec2.
static inline int pwrapVec2Ctor0(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec2.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2();

	return 1;
}

// Pre-wrap constructor for Vec2.
static inline int pwrapVec2Ctor1(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec2.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2(arg0, arg1);

	return 1;
}

// Wrap constructors for Vec2.
static int wrapVec2Ctor(lua_State* l)
{
	// Chose the right overload
	const int argCount = lua_gettop(l);
	int res = 0;
	switch(argCount)
	{
	case 0:
		res = pwrapVec2Ctor0(l);
		break;
	case 2:
		res = pwrapVec2Ctor1(l);
		break;
	default:
		lua_pushfstring(l, "Wrong overloaded new. Wrong number of arguments: %d", argCount);
		res = -1;
	}

	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

// Wrap destructor for Vec2.
static int wrapVec2Dtor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(ud->isGarbageCollected())
	{
		Vec2* inst = ud->getData<Vec2>();
		inst->~Vec2();
	}

	return 0;
}

// Wrap writing the member vars of Vec2
static int wrapVec2__newindex(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Get the member variable name
	const Char* ckey;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]
	{
		return lua_error(l);
	}

	CString key = ckey;

	// Try to find the member variable
	if(key == "x")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->x = arg0;
		return 0;
	}
	else if(key == "y")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->y = arg0;
		return 0;
	}

	return luaL_error(l, "Unknown field %s. Location %s:%d %s", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);
}

// Wrap reading the member vars of Vec2
static int wrapVec2__index(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Get the member variable name
	const Char* ckey;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]
	{
		return lua_error(l);
	}

	CString key = ckey;

	// Try to find the member variable
	if(key == "x")
	{
		F32 ret = self->x;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}
	else if(key == "y")
	{
		F32 ret = self->y;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}

	// Fallback to methods
	luaL_getmetatable(l, "Vec2");
	lua_getfield(l, -1, ckey);
	if(!lua_isnil(l, -1))
	{
		return 1;
	}

	return luaL_error(l, "Unknown field %s. Location %s:%d %s", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);
}

// Wrap method Vec2::setAll.
static inline int wrapVec2setAll(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

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
	(*self) = Vec2(arg0, arg1);

	return 0;
}

// Wrap method Vec2::getAt.
static inline int wrapVec2getAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	F32 ret = (*self)[arg0];

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Vec2::setAt.
static inline int wrapVec2setAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	U32 arg0;
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
	(*self)[arg0] = arg1;

	return 0;
}

// Wrap method Vec2::operator=.
static inline int wrapVec2copy(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

// Wrap method Vec2::operator+.
static inline int wrapVec2__add(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator+(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

// Wrap method Vec2::operator-.
static inline int wrapVec2__sub(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator-(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

// Wrap method Vec2::operator*.
static inline int wrapVec2__mul(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator*(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

// Wrap method Vec2::operator/.
static inline int wrapVec2__div(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Vec2 ret = self->operator/(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

// Wrap method Vec2::operator==.
static inline int wrapVec2__eq(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	Bool ret = self->operator==(arg0);

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

// Wrap method Vec2::length.
static inline int wrapVec2length(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	F32 ret = self->length();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Vec2::normalize.
static inline int wrapVec2normalize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Call the method
	Vec2 ret = self->normalize();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec2>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec2");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec2);
	::new(ud->getData<Vec2>()) Vec2(std::move(ret));

	return 1;
}

// Wrap method Vec2::dot.
static inline int wrapVec2dot(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* self = ud->getData<Vec2>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec2;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec2, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec2* iarg0 = ud->getData<Vec2>();
	const Vec2& arg0(*iarg0);

	// Call the method
	F32 ret = self->dot(arg0);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class Vec2.
static inline void wrapVec2(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoVec2);
	LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfoVec2.m_typeName, "new", wrapVec2Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapVec2Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapVec2setAll);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapVec2getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapVec2setAt);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapVec2copy);
	LuaBinder::pushLuaCFuncMethod(l, "__add", wrapVec2__add);
	LuaBinder::pushLuaCFuncMethod(l, "__sub", wrapVec2__sub);
	LuaBinder::pushLuaCFuncMethod(l, "__mul", wrapVec2__mul);
	LuaBinder::pushLuaCFuncMethod(l, "__div", wrapVec2__div);
	LuaBinder::pushLuaCFuncMethod(l, "__eq", wrapVec2__eq);
	LuaBinder::pushLuaCFuncMethod(l, "length", wrapVec2length);
	LuaBinder::pushLuaCFuncMethod(l, "normalize", wrapVec2normalize);
	LuaBinder::pushLuaCFuncMethod(l, "dot", wrapVec2dot);
	LuaBinder::pushLuaCFuncMethod(l, "__newindex", wrapVec2__newindex);
	LuaBinder::pushLuaCFuncMethod(l, "__index", wrapVec2__index);
	lua_settop(l, 0);
}

// Serialize Vec3
static void serializeVec3(LuaUserData& self, void* data, PtrSize& size)
{
	Vec3* obj = self.getData<Vec3>();
	obj->serialize(data, size);
}

// De-serialize Vec3
static void deserializeVec3(const void* data, LuaUserData& self)
{
	ANKI_ASSERT(data);
	Vec3* obj = self.getData<Vec3>();
	::new(obj) Vec3();
	obj->deserialize(data);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3 = {-2740589931080892768, "Vec3", LuaUserData::computeSizeForGarbageCollected<Vec3>(), serializeVec3,
												 deserializeVec3};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Vec3>()
{
	return g_luaUserDataTypeInfoVec3;
}

// Pre-wrap constructor for Vec3.
static inline int pwrapVec3Ctor0(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec3.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3();

	return 1;
}

// Pre-wrap constructor for Vec3.
static inline int pwrapVec3Ctor1(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec3.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3(arg0);

	return 1;
}

// Pre-wrap constructor for Vec3.
static inline int pwrapVec3Ctor2(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec3.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec3);
	::new(ud->getData<Vec3>()) Vec3(arg0, arg1, arg2);

	return 1;
}

// Wrap constructors for Vec3.
static int wrapVec3Ctor(lua_State* l)
{
	// Chose the right overload
	const int argCount = lua_gettop(l);
	int res = 0;
	switch(argCount)
	{
	case 0:
		res = pwrapVec3Ctor0(l);
		break;
	case 1:
		res = pwrapVec3Ctor1(l);
		break;
	case 3:
		res = pwrapVec3Ctor2(l);
		break;
	default:
		lua_pushfstring(l, "Wrong overloaded new. Wrong number of arguments: %d", argCount);
		res = -1;
	}

	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

// Wrap destructor for Vec3.
static int wrapVec3Dtor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(ud->isGarbageCollected())
	{
		Vec3* inst = ud->getData<Vec3>();
		inst->~Vec3();
	}

	return 0;
}

// Wrap writing the member vars of Vec3
static int wrapVec3__newindex(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Get the member variable name
	const Char* ckey;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]
	{
		return lua_error(l);
	}

	CString key = ckey;

	// Try to find the member variable
	if(key == "x")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->x = arg0;
		return 0;
	}
	else if(key == "y")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->y = arg0;
		return 0;
	}
	else if(key == "z")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->z = arg0;
		return 0;
	}

	return luaL_error(l, "Unknown field %s. Location %s:%d %s", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);
}

// Wrap reading the member vars of Vec3
static int wrapVec3__index(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Get the member variable name
	const Char* ckey;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]
	{
		return lua_error(l);
	}

	CString key = ckey;

	// Try to find the member variable
	if(key == "x")
	{
		F32 ret = self->x;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}
	else if(key == "y")
	{
		F32 ret = self->y;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}
	else if(key == "z")
	{
		F32 ret = self->z;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}

	// Fallback to methods
	luaL_getmetatable(l, "Vec3");
	lua_getfield(l, -1, ckey);
	if(!lua_isnil(l, -1))
	{
		return 1;
	}

	return luaL_error(l, "Unknown field %s. Location %s:%d %s", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);
}

// Wrap method Vec3::setAll.
static inline int wrapVec3setAll(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

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

	// Call the method
	(*self) = Vec3(arg0, arg1, arg2);

	return 0;
}

// Wrap method Vec3::getAt.
static inline int wrapVec3getAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	F32 ret = (*self)[arg0];

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Vec3::setAt.
static inline int wrapVec3setAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	U32 arg0;
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
	(*self)[arg0] = arg1;

	return 0;
}

// Wrap method Vec3::operator=.
static inline int wrapVec3copy(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

// Wrap method Vec3::operator+.
static inline int wrapVec3__add(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator+(arg0);

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

// Wrap method Vec3::operator-.
static inline int wrapVec3__sub(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator-(arg0);

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

// Wrap method Vec3::operator*.
static inline int wrapVec3__mul(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator*(arg0);

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

// Wrap method Vec3::operator/.
static inline int wrapVec3__div(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Vec3 ret = self->operator/(arg0);

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

// Wrap method Vec3::operator==.
static inline int wrapVec3__eq(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	Bool ret = self->operator==(arg0);

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

// Wrap method Vec3::length.
static inline int wrapVec3length(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	F32 ret = self->length();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Vec3::normalize.
static inline int wrapVec3normalize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Call the method
	Vec3 ret = self->normalize();

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

// Wrap method Vec3::dot.
static inline int wrapVec3dot(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* self = ud->getData<Vec3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	F32 ret = self->dot(arg0);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class Vec3.
static inline void wrapVec3(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoVec3);
	LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfoVec3.m_typeName, "new", wrapVec3Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapVec3Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapVec3setAll);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapVec3getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapVec3setAt);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapVec3copy);
	LuaBinder::pushLuaCFuncMethod(l, "__add", wrapVec3__add);
	LuaBinder::pushLuaCFuncMethod(l, "__sub", wrapVec3__sub);
	LuaBinder::pushLuaCFuncMethod(l, "__mul", wrapVec3__mul);
	LuaBinder::pushLuaCFuncMethod(l, "__div", wrapVec3__div);
	LuaBinder::pushLuaCFuncMethod(l, "__eq", wrapVec3__eq);
	LuaBinder::pushLuaCFuncMethod(l, "length", wrapVec3length);
	LuaBinder::pushLuaCFuncMethod(l, "normalize", wrapVec3normalize);
	LuaBinder::pushLuaCFuncMethod(l, "dot", wrapVec3dot);
	LuaBinder::pushLuaCFuncMethod(l, "__newindex", wrapVec3__newindex);
	LuaBinder::pushLuaCFuncMethod(l, "__index", wrapVec3__index);
	lua_settop(l, 0);
}

// Serialize Vec4
static void serializeVec4(LuaUserData& self, void* data, PtrSize& size)
{
	Vec4* obj = self.getData<Vec4>();
	obj->serialize(data, size);
}

// De-serialize Vec4
static void deserializeVec4(const void* data, LuaUserData& self)
{
	ANKI_ASSERT(data);
	Vec4* obj = self.getData<Vec4>();
	::new(obj) Vec4();
	obj->deserialize(data);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4 = {8378982108621294078, "Vec4", LuaUserData::computeSizeForGarbageCollected<Vec4>(), serializeVec4,
												 deserializeVec4};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Vec4>()
{
	return g_luaUserDataTypeInfoVec4;
}

// Pre-wrap constructor for Vec4.
static inline int pwrapVec4Ctor0(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec4.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4();

	return 1;
}

// Pre-wrap constructor for Vec4.
static inline int pwrapVec4Ctor1(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec4.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(arg0);

	return 1;
}

// Pre-wrap constructor for Vec4.
static inline int pwrapVec4Ctor2(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg2)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg3;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, arg3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoVec4.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(arg0, arg1, arg2, arg3);

	return 1;
}

// Wrap constructors for Vec4.
static int wrapVec4Ctor(lua_State* l)
{
	// Chose the right overload
	const int argCount = lua_gettop(l);
	int res = 0;
	switch(argCount)
	{
	case 0:
		res = pwrapVec4Ctor0(l);
		break;
	case 1:
		res = pwrapVec4Ctor1(l);
		break;
	case 4:
		res = pwrapVec4Ctor2(l);
		break;
	default:
		lua_pushfstring(l, "Wrong overloaded new. Wrong number of arguments: %d", argCount);
		res = -1;
	}

	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

// Wrap destructor for Vec4.
static int wrapVec4Dtor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(ud->isGarbageCollected())
	{
		Vec4* inst = ud->getData<Vec4>();
		inst->~Vec4();
	}

	return 0;
}

// Wrap writing the member vars of Vec4
static int wrapVec4__newindex(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Get the member variable name
	const Char* ckey;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]
	{
		return lua_error(l);
	}

	CString key = ckey;

	// Try to find the member variable
	if(key == "x")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->x = arg0;
		return 0;
	}
	else if(key == "y")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->y = arg0;
		return 0;
	}
	else if(key == "z")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->z = arg0;
		return 0;
	}
	else if(key == "w")
	{
		F32 arg0;
		if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg0)) [[unlikely]]
		{
			return lua_error(l);
		}
		self->w = arg0;
		return 0;
	}

	return luaL_error(l, "Unknown field %s. Location %s:%d %s", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);
}

// Wrap reading the member vars of Vec4
static int wrapVec4__index(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Get the member variable name
	const Char* ckey;
	if(LuaBinder::checkString(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, ckey)) [[unlikely]]
	{
		return lua_error(l);
	}

	CString key = ckey;

	// Try to find the member variable
	if(key == "x")
	{
		F32 ret = self->x;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}
	else if(key == "y")
	{
		F32 ret = self->y;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}
	else if(key == "z")
	{
		F32 ret = self->z;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}
	else if(key == "w")
	{
		F32 ret = self->w;
		// Push return value
		lua_pushnumber(l, lua_Number(ret));

		return 1;
	}

	// Fallback to methods
	luaL_getmetatable(l, "Vec4");
	lua_getfield(l, -1, ckey);
	if(!lua_isnil(l, -1))
	{
		return 1;
	}

	return luaL_error(l, "Unknown field %s. Location %s:%d %s", key.cstr(), ANKI_FILE, __LINE__, ANKI_FUNC);
}

// Wrap method Vec4::setAll.
static inline int wrapVec4setAll(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 5)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

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
	(*self) = Vec4(arg0, arg1, arg2, arg3);

	return 0;
}

// Wrap method Vec4::getAt.
static inline int wrapVec4getAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	F32 ret = (*self)[arg0];

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Vec4::setAt.
static inline int wrapVec4setAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	U32 arg0;
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
	(*self)[arg0] = arg1;

	return 0;
}

// Wrap method Vec4::operator=.
static inline int wrapVec4copy(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

// Wrap method Vec4::operator+.
static inline int wrapVec4__add(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator+(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

// Wrap method Vec4::operator-.
static inline int wrapVec4__sub(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator-(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

// Wrap method Vec4::operator*.
static inline int wrapVec4__mul(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator*(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

// Wrap method Vec4::operator/.
static inline int wrapVec4__div(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Vec4 ret = self->operator/(arg0);

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

// Wrap method Vec4::operator==.
static inline int wrapVec4__eq(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	Bool ret = self->operator==(arg0);

	// Push return value
	lua_pushboolean(l, ret);

	return 1;
}

// Wrap method Vec4::length.
static inline int wrapVec4length(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	F32 ret = self->length();

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Vec4::normalize.
static inline int wrapVec4normalize(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Call the method
	Vec4 ret = self->normalize();

	// Push return value
	size = LuaUserData::computeSizeForGarbageCollected<Vec4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, "Vec4");
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoVec4);
	::new(ud->getData<Vec4>()) Vec4(std::move(ret));

	return 1;
}

// Wrap method Vec4::dot.
static inline int wrapVec4dot(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* self = ud->getData<Vec4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec4* iarg0 = ud->getData<Vec4>();
	const Vec4& arg0(*iarg0);

	// Call the method
	F32 ret = self->dot(arg0);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap class Vec4.
static inline void wrapVec4(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoVec4);
	LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfoVec4.m_typeName, "new", wrapVec4Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapVec4Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapVec4setAll);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapVec4getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapVec4setAt);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapVec4copy);
	LuaBinder::pushLuaCFuncMethod(l, "__add", wrapVec4__add);
	LuaBinder::pushLuaCFuncMethod(l, "__sub", wrapVec4__sub);
	LuaBinder::pushLuaCFuncMethod(l, "__mul", wrapVec4__mul);
	LuaBinder::pushLuaCFuncMethod(l, "__div", wrapVec4__div);
	LuaBinder::pushLuaCFuncMethod(l, "__eq", wrapVec4__eq);
	LuaBinder::pushLuaCFuncMethod(l, "length", wrapVec4length);
	LuaBinder::pushLuaCFuncMethod(l, "normalize", wrapVec4normalize);
	LuaBinder::pushLuaCFuncMethod(l, "dot", wrapVec4dot);
	LuaBinder::pushLuaCFuncMethod(l, "__newindex", wrapVec4__newindex);
	LuaBinder::pushLuaCFuncMethod(l, "__index", wrapVec4__index);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3 = {2771201786814769536, "Mat3", LuaUserData::computeSizeForGarbageCollected<Mat3>(), nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Mat3>()
{
	return g_luaUserDataTypeInfoMat3;
}

// Pre-wrap constructor for Mat3.
static inline int pwrapMat3Ctor0(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Mat3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoMat3.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoMat3);
	::new(ud->getData<Mat3>()) Mat3();

	return 1;
}

// Pre-wrap constructor for Mat3.
static inline int pwrapMat3Ctor1(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Mat3>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoMat3.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoMat3);
	::new(ud->getData<Mat3>()) Mat3(arg0);

	return 1;
}

// Wrap constructors for Mat3.
static int wrapMat3Ctor(lua_State* l)
{
	// Chose the right overload
	const int argCount = lua_gettop(l);
	int res = 0;
	switch(argCount)
	{
	case 0:
		res = pwrapMat3Ctor0(l);
		break;
	case 1:
		res = pwrapMat3Ctor1(l);
		break;
	default:
		lua_pushfstring(l, "Wrong overloaded new. Wrong number of arguments: %d", argCount);
		res = -1;
	}

	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

// Wrap destructor for Mat3.
static int wrapMat3Dtor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(ud->isGarbageCollected())
	{
		Mat3* inst = ud->getData<Mat3>();
		inst->~Mat3();
	}

	return 0;
}

// Wrap method Mat3::operator=.
static inline int wrapMat3copy(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* self = ud->getData<Mat3>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* iarg0 = ud->getData<Mat3>();
	const Mat3& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

// Wrap method Mat3::getAt.
static inline int wrapMat3getAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* self = ud->getData<Mat3>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	U32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	F32 ret = (*self)(arg0, arg1);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Mat3::setAt.
static inline int wrapMat3setAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* self = ud->getData<Mat3>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	U32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, arg2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	(*self)(arg0, arg1) = arg2;

	return 0;
}

// Wrap method Mat3::setAll.
static inline int wrapMat3setAll(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 10)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* self = ud->getData<Mat3>();

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

	F32 arg4;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 6, arg4)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg5;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 7, arg5)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg6;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 8, arg6)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg7;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 9, arg7)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg8;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 10, arg8)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	(*self) = Mat3(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

	return 0;
}

// Wrap class Mat3.
static inline void wrapMat3(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoMat3);
	LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfoMat3.m_typeName, "new", wrapMat3Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapMat3Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapMat3copy);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapMat3getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapMat3setAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapMat3setAll);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3x4 = {6217558188558067156, "Mat3x4", LuaUserData::computeSizeForGarbageCollected<Mat3x4>(), nullptr,
												   nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Mat3x4>()
{
	return g_luaUserDataTypeInfoMat3x4;
}

// Pre-wrap constructor for Mat3x4.
static inline int pwrapMat3x4Ctor0(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Mat3x4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoMat3x4.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3x4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoMat3x4);
	::new(ud->getData<Mat3x4>()) Mat3x4();

	return 1;
}

// Pre-wrap constructor for Mat3x4.
static inline int pwrapMat3x4Ctor1(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Mat3x4>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoMat3x4.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3x4;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoMat3x4);
	::new(ud->getData<Mat3x4>()) Mat3x4(arg0);

	return 1;
}

// Wrap constructors for Mat3x4.
static int wrapMat3x4Ctor(lua_State* l)
{
	// Chose the right overload
	const int argCount = lua_gettop(l);
	int res = 0;
	switch(argCount)
	{
	case 0:
		res = pwrapMat3x4Ctor0(l);
		break;
	case 1:
		res = pwrapMat3x4Ctor1(l);
		break;
	default:
		lua_pushfstring(l, "Wrong overloaded new. Wrong number of arguments: %d", argCount);
		res = -1;
	}

	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

// Wrap destructor for Mat3x4.
static int wrapMat3x4Dtor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3x4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(ud->isGarbageCollected())
	{
		Mat3x4* inst = ud->getData<Mat3x4>();
		inst->~Mat3x4();
	}

	return 0;
}

// Wrap method Mat3x4::operator=.
static inline int wrapMat3x4copy(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3x4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3x4* self = ud->getData<Mat3x4>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3x4;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoMat3x4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3x4* iarg0 = ud->getData<Mat3x4>();
	const Mat3x4& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

// Wrap method Mat3x4::getAt.
static inline int wrapMat3x4getAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3x4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3x4* self = ud->getData<Mat3x4>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	U32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	F32 ret = (*self)(arg0, arg1);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap method Mat3x4::setAt.
static inline int wrapMat3x4setAt(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3x4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3x4* self = ud->getData<Mat3x4>();

	// Pop arguments
	U32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	U32 arg1;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, arg1)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg2;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 4, arg2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	(*self)(arg0, arg1) = arg2;

	return 0;
}

// Wrap method Mat3x4::setAll.
static inline int wrapMat3x4setAll(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 13)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoMat3x4, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3x4* self = ud->getData<Mat3x4>();

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

	F32 arg4;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 6, arg4)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg5;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 7, arg5)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg6;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 8, arg6)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg7;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 9, arg7)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg8;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 10, arg8)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg9;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 11, arg9)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg10;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 12, arg10)) [[unlikely]]
	{
		return lua_error(l);
	}

	F32 arg11;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 13, arg11)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the method
	(*self) = Mat3x4(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);

	return 0;
}

// Wrap class Mat3x4.
static inline void wrapMat3x4(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoMat3x4);
	LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfoMat3x4.m_typeName, "new", wrapMat3x4Ctor);
	LuaBinder::pushLuaCFuncMethod(l, "__gc", wrapMat3x4Dtor);
	LuaBinder::pushLuaCFuncMethod(l, "copy", wrapMat3x4copy);
	LuaBinder::pushLuaCFuncMethod(l, "getAt", wrapMat3x4getAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAt", wrapMat3x4setAt);
	LuaBinder::pushLuaCFuncMethod(l, "setAll", wrapMat3x4setAll);
	lua_settop(l, 0);
}

LuaUserDataTypeInfo g_luaUserDataTypeInfoTransform = {-5465408310055952111, "Transform", LuaUserData::computeSizeForGarbageCollected<Transform>(),
													  nullptr, nullptr};

template<>
const LuaUserDataTypeInfo& LuaUserData::getDataTypeInfoFor<Transform>()
{
	return g_luaUserDataTypeInfoTransform;
}

// Pre-wrap constructor for Transform.
static inline int pwrapTransformCtor0(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Transform>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoTransform.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTransform;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoTransform);
	::new(ud->getData<Transform>()) Transform();

	return 1;
}

// Pre-wrap constructor for Transform.
static inline int pwrapTransformCtor1(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	Vec3 arg0(*iarg0);

	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* iarg1 = ud->getData<Mat3>();
	Mat3 arg1(*iarg1);

	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 3, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg2 = ud->getData<Vec3>();
	Vec3 arg2(*iarg2);

	// Create user data
	size = LuaUserData::computeSizeForGarbageCollected<Transform>();
	voidp = lua_newuserdata(l, size);
	luaL_setmetatable(l, g_luaUserDataTypeInfoTransform.m_typeName);
	ud = static_cast<LuaUserData*>(voidp);
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTransform;
	ud->initGarbageCollected(&g_luaUserDataTypeInfoTransform);
	::new(ud->getData<Transform>()) Transform(arg0, arg1, arg2);

	return 1;
}

// Wrap constructors for Transform.
static int wrapTransformCtor(lua_State* l)
{
	// Chose the right overload
	const int argCount = lua_gettop(l);
	int res = 0;
	switch(argCount)
	{
	case 0:
		res = pwrapTransformCtor0(l);
		break;
	case 3:
		res = pwrapTransformCtor1(l);
		break;
	default:
		lua_pushfstring(l, "Wrong overloaded new. Wrong number of arguments: %d", argCount);
		res = -1;
	}

	if(res >= 0)
	{
		return res;
	}

	lua_error(l);
	return 0;
}

// Wrap destructor for Transform.
static int wrapTransformDtor(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	if(ud->isGarbageCollected())
	{
		Transform* inst = ud->getData<Transform>();
		inst->~Transform();
	}

	return 0;
}

// Wrap method Transform::operator=.
static inline int wrapTransformcopy(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoTransform;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* iarg0 = ud->getData<Transform>();
	const Transform& arg0(*iarg0);

	// Call the method
	self->operator=(arg0);

	return 0;
}

// Wrap method Transform::getOrigin.
static inline int wrapTransformgetOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Call the method
	Vec3 ret = self->getOrigin().xyz;

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

// Wrap method Transform::setOrigin.
static inline int wrapTransformsetOrigin(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setOrigin(arg0);

	return 0;
}

// Wrap method Transform::getRotation.
static inline int wrapTransformgetRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Call the method
	Mat3 ret = self->getRotation().getRotationPart();

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

// Wrap method Transform::setRotation.
static inline int wrapTransformsetRotation(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoMat3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoMat3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Mat3* iarg0 = ud->getData<Mat3>();
	const Mat3& arg0(*iarg0);

	// Call the method
	self->setRotation(arg0);

	return 0;
}

// Wrap method Transform::getScale.
static inline int wrapTransformgetScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Call the method
	Vec3 ret = self->getScale().xyz;

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

// Wrap method Transform::setScale.
static inline int wrapTransformsetScale(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Get "this" as "self"
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, g_luaUserDataTypeInfoTransform, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Transform* self = ud->getData<Transform>();

	// Pop arguments
	extern LuaUserDataTypeInfo g_luaUserDataTypeInfoVec3;
	if(LuaBinder::checkUserData(l, ANKI_FILE, __LINE__, ANKI_FUNC, 2, g_luaUserDataTypeInfoVec3, ud)) [[unlikely]]
	{
		return lua_error(l);
	}

	Vec3* iarg0 = ud->getData<Vec3>();
	const Vec3& arg0(*iarg0);

	// Call the method
	self->setScale(arg0);

	return 0;
}

// Wrap class Transform.
static inline void wrapTransform(lua_State* l)
{
	LuaBinder::createClass(l, &g_luaUserDataTypeInfoTransform);
	LuaBinder::pushLuaCFuncStaticMethod(l, g_luaUserDataTypeInfoTransform.m_typeName, "new", wrapTransformCtor);
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

// Wrap function toRad.
static inline int wraptoRad(lua_State* l)
{
	[[maybe_unused]] LuaUserData* ud;
	[[maybe_unused]] void* voidp;
	[[maybe_unused]] PtrSize size;

	if(LuaBinder::checkArgsCount(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Pop arguments
	F32 arg0;
	if(LuaBinder::checkNumber(l, ANKI_FILE, __LINE__, ANKI_FUNC, 1, arg0)) [[unlikely]]
	{
		return lua_error(l);
	}

	// Call the function
	F32 ret = toRad(arg0);

	// Push return value
	lua_pushnumber(l, lua_Number(ret));

	return 1;
}

// Wrap the module.
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
