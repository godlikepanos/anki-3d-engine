// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/script/Common.h"
#include "anki/Math.h"

namespace anki {

//==============================================================================
static void vec2SetX(Vec2* self, F32 x)
{
	self->x() = x;
}

static void vec2SetY(Vec2* self, F32 y)
{
	self->y() = y;
}

static void vec2SetAt(Vec2* self, U i, F32 f)
{
	(*self)[i] = f;
}

ANKI_SCRIPT_WRAP(Vec2)
{
	ANKI_LUA_CLASS_BEGIN(lb, Vec2)
		ANKI_LUA_EMPTY_CONSTRUCTOR()
		ANKI_LUA_METHOD("copy", &Vec2::operator=)
		// Accessors
		ANKI_LUA_METHOD_DETAIL("getX", F32 (Vec2::Base::*)() const, &Vec2::x, 
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("getY", F32 (Vec2::Base::*)() const, &Vec2::y,
			BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec2SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec2SetY)
		ANKI_LUA_METHOD_DETAIL("getAt",
			F32 (Vec2::Base::*)(const U) const, &Vec2::operator[], 
			BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &vec2SetAt)
		// Ops with same
		ANKI_LUA_METHOD_DETAIL("__add", Vec2 (Vec2::Base::*)(const Vec2&) const, 
			&Vec2::operator+, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__sub", Vec2 (Vec2::Base::*)(const Vec2&) const, 
			&Vec2::operator-, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__mul", Vec2 (Vec2::Base::*)(const Vec2&) const, 
			&Vec2::operator*, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__div", Vec2 (Vec2::Base::*)(const Vec2&) const, 
			&Vec2::operator/, BinderFlag::NONE)
		ANKI_LUA_METHOD("__eq", &Vec2::operator==)
		ANKI_LUA_METHOD("__lt", &Vec2::operator<)
		ANKI_LUA_METHOD("__le", &Vec2::operator<=)
		// Other
		ANKI_LUA_METHOD("getLength", &Vec2::getLength)
		ANKI_LUA_METHOD("getNormalized", &Vec2::getNormalized)
		ANKI_LUA_METHOD("normalize", &Vec2::normalize)
		ANKI_LUA_METHOD("dot", &Vec2::dot)
	ANKI_LUA_CLASS_END()
}

//==============================================================================
static void vec3SetX(Vec3* self, F32 x)
{
	self->x() = x;
}

static void vec3SetY(Vec3* self, F32 y)
{
	self->y() = y;
}

static void vec3SetZ(Vec3* self, F32 z)
{
	self->z() = z;
}

static void vec3SetAt(Vec3* self, U i, F32 f)
{
	(*self)[i] = f;
}

ANKI_SCRIPT_WRAP(Vec3)
{
	ANKI_LUA_CLASS_BEGIN(lb, Vec3)
		ANKI_LUA_EMPTY_CONSTRUCTOR()
		ANKI_LUA_METHOD("copy", &Vec3::operator=)
		// Accessors
		ANKI_LUA_METHOD_DETAIL("getX", F32 (Vec3::Base::*)() const, 
			&Vec3::x, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("getY", F32 (Vec3::Base::*)() const, 
			&Vec3::y, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("getZ", F32 (Vec3::Base::*)() const, 
			&Vec3::z, BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec3SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec3SetY)
		ANKI_LUA_FUNCTION_AS_METHOD("setZ", &vec3SetZ)
		ANKI_LUA_METHOD_DETAIL("getAt", F32 (Vec3::Base::*)(const U) const, 
			&Vec3::operator[], BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &vec3SetAt)
		// Ops with same
		ANKI_LUA_METHOD_DETAIL("__add", Vec3 (Vec3::Base::*)(const Vec3&) const, 
			&Vec3::operator+, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__sub", Vec3 (Vec3::Base::*)(const Vec3&) const, 
			&Vec3::operator-, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__mul", Vec3 (Vec3::Base::*)(const Vec3&) const, 
			&Vec3::operator*, BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__div", Vec3 (Vec3::Base::*)(const Vec3&) const, 
			&Vec3::operator/, BinderFlag::NONE)
		ANKI_LUA_METHOD("__eq", &Vec3::operator==)
		ANKI_LUA_METHOD("__lt", &Vec3::operator<)
		ANKI_LUA_METHOD("__le", &Vec3::operator<=)
		// Other
		ANKI_LUA_METHOD("getLength", &Vec3::getLength)
		ANKI_LUA_METHOD("getNormalized", &Vec3::getNormalized)
		ANKI_LUA_METHOD("normalize", &Vec3::normalize)
		ANKI_LUA_METHOD("dot", &Vec3::dot)
	ANKI_LUA_CLASS_END()
}

//==============================================================================
static void vec4SetX(Vec4* self, F32 x)
{
	self->x() = x;
}

static void vec4SetY(Vec4* self, F32 y)
{
	self->y() = y;
}

static void vec4SetZ(Vec4* self, F32 z)
{
	self->z() = z;
}

static void vec4SetW(Vec4* self, F32 w)
{
	self->w() = w;
}

static void vec4SetAt(Vec4* self, U i, F32 f)
{
	(*self)[i] = f;
}

ANKI_SCRIPT_WRAP(Vec4)
{
	ANKI_LUA_CLASS_BEGIN(lb, Vec4)
		ANKI_LUA_EMPTY_CONSTRUCTOR()
		ANKI_LUA_METHOD("copy", &Vec4::operator=)
		// Accessors
		ANKI_LUA_METHOD_DETAIL("getX", F32 (Vec4::Base::*)() const, &Vec4::x, 
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("getY", F32 (Vec4::Base::*)() const, &Vec4::y, 
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("getZ", F32 (Vec4::Base::*)() const, &Vec4::z, 
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("getW", F32 (Vec4::Base::*)() const, &Vec4::w,
			BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec4SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec4SetY)
		ANKI_LUA_FUNCTION_AS_METHOD("setZ", &vec4SetZ)
		ANKI_LUA_FUNCTION_AS_METHOD("setW", &vec4SetW)
		ANKI_LUA_METHOD_DETAIL("getAt", F32 (Vec4::Base::*)(const U) const, 
			&Vec4::operator[], BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &vec4SetAt)
		// Ops with same
		ANKI_LUA_METHOD_DETAIL("__add", 
			Vec4 (Vec4::Base::*)(const Vec4&) const, &Vec4::operator+, 
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__sub",
			Vec4 (Vec4::Base::*)(const Vec4&) const, &Vec4::operator-, 
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__mul",
			Vec4 (Vec4::Base::*)(const Vec4&) const, &Vec4::operator*,
			BinderFlag::NONE)
		ANKI_LUA_METHOD_DETAIL("__div",
			Vec4 (Vec4::Base::*)(const Vec4&) const, &Vec4::operator/,
			BinderFlag::NONE)
		ANKI_LUA_METHOD("__eq", &Vec4::operator==)
		ANKI_LUA_METHOD("__lt", &Vec4::operator<)
		ANKI_LUA_METHOD("__le", &Vec4::operator<=)
		// Other
		ANKI_LUA_METHOD("getLength", &Vec4::getLength)
		ANKI_LUA_METHOD("getNormalized", &Vec4::getNormalized)
		ANKI_LUA_METHOD("normalize", &Vec4::normalize)
		ANKI_LUA_METHOD("dot", &Vec4::dot)
	ANKI_LUA_CLASS_END()
}

//==============================================================================
static void mat3SetAt(Mat3* self, U j, U i, F32 f)
{
	(*self)(j, i) = f;
}

ANKI_SCRIPT_WRAP(Mat3)
{
	ANKI_LUA_CLASS_BEGIN(lb, Mat3)
		ANKI_LUA_EMPTY_CONSTRUCTOR()
		ANKI_LUA_METHOD("copy", &Mat3::operator=)
		// Accessors
		ANKI_LUA_METHOD_DETAIL("getAt", 
			F32 (Mat3::Base::*)(const U, const U) const, 
			&Mat3::operator(), BinderFlag::NONE)
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &mat3SetAt)
	ANKI_LUA_CLASS_END()
}

} // end namespace anki

