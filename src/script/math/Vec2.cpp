#include "anki/script/MathCommon.h"
#include "anki/Math.h"

namespace anki {

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
		ANKI_LUA_CONSTRUCTOR(F32, F32)
		ANKI_LUA_METHOD("copy", &Vec2::operator=)
		// Accessors
		ANKI_LUA_METHOD("getX", (F32 (Vec2::*)() const)(&Vec2::x))
		ANKI_LUA_METHOD("getY", (F32 (Vec2::*)() const)(&Vec2::y))
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec2SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec2SetY)
		ANKI_LUA_METHOD("getAt",
			(F32 (Vec2::*)(const U) const)(&Vec2::operator[]))
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &vec2SetAt)
		// Ops with same
		ANKI_LUA_METHOD("__add",
			(Vec2 (Vec2::*)(const Vec2&) const)(&Vec2::operator+))
		ANKI_LUA_METHOD("__sub",
			(Vec2 (Vec2::*)(const Vec2&) const)(&Vec2::operator-))
		ANKI_LUA_METHOD("__mul",
			(Vec2 (Vec2::*)(const Vec2&) const)(&Vec2::operator*))
		ANKI_LUA_METHOD("__div",
			(Vec2 (Vec2::*)(const Vec2&) const)(&Vec2::operator/))
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

} // end namespace anki
